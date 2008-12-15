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

#include <Log.h>
#include "Options.h"
#include "Inspector.h"

#include <CoffImage.h>
#include <CoffFile.h>
#include <CoffImportDirectory.h>

#include <Environment.h>
#include <Pointer.h>
#include <MappedFileCache.h>

#include <list>
#include <sstream>

//
// CONFIX:EXENAME('parity.inspector')
//

int main(int argc, char** argv)
{
	//
	// initialize from command line:
	//
	try {
		parity::options::CommandLine::process(argc - 1, &argv[1], parity::inspector::gInspectorOptionTable, 0);
	} catch(const parity::utils::Exception& ex)
	{
		parity::utils::Log::error("while processing commandline: %s\n", ex.what());
	}

	parity::inspector::InspectorLibraryVectorMap files;

	if(!parity::inspector::ProcessFileList(parity::inspector::gFilesToProcess, files))
		exit(1);

	//
	// now somehow display the information....
	//
	for(parity::inspector::InspectorLibraryVectorMap::iterator it = files.begin(); it != files.end(); ++it)
	{
		if(!parity::inspector::gShowLddLike)
			std::cout << "=== Inspection of " << it->first.get() << " ===" << std::endl;

		parity::inspector::DisplayItem(it->second);

		if(!parity::inspector::gShowLddLike)
			std::cout << std::endl;
	}

	return 0;
}

namespace parity 
{
	namespace inspector 
	{
		bool ProcessFileList(const utils::PathVector& vec, InspectorLibraryVectorMap& map)
		{
			for(utils::PathVector::const_iterator it = vec.begin(); it != vec.end(); ++it)
			{
				try {
					if(!ProcessFile(*it, map[*it]))
						return false;
				} catch(const utils::Exception& ex)
				{
					utils::Log::error("while processing %s: %s\n", it->get().c_str(), ex.what());
					return false;
				}
			}
			return true;
		}

		bool ProcessFile(const utils::Path& pth, InspectorLibraryVector& target)
		{
			utils::MappedFileCache& cache = utils::MappedFileCache::getCache();
			utils::MappedFile& mapping = cache.get(pth, utils::ModeRead);
			
			if(binary::File::getType(&mapping) != binary::File::TypeImage)
			{
				utils::Log::error("%s: not a PE32 image file.\n", pth.get().c_str());
				return false;
			}

			binary::Image img(&mapping);
			const binary::Section::IndexedSectionMap& sections = img.getHeader().getSections();

			const char*				genImageName	= 0;
			utils::SubsystemType	genSubsystem	= utils::SubsystemUnknown;
			utils::PathVector		genRunPaths;
			InspectorLibraryVector	genLibs;
			bool					genSectionFound	= false;

			utils::Log::verbose("%s:\n", pth.file().c_str());

			for(binary::Section::IndexedSectionMap::const_iterator it = sections.begin(); it != sections.end(); ++it)
			{
				if(it->second.getName() == NAMEOF_SECTION_POINTERS)
				{
					InspectorPointerLayoutVA* pointers = 0;
					const char* genRunPathPtr = 0;
					unsigned int* genTablePtr = 0;

					genSectionFound = true;

					pointers = MAKEPTR(InspectorPointerLayoutVA*, mapping.getBase(), it->second.getPointerToRawData());

					genImageName	= reinterpret_cast<const char*>(img.getHeader().getPointerFromVA(pointers->name));
					genRunPathPtr	= reinterpret_cast<const char*>(img.getHeader().getPointerFromVA(pointers->rpaths));
					genTablePtr		= reinterpret_cast<unsigned int*>(img.getHeader().getPointerFromVA(pointers->table));
					genSubsystem	= pointers->subsystem;

					if(genTablePtr && *genTablePtr)
						genLibs		= InspectorLayoutToLibraries(reinterpret_cast<InspectorLibrariesPointerLayoutVA*>(img.getHeader().getPointerFromVA(*genTablePtr)), img.getHeader());

					if(genRunPathPtr)
						genRunPaths	= ConvertRunPaths(genRunPathPtr);

					utils::Log::verbose("  * found image name %s\n", genImageName);
					utils::Log::verbose("  * found runpaths at location %p\n", genRunPathPtr);

					if(genTablePtr && *genTablePtr)
						utils::Log::verbose("  * found generated import table at %p\n", reinterpret_cast<void*>(*genTablePtr));
					else
						utils::Log::verbose("  * image has no import dependencies!\n");

					utils::Log::verbose("  * found subsystem type %d (%s)\n", genSubsystem, utils::Context::getContext().printable(genSubsystem).c_str());

					continue;
				}
			}

			//
			// Do native library lookup here...
			//
			binary::ImportDirectory imp(img);

			for(binary::ImportDirectory::NativeImportVector::const_iterator impit = imp.getNativeImports().begin(); impit != imp.getNativeImports().end(); ++impit) {
				InspectorLibraries item;
				
				item.name = impit->LibraryName;
				item.native = true;
				
				for(binary::ImportDirectory::NativeImportSymbolVector::const_iterator symit = impit->ImportedSymbols.begin(); symit != impit->ImportedSymbols.end(); ++symit) {
					InspectorImports ii;

					ii.import = NULL;
					ii.library = NULL; /* redundant information, just there to keep binary compat */
					ii.name = strdup(symit->Name.c_str()); /* memory leak, i know */
					ii.ordinal = symit->Ordinal;

					item.imports.push_back(ii);
				}

				genLibs.push_back(item);
			}

			//
			// Here we have all information converted to something usefull:
			//  *) Image Name
			//  *) Run Paths
			//  *) Library Table
			//     *) Import Table
			//

			if(genRunPaths.empty())
				utils::Log::verbose("  * no runpaths hardcoded.\n");
			else
				utils::Log::verbose("  * using the following runpaths:\n");

			for(utils::PathVector::iterator it = genRunPaths.begin(); it != genRunPaths.end(); ++it)
			{
				utils::Log::verbose("    * %s\n", it->get().c_str());
			}

			//
			// recurse down a level, but prevent circles.
			// (i use list instead of stack to be able to
			// output kind of a stack trace, and be able to search)
			//
			typedef std::vector<utils::Path> CirclePreventionVector;
			static CirclePreventionVector circleStack;

			for(CirclePreventionVector::iterator it = circleStack.begin(); it != circleStack.end(); ++it)
			{
				if(*it == pth)
				{
					//
					// circle detected!
					//
					utils::Log::verbose("detected circle: %s is allready on the stack. trace of circle:\n", pth.get().c_str());

					for(CirclePreventionVector::iterator subit = circleStack.begin(); subit != circleStack.end(); ++subit)
					{
						utils::Log::verbose("  * %s\n", subit->get().c_str());
					}

					/*
					InspectorLibraries item;
					item.name = "<circle omitted> (" + pth.get() + " already on stack)";

					target.push_back(item);
					*/
					return true;
				}
			}

			//
			// no circle ;o)
			//
			circleStack.push_back(pth);

			//
			// some environements.
			//
			static utils::Environment envPath("PATH");
			static utils::Environment envLdLibraryPath("LD_LIBRARY_PATH");
			static utils::PathVector pathsPath = envPath.getPathVector();
			static utils::PathVector pathsLdLibraryPath = envLdLibraryPath.getPathVector();

			utils::PathVector paths;

			paths.insert(paths.end(), genRunPaths.begin(), genRunPaths.end());
			paths.insert(paths.end(), pathsLdLibraryPath.begin(), pathsLdLibraryPath.end());
			paths.insert(paths.end(), pathsPath.begin(), pathsPath.end());
			paths.push_back(utils::Path("."));

			//
			// process all children.
			//
			for(InspectorLibraryVector::iterator it = genLibs.begin(); it != genLibs.end(); ++it)
			{
				it->file = LookupFile(it->name, paths);
				
				if(it->file.get().empty() || !it->file.exists())
				{
					utils::Log::verbose("  * cannot find dependency: %s\n", it->name.c_str());
				} else {
					//
					// we want the image base of it->file too!
					//
					utils::MappedFile& mapped = cache.get(it->file, utils::ModeRead);
					binary::Image img(&mapped);

					it->base = img.getOptionalHeader().getImageBase();

					if((gShowLddLike && !gShowLddFlat) || !gShowLddLike)
						ProcessFile(it->file, it->children);
				}
			}

			cache.close(pth);

			target.insert(target.end(), genLibs.begin(), genLibs.end());

			//
			// finally, pop ourselfs from the stack.
			//
			circleStack.pop_back();

			return true;
		}

		InspectorLibraryVector InspectorLayoutToLibraries(const InspectorLibrariesPointerLayoutVA* ptr, binary::FileHeader& hdr)
		{
			//
			// ptr is an array of InspectorLibrariesPointerLayoutVA.
			//
			InspectorLibraryVector libraries;

			if(!ptr)
				return libraries;

			while(ptr->name != 0 && ptr->imports != 0)
			{
				InspectorImportsPointerLayoutVA* imp = reinterpret_cast<InspectorImportsPointerLayoutVA*>(hdr.getPointerFromVA(ptr->imports));
				InspectorImportVector imports;

				while(imp->name != 0)
				{
					InspectorImports item;

					item.name		= reinterpret_cast<const char*>(hdr.getPointerFromVA(imp->name));
					item.library	= reinterpret_cast<const char*>(hdr.getPointerFromVA(imp->library));
					item.import		= reinterpret_cast<const void*>(imp->import);
					item.ordinal	= imp->ordinal;

					imports.push_back(item);

					++imp;
				}

				InspectorLibraries library;

				library.name	= reinterpret_cast<const char*>(hdr.getPointerFromVA(ptr->name));
				library.imports	= imports;
				library.native	= false;

				libraries.push_back(library);

				++ptr;
			}

			return libraries;
		}

		utils::PathVector ConvertRunPaths(const char* rpaths)
		{
			utils::PathVector res;

			while(rpaths && *rpaths != '\0')
			{
				res.push_back(utils::Path(rpaths));

				while(*rpaths != '\0')
					++rpaths;

				++rpaths;
			}

			return res;
		}

		utils::Path LookupFile(const std::string& file, utils::PathVector& vec)
		{
			for(utils::PathVector::iterator it = vec.begin(); it != vec.end(); ++it)
			{
				it->toNative();

				if(it->exists())
				{
					utils::Path pth(*it);
					pth.append(file);

					if(pth.exists())
						return pth;
				}
			}

			return utils::Path();
		}

		static void indent(int level)
		{
			while(level-- > 0)
			{
				std::cout << " ";
			}
		}

		void DisplayItem(const InspectorLibraryVector& vec)
		{
			static int level = 0;
			static std::map<std::string, bool> unique;

			++level;

			for(InspectorLibraryVector::const_iterator it = vec.begin(); it != vec.end(); ++it)
			{
				if(gShowLddLike) {
					bool& bProc = unique[it->name];

					if(!bProc) {
						bProc = true;
						std::ostringstream oss;
						if(!it->file.get().empty()) {
							oss << " (0x" << std::hex << it->base << ")";
						}
						std::cout << "\t" << it->name << (it->native ? "*" : "" ) << " => " << (it->file.get().empty() ? "not found" : (gShortFormat ? "found" : it->file.get())) << oss.str() << std::endl;
					}
				} else {
					indent(level);
					std::cout << "* " << it->name << (it->native ? "*" : "" ) << " (" << (it->file.get().empty() ? "not found" : (gShortFormat ? "found" : "found, " + it->file.get()));
					std::cout << ", direct dep.: " << it->children.size() << ", imports: " << it->imports.size() << ")" << std::endl;
				}

				if(!gShowLddLike && gShowSymbols)
				{
					for(InspectorImportVector::const_iterator sym = it->imports.begin(); sym != it->imports.end(); ++sym)
					{
						indent(level + 2);

						if(it->native) {
							std::cout << "[NAT , ";
						} else {
							if(sym->import == 0x00000000)
								std::cout << "[CODE, ";
							else if(sym->import == reinterpret_cast<void*>(0xbaadf00d))
								std::cout << "[DATA, ";
							else
								std::cout << "[LAZY, ";
						}

						if(sym->ordinal)
							std::cout << "ORD ] ";
						else
							std::cout << "NAME] ";

						if(sym->ordinal)
							std::cout << "{" << sym->ordinal << "} ";

						if(!(it->native && sym->ordinal)) {
							std::cout << sym->name << std::endl;
						}
					}
				}

				if((gShowLddLike && !gShowLddFlat) || !gShowLddLike)
					DisplayItem(it->children);
			}

			--level;
		}

	}
}

