/****************************************************************\
*                                                                *
* Copyright (C) 2007 by Markus Duft <markus.duft@salomon.at>     *
*                                                                *
* This file is part of parity.                                   *
*                                                                *
* parity is free software: you can redistribute it and/or modify *
* it under the terms of the GNU Lesser General Public License as *
* published by the Free Software Foundation, either version 3 of *
* the License, or (at your option) any later version.            *
*                                                                *
* parity is distributed in the hope that it will be useful,      *
* but WITHOUT ANY WARRANTY; without even the implied warranty of *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the  *
* GNU Lesser General Public License for more details.            *
*                                                                *
* You should have received a copy of the GNU Lesser General      *
* Public License along with parity. If not,                      *
* see <http://www.gnu.org/licenses/>.                            *
*                                                                *
\****************************************************************/

#include "MsLoaderGenerator.h"

#include <map>
#include <sstream>

#include <Log.h>
#include <Path.h>
#include <MemoryFile.h>
#include <Context.h>
#include <Environment.h>
#include <Statistics.h>

#include <CoffObject.h>

#ifdef HAVE_CONFIG_H
//
// required for PARITY_LIBDIR
//
#  include <config.h>
#endif

namespace parity
{
	namespace tasks
	{
		MsLoaderGenerator::MsLoaderGenerator(const tasks::BinaryGatherer::ImportHybridityMap& imports)
			: imports_(imports)
		{
			//
			// lookup the loader library, and put it on the link line.
			//
			utils::Context& ctx = utils::Context::getContext();

			utils::Log::verbose("trying to find suitable parity.loader library...\n");

			if(ctx.getParityLoader().get().empty())
			{
				//
				// try to lookup library ourselves
				//
				utils::Path location;

				#if defined(_WIN32) && !defined(PARITY_LIBDIR)
					char fnBuffer[1024];
					GetModuleFileName(GetModuleHandle(NULL), fnBuffer, 1024);
					
					location = utils::Path(fnBuffer);
					location = location.base();
					location.append("parity.loader.lib");
				#else
					location = utils::Path(PARITY_LIBDIR);
					location.toNative();
					location.append("libparity_parity.loader.a");

					if(!location.exists())
					{
						//
						// try from non-installed structure?
						//
						utils::Log::warning("TODO: non-installed loader library search.");
					}
				#endif

				if(location.exists())
					ctx.getObjectsLibraries().push_back(location);
				else
					throw utils::Exception("cannot lookup parity.loader library, please set in configuration!");
			} else {
				ctx.getObjectsLibraries().push_back(ctx.getParityLoader());
			}

			if(ctx.getGenerateLoader())
				utils::Log::verbose("Loader generator initialized for %d imports.\n", imports.size());
			else
				utils::Log::verbose("Loader generator to be skipped (disabled). %d imports not loaded.\n", imports.size());
		}

		static std::string symbolifyName(std::string str)
		{
			for(std::string::iterator it = str.begin(); it != str.end(); ++it)
				if(!isalnum(*it))
					*it = '_';

			return str;
		}

		static const unsigned char dataEmptyPtr[] = { 0x00, 0x00, 0x00, 0x00 };
		static const unsigned char dataBFoodPtr[] = { 0x0D, 0xF0, 0xAD, 0xBA };

		void MsLoaderGenerator::doWork()
		{
			//
			// 1) Generate the object for the loader
			// 2) Remove all libraries that are handled by the loader.
			//    *) keep hybrid libraries
			//
			utils::Context& ctx = utils::Context::getContext();

			//
			// We can exit here safely, since the library is added to the binary, which
			// means all the things required from the loader lib can be resolved.
			//
			if(!ctx.getGenerateLoader())
				return;

			binary::Object obj;
			binary::FileHeader& hdr = obj.getHeader();

			hdr.setMachine(binary::FileHeader::MachineI386);

			binary::Section& sectText = hdr.addSection(".text");
			binary::Section& sectData = hdr.addSection(".data");
			binary::Section& sectRData = hdr.addSection(".rdata");

			sectText.setCharacteristics( binary::Section::CharAlign16Bytes | binary::Section::CharMemoryExecute | binary::Section::CharMemoryRead | binary::Section::CharContentCode);
			sectData.setCharacteristics( binary::Section::CharAlign4Bytes  | binary::Section::CharMemoryRead    | binary::Section::CharMemoryWrite| binary::Section::CharContentInitData);
			sectRData.setCharacteristics(binary::Section::CharAlign4Bytes  | binary::Section::CharMemoryRead    | binary::Section::CharContentInitData);

			unsigned int indexOfLazyLoader = 0;

			if(ctx.getLazyLoading())
			{
				binary::Symbol& ext = hdr.addSymbol("_ParityLoaderLazyLoadSymbol");

				ext.setSectionNumber(0);
				ext.setStorageClass(binary::Symbol::ClassExternal);
				ext.setType(binary::Symbol::ComplexFunction);

				indexOfLazyLoader = ext.getIndex();
			}

			LoaderWorkingMap items;

			for(tasks::BinaryGatherer::ImportHybridityMap::const_iterator it = imports_.begin(); it != imports_.end(); ++it)
			{
				std::string libraryName = it->first.second.getLibraryName();
				LoaderWorkingItem& item = items[libraryName];

				if(item.name.empty())
				{
					item.name = libraryName;
					item.hybrid = it->second;
					item.path = it->first.first;
					item.recordIndex = 0;
				}

				item.imports.push_back(it->first.second);
			}

			std::map<std::string, unsigned int> stringMapping;
			unsigned int stringIndex = 0;

			utils::Statistics::instance().addInformation("loaded-libraries", items.size());

			for(LoaderWorkingMap::iterator it = items.begin(); it != items.end(); ++it)
			{
				std::string libraryName   = it->first;
				LoaderWorkingItem& item	  = it->second;	
				unsigned int& indexOfLibNameSym = stringMapping[libraryName];

				if(!indexOfLibNameSym)
				{
					//
					// generate unique name for the string variable.
					//
					std::ostringstream oss;
					oss << "$LDR" << stringIndex++;

					//
					// and create the symbol for it.
					//
					binary::Symbol& symVar = hdr.addSymbol(oss.str());
					sectRData.markSymbol(symVar);
					sectRData.addData(libraryName.c_str(), libraryName.length() + 1);
					symVar.setStorageClass(binary::Symbol::ClassStatic);
					indexOfLibNameSym = symVar.getIndex();
				}

				binary::Symbol& symLibraryName = hdr.getAllSymbols()[indexOfLibNameSym];

				//
				// Start a new import library table.
				//
				// general layout is as follows
				//  struct {
				//    unsigned int name;  // pointer to string containing library name.
				//    unsigned int table; // pointer to the import table generated below.
				//  };
				//
				// because the table pointer is unknwon here, the structure is generated *after*
				// the import table itself.
				//

				//
				// generate the start of the table (symbol table entry)
				//
				std::string tableSymbolName = "$ITAB_" + symbolifyName(libraryName);
				binary::Symbol& symTableStart = hdr.addSymbol(tableSymbolName);
				sectData.markSymbol(symTableStart);
				symTableStart.setStorageClass(binary::Symbol::ClassStatic);

				item.tableIndex = symTableStart.getIndex();
				item.nameIndex = indexOfLibNameSym;

				utils::Statistics::instance().addInformation("loaded-imports(" + libraryName + ")", item.imports.size());
				
				for(binary::Import::ImportVector::iterator import = item.imports.begin(); import != item.imports.end(); ++import)
				{
					std::string  symbolName = import->getSymbolName();
					unsigned int& indexOfImportNameSym = stringMapping[symbolName];

					if(!indexOfImportNameSym)
					{
						//
						// generate another unique name, this time for the symbol.
						//
						std::ostringstream oss;
						oss << "$LDR" << stringIndex++;

						//
						// and the mathing symbol.
						//
						binary::Symbol& symVar = hdr.addSymbol(oss.str());
						sectRData.markSymbol(symVar);

						//
						// calculate symbol name (undecorate, etc.)
						//
						std::string undecorated = symbolName;
						switch(import->getNameType())
						{
						case binary::Import::ImportByName:
							break;
						case binary::Import::ImportByNameNoPrefix:
							if(undecorated[0] == '_' || undecorated[0] == '@' || undecorated[0] == '?')
								undecorated = undecorated.substr(1);
							break;
						case binary::Import::ImportByNameUndecorate:
							{
								if(undecorated[0] == '_' || undecorated[0] == '@' || undecorated[0] == '?')
									undecorated = undecorated.substr(1);

								std::string::size_type pos = 0;
								if((pos = undecorated.find('@')) != std::string::npos)
									undecorated = undecorated.substr(0, pos);
							}
							break;
						case binary::Import::ImportByOrdinal:
							undecorated = "";
							break;
						}

						sectRData.addData(undecorated.c_str(), undecorated.length() + 1);
						symVar.setStorageClass(binary::Symbol::ClassStatic);

						indexOfImportNameSym = symVar.getIndex();
					}

					binary::Symbol& symImportName = hdr.getAllSymbols()[indexOfImportNameSym];

					//
					// generate data table rows for the loader
					//
					// general layout is as follows:
					//  struct {
					//    unsigned int name; // pointer to char[] containing the name of the symbol.
					//    unsigned int ptr;  // pointer to be patched by the loader, will contain addr of real var.
					//    unsigned short ord;  // if non-zero, import symbol by using this ordinal number.
					//    unsigned int library;// pointer to char[] conatining name of library.
					//  };
					//
					// warning: the generator marks the ptr field as __imp_* too, this means
					// that both are the SAME! you can assign something to the ptr field which
					// will also modify the __imp_ ptr.
					//

					//
					// generate a symbol table entry for this, so we can relocate to this
					// for the lazy loading stuff below.
					//
					binary::Symbol& symCurrentItem = hdr.addSymbol("$ISYM_" + symbolifyName(symbolName));
					symCurrentItem.setStorageClass(binary::Symbol::ClassStatic);
					sectData.markSymbol(symCurrentItem);

					//
					// first item of struct is the name field
					//
					sectData.markRelocation(symImportName, binary::Relocation::i386Direct32);
					sectData.addData(dataEmptyPtr, sizeof(dataEmptyPtr));

					if(import->getType() == binary::Import::ImportCode)
					{
						//
						// if its a code symbol generate a stub.
						//
						if(ctx.getLazyLoading())
						{
							//
							// with lazy loading things get a little more sofisticated.
							//
							binary::Symbol& symLazyStub = hdr.addSymbol(symbolName);
							sectText.markSymbol(symLazyStub);
							symLazyStub.setStorageClass(binary::Symbol::ClassExternal);
							symLazyStub.setType(binary::Symbol::ComplexFunction);

							//
							// now generate import pointer which points initially to the stub
							//
							binary::Symbol& symImp = hdr.addSymbol("__imp_" + symbolName);
							symImp.setStorageClass(binary::Symbol::ClassExternal);

							sectData.markSymbol(symImp);
							sectData.markRelocation(symLazyStub, binary::Relocation::i386Direct32);
							sectData.addData(dataEmptyPtr, sizeof(dataEmptyPtr));

							//
							// now that all symbols are set up, the data for the .text section.
							//
							binary::Symbol& symLazyLoader = hdr.getAllSymbols()[indexOfLazyLoader];

							sectText.addData(dataLazyStubPart1, sizeof(dataLazyStubPart1));
							sectText.markRelocation(symImp, binary::Relocation::i386Direct32);
							sectText.addData(dataEmptyPtr, sizeof(dataEmptyPtr));
							sectText.markRelocation(symLazyStub, binary::Relocation::i386Direct32);
							sectText.addData(dataEmptyPtr, sizeof(dataEmptyPtr));
							sectText.addData(dataLazyStubPart2, sizeof(dataLazyStubPart2));
							sectText.markRelocation(symCurrentItem, binary::Relocation::i386Direct32);
							sectText.addData(dataEmptyPtr, sizeof(dataEmptyPtr));
							sectText.addData(dataLazyStubPart3, sizeof(dataLazyStubPart3));
							sectText.markRelocation(symLazyLoader, binary::Relocation::i386Relative32);
							sectText.addData(dataEmptyPtr, sizeof(dataEmptyPtr));
							sectText.addData(dataLazyStubPart4, sizeof(dataLazyStubPart4));
							sectText.markRelocation(symImp, binary::Relocation::i386Direct32);
							sectText.addData(dataEmptyPtr, sizeof(dataEmptyPtr));

							sectText.padSection();
						} else {
							//
							// Without lazy loading this is easy ;o)
							//
							binary::Symbol& symImp = hdr.addSymbol("__imp_" + symbolName);
							symImp.setStorageClass(binary::Symbol::ClassExternal);

							sectData.markSymbol(symImp);
							sectData.addData(dataEmptyPtr, sizeof(dataEmptyPtr));

							//
							// now the stub
							//
							binary::Symbol& symSmallStub = hdr.addSymbol(symbolName);
							sectText.markSymbol(symSmallStub);
							symSmallStub.setStorageClass(binary::Symbol::ClassExternal);
							symSmallStub.setType(binary::Symbol::ComplexFunction);

							const unsigned char dataSmallStub[] = { 0xFF, 0x25 }; // jmp dword ptr [...]
							
							sectText.addData(dataSmallStub, sizeof(dataSmallStub));
							sectText.markRelocation(symImp, binary::Relocation::i386Direct32);
							sectText.addData(dataEmptyPtr, sizeof(dataEmptyPtr));

							//
							// finally pad until default alignment.
							//
							sectText.padSection();
						}
					} else {
						//
						// if it's a data symbol, simply generate the import pointer
						//
						binary::Symbol& imp = hdr.addSymbol("__imp_" + symbolName);
						imp.setStorageClass(binary::Symbol::ClassExternal);

						sectData.markSymbol(imp);
						sectData.addData(dataBFoodPtr, sizeof(dataBFoodPtr));
					}

					//
					// generate ordinal number field.
					//
					if(import->getNameType() == binary::Import::ImportByOrdinal)
					{
						unsigned short ord = import->getOrdinalOrHint();
						sectData.addData(&ord, 2);
					} else {
						sectData.addData(dataEmptyPtr, sizeof(unsigned short));
					}

					//
					// generate pointer to library name for easy of loading.
					//
					sectData.markRelocation(symLibraryName, binary::Relocation::i386Direct32);
					sectData.addData(dataEmptyPtr, sizeof(dataEmptyPtr));
				}

				//
				// terminate the table with 3 zero pointers
				//
				sectData.addData(dataEmptyPtr, sizeof(dataEmptyPtr)); // name
				sectData.addData(dataEmptyPtr, sizeof(dataEmptyPtr)); // ptr
				sectData.addData(dataEmptyPtr, sizeof(dataEmptyPtr)); // ord
			}

			//
			// generate table entry seperatly, so they are behind each other.
			//
			for(LoaderWorkingMap::iterator it = items.begin(); it != items.end(); ++it)
			{
				//
				// import table now known, so generate library record.
				//
				std::string recordName = "$LIB_" + symbolifyName(it->first);
				binary::Symbol& symRecord = hdr.addSymbol(recordName);
				sectData.markSymbol(symRecord);
				symRecord.setStorageClass(binary::Symbol::ClassStatic);
				it->second.recordIndex = symRecord.getIndex();
				
				sectData.markRelocation(hdr.getAllSymbols()[it->second.nameIndex], binary::Relocation::i386Direct32);
				sectData.addData(dataEmptyPtr, sizeof(dataEmptyPtr));
				sectData.markRelocation(hdr.getAllSymbols()[it->second.tableIndex], binary::Relocation::i386Direct32);
				sectData.addData(dataEmptyPtr, sizeof(dataEmptyPtr));
			}

			//
			// terminate the table with 2 zero pointers
			//
			sectData.addData(dataEmptyPtr, sizeof(dataEmptyPtr)); // name
			sectData.addData(dataEmptyPtr, sizeof(dataEmptyPtr)); // ptr

			//
			// once more to generate the final record
			//

			binary::Symbol& symLoaderTable = hdr.addSymbol("_ParityLoaderGeneratedTable");
			sectData.markSymbol(symLoaderTable);
			symLoaderTable.setStorageClass(binary::Symbol::ClassExternal);

			for(LoaderWorkingMap::iterator it = items.begin(); it != items.end(); ++it)
			{
				//
				// finally generate a table which contains pointers to all library tables
				//
				sectData.markRelocation(hdr.getAllSymbols()[it->second.recordIndex], binary::Relocation::i386Direct32);
				sectData.addData(dataEmptyPtr, sizeof(dataEmptyPtr));

				//
				// now remove non-hybrid items from the library list for the linker.
				//
				if(!it->second.hybrid)
				{
					utils::PathVector& paths = ctx.getObjectsLibraries();
					utils::PathVector::iterator pos;

					for(pos = paths.begin(); pos != paths.end(); ++pos)
					{
						if(*pos == it->second.path)
							break;
					}

					if(pos == paths.end())
						utils::Log::warning("cannot find %s in library list for removal!\n", it->second.path.get().c_str());
					else {
						utils::Log::verbose("removing %s from library list, handled by loader!\n", it->second.path.get().c_str());
						paths.erase(pos);
					}
				}
			}

			//
			// terminate list with zero pointer.
			//
			sectData.addData(dataEmptyPtr, sizeof(dataEmptyPtr));

			if(!items.empty())
			{
				//
				// handle entry point stuff
				//

				std::string entry = ctx.getSharedLink() ? ctx.getSharedEntryPoint() : ctx.getEntryPoint();

				if(entry.empty())
					entry = ctx.calculateDefaultEntrypoint();

				//
				// in any case we need the loaders init routine.
				//
				binary::Symbol& symLoaderInit = hdr.addSymbol("_ParityLoaderInit");
				symLoaderInit.setSectionNumber(0);
				symLoaderInit.setStorageClass(binary::Symbol::ClassExternal);
				symLoaderInit.setType(binary::Symbol::ComplexFunction);

				//
				// extern definition for real entry point.
				//
				binary::Symbol& symRealEntry = hdr.addSymbol("_" + entry);
				symRealEntry.setSectionNumber(0);
				symRealEntry.setStorageClass(binary::Symbol::ClassExternal);
				symRealEntry.setType(binary::Symbol::ComplexFunction);

				std::string ename = "ParityLoaderGeneratedEntry";

				if(ctx.getSharedLink())
					ename.append("@12");

				binary::Symbol& symParityEntry = hdr.addSymbol("_" + ename);
				symParityEntry.setStorageClass(binary::Symbol::ClassExternal);
				symParityEntry.setType(binary::Symbol::ComplexFunction);

				sectText.markSymbol(symParityEntry);

				if(ctx.getSharedLink())
				{
					//
					// generate entry point that is able of handling arguments and stdcall.
					//

					sectText.addData(dataDllEntryPart1, sizeof(dataDllEntryPart1));
					sectText.markRelocation(symLoaderInit, binary::Relocation::i386Relative32);
					sectText.addData(dataEmptyPtr, sizeof(dataEmptyPtr));
					sectText.addData(dataDllEntryPart2, sizeof(dataDllEntryPart2));
					sectText.markRelocation(symRealEntry, binary::Relocation::i386Relative32);
					sectText.addData(dataEmptyPtr, sizeof(dataEmptyPtr));
					sectText.addData(dataDllEntryPart3, sizeof(dataDllEntryPart3));

					sectText.padSection();
				} else {
					//
					// generate entry point for void entry point without arguments
					//

					sectText.addData(dataExeEntryPart1, sizeof(dataExeEntryPart1));
					sectText.markRelocation(symLoaderInit, binary::Relocation::i386Relative32);
					sectText.addData(dataEmptyPtr, sizeof(dataEmptyPtr));
					sectText.addData(dataExeEntryPart2, sizeof(dataExeEntryPart2));
					sectText.markRelocation(symRealEntry, binary::Relocation::i386Relative32);
					sectText.addData(dataEmptyPtr, sizeof(dataEmptyPtr));
					sectText.addData(dataExeEntryPart3, sizeof(dataExeEntryPart3));

					sectText.padSection();
				}

				ctx.setEntryPoint(ename);
				ctx.setSharedEntryPoint(ename);
			}

			//
			// generate symbols for subsystem and runpath.
			// we need both symbols even if runpath is not set.
			// in this case the symbol data is set to all zeros.
			// (runpath has it's own section too)
			//

			//
			// set up the section where all the pointer will go to.
			//
			binary::Section& sectPtrs = hdr.addSection(".p.ptrs");
			sectPtrs.setCharacteristics(binary::Section::CharAlign4Bytes | binary::Section::CharContentInitData | binary::Section::CharMemoryRead);

			//
			// need a pointer to the symbol table, but this does not need a symbol name. it's only
			// for access through the parity.inspector
			//
			binary::Symbol& symTablePointer = hdr.addSymbol("$PTR_ParityLoaderGeneratedTable");
			symTablePointer.setStorageClass(binary::Symbol::ClassStatic);
			sectPtrs.markSymbol(symTablePointer);
			sectPtrs.markRelocation(symLoaderTable, binary::Relocation::i386Direct32);
			sectPtrs.addData(dataEmptyPtr, sizeof(dataEmptyPtr));

			//
			// set up section where runpaths will go.
			//
			binary::Section& sectRPath = hdr.addSection(".p.rpath");
			sectRPath.setCharacteristics(binary::Section::CharAlign1Bytes | binary::Section::CharContentInitData | binary::Section::CharMemoryRead);
			
			binary::Symbol& symRPath = hdr.addSymbol("$START_ParityLoaderGeneratedRunPath");
			symRPath.setStorageClass(binary::Symbol::ClassStatic);

			sectRPath.markSymbol(symRPath);

			if(ctx.getRunPaths().empty())
			{
				//
				// try LD_RUN_PATH
				//
				utils::Environment envRunPath("LD_RUN_PATH");
				utils::PathVector vec = envRunPath.getPathVector();

				for(utils::PathVector::iterator it = vec.begin(); it != vec.end(); ++it)
				{
					it->toForeign();
					sectRPath.addData(it->get().c_str(), it->get().length() + 1);
				}
			} else {
				std::map<utils::Path, bool> dupCheck;

				for(utils::PathVector::const_iterator it = ctx.getRunPaths().begin(); it != ctx.getRunPaths().end(); ++it)
				{
					//
					// add a path. the path is zero terminated, the last one is double-zero terminated
					// warning: this relies on zero terminated strings in std::string.
					//
					utils::Path tmp = *it;
					tmp.toForeign();

					bool& ref = dupCheck[tmp];

					if(!ref) {
						sectRPath.addData(tmp.get().c_str(), tmp.get().length() + 1);
						ref = true;
					}
				}
			}

			//
			// the following is to add the trailing zero for double termination.
			// also if no runpaths are set, then there would be no section data
			// without this line, since there would be no need found to pad the
			// section until 4KB are reached.
			//
			unsigned char nullChar = 0x00;
			sectRPath.addData(&nullChar, sizeof(nullChar));

			//
			// pad until full 4KB are reached. need to make room for possible
			// later modifications of the image, since there may be calls to
			// chrpath or such.
			//
			if(sectRPath.getSizeOfRawData() < (4 * 1024))
				sectRPath.padSection(4 * 1024);
			
			//
			// now we still need a pointer to the rpaths.
			//
			binary::Symbol& symRPathPointer = hdr.addSymbol("_ParityLoaderGeneratedRunPath");
			symRPathPointer.setStorageClass(binary::Symbol::ClassExternal);
			sectPtrs.markSymbol(symRPathPointer);
			sectPtrs.markRelocation(symRPath, binary::Relocation::i386Direct32);
			sectPtrs.addData(dataEmptyPtr, sizeof(dataEmptyPtr));

			//
			// now a symbol containing the filename of the executable module.
			//
			binary::Symbol& symImageNameData = hdr.addSymbol("$START__ParityLoaderGeneratedImageName");
			symImageNameData.setStorageClass(binary::Symbol::ClassStatic);
			sectRData.markSymbol(symImageNameData);
			sectRData.addData(ctx.getOutputFile().file().c_str(), ctx.getOutputFile().file().length() + 1);
			sectRData.padSection();

			binary::Symbol& symImageNamePtr = hdr.addSymbol("_ParityLoaderGeneratedImageName");
			symImageNamePtr.setStorageClass(binary::Symbol::ClassExternal);
			sectPtrs.markSymbol(symImageNamePtr);
			sectPtrs.markRelocation(symImageNameData, binary::Relocation::i386Direct32);
			sectPtrs.addData(dataEmptyPtr, sizeof(dataEmptyPtr));

			binary::Symbol& symSubsystem = hdr.addSymbol("_ParityLoaderGeneratedSubsystem");
			symSubsystem.setStorageClass(binary::Symbol::ClassExternal);
			sectPtrs.markSymbol(symSubsystem);
			//
			// WARNING: since the loader cannot use parity::utils, it needs to
			// define the subsystem values itself, and this must be kept in sync!
			//
			int subsys = static_cast<int>(ctx.getSubsystem());
			sectPtrs.addData(&subsys, sizeof(int));

			//
			// now save the object file to disk.
			//
			utils::MemoryFile mem;
			obj.update(mem);

			utils::Path pth = utils::Path::getTemporary(".parity.loader.XXXXXX.o");

			mem.save(pth);
			ctx.getObjectsLibraries().push_back(pth);
			ctx.getTemporaryFiles().push_back(pth);
		}
	}
}

