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

#include <Environment.h>
#include <Pointer.h>
#include <MappedFileCache.h>

#include <list>

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
		std::cout << "=== Inspection of " << it->first.get() << " ===" << std::endl;
		parity::inspector::DisplayItem(it->second);
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

			for(binary::Section::IndexedSectionMap::const_iterator it = sections.begin(); it != sections.end(); ++it)
			{
				if(it->second.getName() == NAMEOF_SECTION_POINTERS)
				{
					InspectorPointerLayoutVA* pointers = 0;
					const char* genRunPathPtr = 0;
					unsigned int* genTablePtr = 0;

					genSectionFound = true;

					pointers = MAKEPTR(InspectorPointerLayoutVA*, mapping.getBase(), it->second.getPointerToRawData());
					genImageName	= (const char*)img.getHeader().getPointerFromVA(pointers->name);
					genRunPathPtr	= (const char*)img.getHeader().getPointerFromVA(pointers->rpaths);
					genTablePtr		= (unsigned int*)img.getHeader().getPointerFromVA(pointers->table);
					genSubsystem	= pointers->subsystem;

					if(*genTablePtr)
						genLibs		= InspectorLayoutToLibraries((InspectorLibrariesPointerLayoutVA*)img.getHeader().getPointerFromVA(*genTablePtr), img.getHeader());

					genRunPaths		= ConvertRunPaths(genRunPathPtr);

					utils::Log::verbose("%s: found image name %s\n", pth.file().c_str(), genImageName);
					utils::Log::verbose("%s: found runpaths at location %p\n", pth.file().c_str(), genRunPathPtr);

					if(*genTablePtr)
						utils::Log::verbose("%s: found generated import table at %p\n", pth.file().c_str(), *genTablePtr);
					else
						utils::Log::verbose("%s: image has no import dependencies!\n", pth.file().c_str());

					utils::Log::verbose("%s: found subsystem type %d (%s)\n", pth.file().c_str(), genSubsystem, utils::Context::getContext().printable(genSubsystem).c_str());

					break;
				}
			}

			if(!genSectionFound)
				utils::Log::verbose("%s: image was not generated by parity, %s section not found!\n", pth.file().c_str(), NAMEOF_SECTION_POINTERS);

			//
			// Do native library lookup here...
			//

			//
			// Here we have all information converted to something usefull:
			//  *) Image Name
			//  *) Run Paths
			//  *) Library Table
			//     *) Import Table
			//

			if(genRunPaths.empty())
				utils::Log::verbose("%s: no runpaths hardcoded.\n", pth.file().c_str());
			else
				utils::Log::verbose("%s: using the following runpaths:\n", pth.file().c_str());

			for(utils::PathVector::iterator it = genRunPaths.begin(); it != genRunPaths.end(); ++it)
			{
				utils::Log::verbose(" * %s\n", it->get().c_str());
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
					utils::Log::warning("detected circle: %s is allready on the stack. trace of circle:\n", pth.get().c_str());

					for(CirclePreventionVector::iterator subit = circleStack.begin(); subit != circleStack.end(); ++subit)
					{
						utils::Log::verbose(" * %s\n", subit->get().c_str());
					}

					InspectorLibraries item;
					item.name = "<circle omitted> (" + pth.get() + " already on stack)";

					target.push_back(item);
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
					utils::Log::verbose("cannot find file: %s\n", it->name.c_str());
				} else {
					ProcessFile(it->file, it->children);
				}
			}

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
				InspectorImportsPointerLayoutVA* imp = (InspectorImportsPointerLayoutVA*)hdr.getPointerFromVA(ptr->imports);
				InspectorImportVector imports;

				while(imp->name != 0)
				{
					InspectorImports item;

					item.name		= (const char*)hdr.getPointerFromVA(imp->name);
					item.library	= (const char*)hdr.getPointerFromVA(imp->library);
					item.import		= (const void*)imp->import;
					item.ordinal	= imp->ordinal;

					imports.push_back(item);

					++imp;
				}

				InspectorLibraries library;

				library.name	= (const char*)hdr.getPointerFromVA(ptr->name);
				library.imports	= imports;

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

			++level;

			for(InspectorLibraryVector::const_iterator it = vec.begin(); it != vec.end(); ++it)
			{
				indent(level);
				std::cout << "* " << it->name << " (" << (it->file.get().empty() ? "not found" : (gShortFormat ? "found" : "found, " + it->file.get()));
				std::cout << ", direct dep.: " << it->children.size() << ", imports: " << it->imports.size() << ")" << std::endl;

				if(gShowSymbols)
				{
					for(InspectorImportVector::const_iterator sym = it->imports.begin(); sym != it->imports.end(); ++sym)
					{
						indent(level + 2);

						if(sym->import == 0x00000000)
							std::cout << "[CODE, ";
						else if(sym->import == (void*)0xbaadf00d)
							std::cout << "[DATA, ";
						else
							std::cout << "[LAZY, ";

						if(sym->ordinal)
							std::cout << "ORD ] ";
						else
							std::cout << "NAME] ";

						std::cout << sym->name << std::endl;
					}
				}

				DisplayItem(it->children);
			}

			--level;
		}

	}
}

