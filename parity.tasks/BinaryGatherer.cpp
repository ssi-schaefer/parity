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

#include "BinaryGatherer.h"

#include <Context.h>
#include <MappedFile.h>
#include <MappedFileCache.h>
#include <GenericFile.h>
#include <Log.h>
#include <Statistics.h>

#include <CoffFile.h>
#include <CoffImage.h>
#include <CoffObject.h>
#include <CoffDirectiveSection.h>

namespace parity
{
	namespace tasks
	{
		//
		// The Binary Gatherer needs to use MappedFileCache to keep
		// all Files open, since otherwise all the stored symbols etc.
		// would point into invalid memory. This may result in too many
		// open file handls
		//

		BinaryGatherer::BinaryGatherer()
		{
			if(utils::Context::getContext().getObjectsLibraries().empty())
				throw utils::Exception("construction of binary gatherer without binaries is invalid!");
		}

		void BinaryGatherer::doWork()
		{
			//
			// does the following:
			// * lookup libraries from directive sections.
			// * lookup all symbols from all objects / libraries.
			// * while doing so, check for debug infos.
			//

			utils::Context& ctx = utils::Context::getContext();
			utils::PathVector& all = ctx.getObjectsLibraries();
			bool bSeenObject = false;

			for(utils::PathVector::iterator it = all.begin(); it != all.end(); ++it)
			{
				it->toNative();

				if(!ctx.getGatherSystem())
				{
					//
					// skip if path is inside of a system path.
					//
					bool bIsSystem = false;;
					for(utils::PathVector::iterator sys = ctx.getSysLibraryPaths().begin(); sys != ctx.getSysLibraryPaths().end(); ++sys)
					{
						sys->toNative();
						if(it->get().find(sys->get()) != std::string::npos)
							bIsSystem = true;
					}

					if(bIsSystem)
					{
						utils::Log::verbose("skipping system library %s\n", it->get().c_str());
						continue;
					}
				}

				//
				// skip if handled allready through an implicit dependency!
				//
				bool& ref = processed_[*it];
				if(ref)
				{
					utils::Log::verbose("skipping %s, allready cached!\n", it->get().c_str());
					continue;
				}
				ref = true;

				#ifndef _WIN32
				#  define _strnicmp strncasecmp
				#endif

				if(_strnicmp(it->file().c_str(), "kernel32", 8) == 0)
				{
					utils::Log::verbose("skipping special library %s!\n", it->get().c_str());
					continue;
				}

				#ifdef _strnicmp
				#  undef _strnicmp
				#endif

				utils::MappedFile mapping(*it, utils::ModeRead);
				utils::Log::verbose("gathering in %s ...\n", mapping.getPath().get().c_str());

				switch(binary::File::getType(&mapping))
				{
				case binary::File::TypeImage:
					utils::Log::warning("gathering from EXE or DLL not yet supported, skipping %s!\n", it->get().c_str());
					break;
				case binary::File::TypeLibrary:
					{
						//
						// We need the intermediate variable for gcc 3.3 to be able to
						// pass the archive as reference.
						//
						binary::Archive arch(&mapping);
						processArchive(arch, *it);
					}
					break;
				case binary::File::TypeObject:
					{
						binary::Object obj(&mapping);
						processHeader(obj.getHeader(), *it);

						bSeenObject = true;

						if(!ctx.getDebugable())
						{
							//
							// only from local object, try finding debug information
							//
							binary::Section::IndexedSectionMap sects = obj.getHeader().getSections();

							for(binary::Section::IndexedSectionMap::const_iterator sec = sects.begin(); sec != sects.end(); ++sec)
							{
								if(sec->second.getName() == ".debug$T")
								{
									ctx.setDebugable(true);
									utils::Log::verbose("%s contains debug type information, linking debugable.\n", it->get().c_str());
								}
							}
						}
					}
					break;
				default:
					throw utils::Exception("invalid or unhandled binary file type for %s!", mapping.getPath().get().c_str());
				}
			}

			//
			// now merge the looked up implicits.
			//
			for(PathUsageMap::iterator it = implicits_.begin(); it != implicits_.end(); ++it)
			{
				all.push_back(it->first);
			}

			//
			// are we exporting from libraries?
			//
			if(!bSeenObject) {
				utils::Log::verbose("no object seen, switching to library export mode.\n");
				ctx.setExportAll(true);
			}

			if((ctx.getExportAll() && !maybeExports_.empty())
				|| (!maybeExports_.empty() && (!ctx.getSharedLink() && ctx.getExportFromExe())))
			{
				//
				// merge maybeExports_ to exports_
				//
				exports_.insert(exports_.end(), maybeExports_.begin(), maybeExports_.end());
			}

			//
			// resolve all symbols
			//
			utils::Log::verbose("resolving symbols from all data...\n");
			resolveSymbols();

			utils::Statistics::instance().addInformation("symbols-unresolved", unresolved_.size());
			utils::Statistics::instance().addInformation("symbols-local", local_.size());
			utils::Statistics::instance().addInformation("symbols-import", imports_.size());

			utils::Log::verbose("gathered %d unresolved symbols, %d local symbols and %d imports\n", unresolved_.size(), local_.size(), imports_.size());
			utils::Log::verbose("prepared %d symbols for export and %d symbols for static import\n", exports_.size(), staticImports_.size());
		}

		void BinaryGatherer::processArchive(binary::Archive& arch, const utils::Path& path)
		{
			binary::Archive::ImportMap& imports = arch.getImports();

			if(imports.size() > 0)
			{
				//
				// Import Library found!
				//
				for(binary::Archive::ImportMap::iterator it = imports.begin(); it != imports.end(); ++it)
				{
					processImport(it->second, path);
				}
			}

			binary::Archive::MemberMap& members = arch.getMembers();

			if(members.size() > 0)
			{
				bool bIsHybrid = false;
				bool bIsPureImport = false;

				if(imports.size() > 0) {
					std::string libBaseName = imports.begin()->first;

					int cntSeenImportMember = 0;
					int cntSeenNormalMember = 0;

					for(binary::Archive::MemberMap::iterator it = members.begin(); it != members.end(); ++it)
					{
						if(it->first.find(libBaseName) != std::string::npos)
							++cntSeenImportMember;
						else
							++cntSeenNormalMember;
					}

					if(cntSeenNormalMember == 0 && cntSeenImportMember > 0)
						bIsPureImport = true;
					else if(cntSeenImportMember > 0 && cntSeenNormalMember > 0)
						bIsHybrid = true;
				}

				if(bIsHybrid)
					utils::Log::verbose("hybrid library found: %s\n", arch.getFile()->getPath().get().c_str());

				for(binary::Archive::ImportMap::iterator it = imports.begin(); it != imports.end(); ++it)
					hybrids_[it->second.getLibraryName()] = LibraryPathHybridityPair(arch.getFile()->getPath(), bIsHybrid);

				if(!bIsPureImport)
				{
					//
					// Static Library
					//
					for(binary::Archive::MemberMap::iterator it = members.begin(); it != members.end(); ++it)
					{
						processHeader(it->second, path);
					}
				}
			}

			if(imports.empty() && members.empty())
				utils::Log::warning("%s doesn't contain either imports or members, is it damaged?\n", arch.getFile()->getPath().get().c_str());
		}

		void BinaryGatherer::processHeader(binary::FileHeader& hdr, const utils::Path& path)
		{
			//
			// This in any case are local symbols. And of course it may contain
			// unresolved symbols, which we may match together after looking them up.
			//

			utils::Context& ctx = utils::Context::getContext();
			const binary::Symbol::IndexedSymbolMap& vec = hdr.getAllSymbols();

			//
			// Split up the symbols into local and unresolved ones.
			//
			for(binary::Symbol::IndexedSymbolMap::const_iterator it = vec.begin(); it != vec.end(); ++it)
			{
				//
				// Symbols may be invalid if they are dummy symbols placed in the
				// vector to keep up indexing correctly.
				//
				//if(!it->isValid())
				//	continue;

				//
				// For now, we only want symbols with External storage class
				// (microsoft uses only 4 values for storage class, and external
				// is the only interesting one here, see PE/COFF Spec v8)
				//
				if(!(it->second.getStorageClass() == binary::Symbol::ClassExternal))
					continue;

				//
				// check for C++ symbols which should not be exported
				// like the "scalar deleting constructor"
				//
				if(it->second.getName().compare(0, 4, "??_G") == 0 || it->second.getName().compare(0, 4, "??_E") == 0)
					continue;

				//
				// These seem to be string constants. i saw one breaking the linker
				// if it is exported... argh...
				//
				if(it->second.getName().compare(0, 4, "??_C") == 0)
					continue;

				if(it->second.getSectionNumber() == binary::Symbol::SymbolUndefined) {
					static std::map<std::string, bool> unresolved_seen_;
					bool& ref = unresolved_seen_[it->second.getName()];

					if(!ref) {
						ref = true;

						unresolved_.insert(SymbolUsageMap::value_type(it->second.getName(), SymbolUsagePair(it->second, path)));
					}
				} else if(it->second.getSectionNumber() == binary::Symbol::SymbolAbsolute)
					; // for now ignore
				else if(it->second.getSectionNumber() == binary::Symbol::SymbolDebug)
					; // for now ignore
				else {
					static std::map<std::string, bool> locals_seen_;
					bool& ref = locals_seen_[it->second.getName()];

					if(!ref)
					{
						ref = true;

						local_.insert(SymbolUsageMap::value_type(it->second.getName(), SymbolUsagePair(it->second, path)));

						//
						// Export symbol if linking shared.
						//
						switch(binary::File::getType(hdr.getFile()))
						{
						case binary::File::TypeLibrary:
							maybeExports_.push_back(it->second);
							break;
						case binary::File::TypeObject:
							exports_.push_back(it->second);
							break;
						default:
							throw utils::Exception("wrong file type while processing header!");
						}
					}
				}
			}

			//
			// After gathering from this header, lookup any implicit dependencies.
			// Don't use getSection but iterate ourselves, since this is faster.
			//
			binary::Section::IndexedSectionMap& sec = hdr.getSections();

			for(binary::Section::IndexedSectionMap::iterator it = sec.begin(); it != sec.end(); ++it)
			{
				if((it->second.getCharacteristics() & binary::Section::CharLinkInfo) && it->second.getName() == ".drectve")
				{
					try {
						binary::DirectiveSection dir(it->second);
						const binary::DirectiveSection::DirectiveVector& directives = dir.getDirectives();

						for(binary::DirectiveSection::DirectiveVector::const_iterator d = directives.begin(); d != directives.end(); ++d)
						{
							std::string::size_type pos = d->find("DEFAULTLIB", 1);

							if(pos != std::string::npos)
							{
								pos += 11; // D,E,F,A,U,L,T,L,I,B,: = 11 chars
								std::string lib = d->substr(pos);
								
								while((pos = lib.find('"')) != std::string::npos)
									lib.replace(pos, 1, "");

								utils::Path pth = ctx.lookupLibrary(lib, true);

								//
								// pth must be native allready, no need to convert!
								//
								bool& ref = implicits_[pth];

								if(!ref)
								{
									ref = true;
									utils::Log::verbose("implicit library: %s\n", pth.get().c_str());

									//
									// TODO: when the loader is added, this libraries should be
									// added to ObjectsLibraries
									//

									//
									// process directly recursively (if not processed allready).
									//
									bool& proc = processed_[pth];
									if(!proc)
									{
										proc = true;

										bool bIsSystem = false;
										if(!ctx.getGatherSystem())
										{
											//
											// skip if path is inside of a system path.
											//
											for(utils::PathVector::iterator sys = ctx.getSysLibraryPaths().begin(); sys != ctx.getSysLibraryPaths().end(); ++sys)
											{
												sys->toNative();
												if(pth.get().find(sys->get()) != std::string::npos)
													bIsSystem = true;
											}
										}

										if(!bIsSystem)
										{
											utils::MappedFile f(pth, utils::ModeRead);

											//
											// This shouldn't happen, if it does, implement object handling too.
											//
											if(binary::File::getType(&f) != binary::File::TypeLibrary)
												throw utils::Exception("implicit %s is not a library!", pth.get().c_str());

											//
											// We need this intermediate variable for gcc 3.3 to be able
											// to pass it as reference.
											//
											binary::Archive arch(&f);
											processArchive(arch, pth);
										}
									}
								}
							}
						}
					} catch(const utils::Exception& e) {
						utils::Log::warning("error while processing directives section: %s\n", e.what());
					}
				}
			}
		}

		void BinaryGatherer::processImport(binary::Import& imp, const utils::Path& path)
		{
			//
			// These are import symbols, which may resolve unresolved symbols
			// looked up by the processHeader function. Still we need to remember
			// those imports and resolve things, after lookup of *everything* is
			// finished.
			//

			imports_.insert(ImportUsageMap::value_type(imp.getSymbolName(), ImportUsagePair(imp, path)));
		}

		void BinaryGatherer::resolveSymbols()
		{
			for(SymbolUsageMap::iterator it = unresolved_.begin(); it != unresolved_.end(); ++it)
			{
				if(it->second.first.getName().compare(0, 6, "__imp_") == 0)
				{
					//
					// request for an imported symbol
					//
					std::string undecorated = it->second.first.getName().substr(6);	// strip __imp_
					ImportUsageMap::iterator imp = imports_.find(undecorated);

					if(imp != imports_.end())
					{
						//
						// Found import with matching name, generate
						// an import through the loader generator.
						//
						LibraryPathHybridityPair& pair = hybrids_[imp->second.first.getLibraryName()];
						loadedImports_.insert(ImportHybridityMap::value_type(PathImportPair(pair.first, imp->second.first), pair.second));
					} else {
						SymbolUsageMap::iterator sym = local_.find(undecorated);

						if(sym != local_.end())
						{
							//
							// found local symbol, but requested as import.
							// parity needs to generate an import trampoline for this!
							//
							staticImports_.push_back(sym->second.first);
						}
					}
				} else {
					//
					// request for a local symbol.
					//
					SymbolUsageMap::iterator sym = local_.find(it->second.first.getName());

					if(sym == local_.end())
					{
						//
						// ok, no local symbol with that name. this means
						// its save for functions to get loaded, and its a K.O.
						// for data symbols!
						//
						// WARNING: The following if is commented out, because:
						// when using inheritance in C++ it is possible that a
						// function is requested as DATA symbol, i think this is
						// because of it beeing in the vtable (seems to happen
						// only for virtuals...)
						//
						// UPDATE: since svn revision 95 there is a binary decoder
						// which enables binary data insertion into unlinked object
						// data. This makes it possible to patch the accesses to
						// global data, and make any unresolved data symbol point
						// to a generated import pointer, which is appended to the
						// corresponding object file.
						// But still there are some problems:
						//  *) if the object comes from a library, the whole library
						//     must be unpacked somewhere, and after patching the
						//     relevant objects, all objects that come from a library
						//     have to be packed together again. If the file is an
						//     object it has to be copied somewhere prior to patching
						//     so that the original objects don't change...
						//  *) detecting accesses to the relevant symbols is pretty
						//     straight forward: simply look for relocations to data
						//     symbols where there is no local, but only an __imp_
						//     version.
						//  *) patching has to be done depending on the instruction
						//     that accesses the variable. also the accessing instruction
						//     may has to be changed for things to work out again. This
						//     would mean checking for every possible instruction taking
						//     m16/32 operands
						//

						//if(it->second.first.getType() != binary::Symbol::ComplexFunction)
						//{
							//
							// This cannot be done, because it may be (it is the default),
							// that system libraries are not inspected, so we cannot be sure
							// wether we see the whole picture.
							//
							//utils::Log::warning("the data symbol %s was requested without\n", it->second.first.getName().c_str());
							//utils::Log::warning("    dll-import, but is provided only through a shared\n");
							//utils::Log::warning("    library! this *will* break linking for sure.\n");
						//} else {
							ImportUsageMap::iterator imp = imports_.find(it->second.first.getName());

							if(imp != imports_.end())
							{
								//
								// found and import for this symbol, so it will be loaded
								// through the LoaderGenerator (if enabled)
								//
								LibraryPathHybridityPair& pair = hybrids_[imp->second.first.getLibraryName()];
								loadedImports_.insert(ImportHybridityMap::value_type(PathImportPair(pair.first, imp->second.first), pair.second));
							}
						//}
					}
				}
			}
		}
	}
}

