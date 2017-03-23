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
#include <Color.h>
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
	parity::utils::Context::getContext().setColored(true);
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

	parity::utils::Color col(parity::utils::Context::getContext().getColorMode());
	parity::utils::Log::verbose("%s", col.green("... done gathering information.\n").c_str());

	//
	// now somehow display the information....
	//
	for(parity::inspector::InspectorLibraryVectorMap::iterator it = files.begin(); it != files.end(); ++it)
	{
		if(!parity::inspector::gShowLddLike)
			std::cout << "=== Inspection of " << it->first.get() << " ===" << std::endl;
		if(parity::inspector::gShowLddLike && parity::inspector::gFilesToProcess.size() > 1)
			std::cout << col.green(it->first.get()) << ":" << std::endl;

		parity::inspector::DisplayItem(&it->second, true);

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
			parity::utils::Color col(parity::utils::Context::getContext().getColorMode());
			for(utils::PathVector::const_iterator it = vec.begin(); it != vec.end(); ++it)
			{
				try {
					if(!ProcessFile(*it, map[*it]))
						return false;
				} catch(const utils::Exception& ex)
				{
					utils::Log::error("while processing %s: %s\n", col.yellow(it->get()).c_str(), col.red(ex.what()).c_str());
					return false;
				}
			}
			return true;
		}

		static InspectorLibraryVectorMap children_cache;

		bool ProcessFile(const utils::Path& pth, InspectorLibraryVector& target)
		{
			static unsigned int indent = 0;
			parity::utils::Color col(parity::utils::Context::getContext().getColorMode());
			utils::MappedFileCache& cache = utils::MappedFileCache::getCache();
			utils::MappedFile& mapping = cache.get(pth, utils::ModeRead);
			
			if(binary::File::getType(&mapping) != binary::File::TypeImage)
			{
				utils::Log::error(col.red("%s: not a PE32 image file.\n").c_str(), pth.get().c_str());
				return false;
			}

			binary::Image img(&mapping);
			const binary::Section::IndexedSectionMap& sections = img.getHeader().getSections();

			const char*				genImageName	= 0;
			utils::SubsystemType	genSubsystem	= utils::SubsystemUnknown;
			utils::PathVector		genRunPaths;
			InspectorLibraryVector	genLibs;
			bool					genSectionFound	= false;

			++indent;
			for(unsigned int in = 0; in < indent; ++in) utils::Log::verbose("  ");
			utils::Log::verbose("%s:\n", col.green(pth.file()).c_str());

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

					for(unsigned int in = 0; in < indent; ++in) utils::Log::verbose("  ");
					utils::Log::verbose("  * found image name %s\n", col.green(genImageName).c_str());
					for(unsigned int in = 0; in < indent; ++in) utils::Log::verbose("  ");
					utils::Log::verbose("  * found runpaths at location %p\n", genRunPathPtr);

					if(genTablePtr && *genTablePtr) {
						for(unsigned int in = 0; in < indent; ++in) utils::Log::verbose("  ");
						utils::Log::verbose("  * found generated import table at %p\n", reinterpret_cast<void*>(*genTablePtr));
					} else {
						for(unsigned int in = 0; in < indent; ++in) utils::Log::verbose("  ");
						utils::Log::verbose("  * image has no import dependencies!\n");
					}

					for(unsigned int in = 0; in < indent; ++in) utils::Log::verbose("  ");
					utils::Log::verbose("  * found subsystem type %d (%s)\n", genSubsystem, col.magenta(utils::Context::getContext().printable(genSubsystem)).c_str());

					continue;
				}
			}

			//
			// Do native library lookup here...
			//
			binary::ImportDirectory imp(img);

			#ifdef _WIN32
			#define strdup _strdup
			#endif

			for(binary::ImportDirectory::NativeImportVector::const_iterator impit = imp.getNativeImports().begin(); impit != imp.getNativeImports().end(); ++impit) {
				InspectorLibraries item;
				
				item.name = impit->LibraryName;
				item.native = true;
				
				for(binary::ImportDirectory::NativeImportSymbolVector::const_iterator symit = impit->ImportedSymbols.begin(); symit != impit->ImportedSymbols.end(); ++symit) {
					InspectorImports ii;

					ii.import = NULL;
					ii.name = symit->Name;
					ii.ordinal = symit->Ordinal;

					item.imports.push_back(ii);
				}

				genLibs.push_back(item);
			}

			#ifdef _WIN32
			#undef strdup
			#endif

			//
			// Here we have all information converted to something usefull:
			//  *) Image Name
			//  *) Run Paths
			//  *) Library Table
			//     *) Import Table
			//

			if(genRunPaths.empty()) {
				for(unsigned int in = 0; in < indent; ++in) utils::Log::verbose("  ");
				utils::Log::verbose("  * %s\n", col.magenta("no runpaths hardcoded.").c_str());
			} else {
				for(unsigned int in = 0; in < indent; ++in) utils::Log::verbose("  ");
				utils::Log::verbose("  * using the following runpaths:\n");
			}

			for(utils::PathVector::iterator it = genRunPaths.begin(); it != genRunPaths.end(); ++it) {
				for(unsigned int in = 0; in < indent; ++in) utils::Log::verbose("  ");
				utils::Log::verbose("    * %s\n", col.yellow(it->get()).c_str());
			}

			//
			// SPECIAL case: If we're in raw output mode, we just output
			// all the information gathered until here, and return, without
			// looking up children.
			//
			if(gDumpRaw) {
				#ifdef _WIN32
					// rpath output format not compatible with windows paths.
					std::cout << "--raw is not currently supported in native win32 builds of parity!" << std::endl;
					return false;
				#endif

				// soname
				std::cout << pth.file() << ";";

				// rpath
				bool first = true;
				for(utils::PathVector::iterator it = genRunPaths.begin(); it != genRunPaths.end(); ++it) {
					it->toNative();
					
					if(!first) { std::cout << ":"; }
					if(first) { first = false; }

					std::cout << it->get();
				}

				std::cout << ";";

				// needed
				first = true;
				for(InspectorLibraryVector::iterator it = genLibs.begin(); it != genLibs.end(); ++it) {
					if(!first) { std::cout << ","; }
					if(first) { first = false; }

					std::cout << it->name;
				}

				std::cout << std::endl;

				return true;
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
					utils::Log::verbose(col.red("detected circle: %s is allready on the stack. trace of circle:\n").c_str(), col.magenta(pth.get()).c_str());

					for(CirclePreventionVector::iterator subit = circleStack.begin(); subit != circleStack.end(); ++subit)
					{
						utils::Log::verbose(col.red("  * %s\n").c_str(), col.magenta(subit->get()).c_str());
					}

					/*
					InspectorLibraries item;
					item.name = "<circle omitted> (" + pth.get() + " already on stack)";

					target.push_back(item);
					*/
					--indent;
					return false;
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
					for(unsigned int in = 0; in < indent; ++in) utils::Log::verbose("  ");
					utils::Log::verbose("  * %s: %s\n", col.magenta("cannot find dependency").c_str(), col.red(it->name).c_str());
				} else {
					//
					// we want the image base of it->file too!
					//
					utils::MappedFile& mapped = cache.get(it->file, utils::ModeRead);
					binary::Image img(&mapped);

					InspectorLibraryVectorMap::iterator ch = children_cache.find(it->file);
					it->base = img.getOptionalHeader().getImageBase();

					if(ch != children_cache.end()) {
						it->children = &ch->second;
						for(unsigned int in = 0; in < indent; ++in) utils::Log::verbose("  ");
						utils::Log::verbose("  * using cached dependency tree for %s\n", col.green(it->file.get()).c_str());
					} else {
						if((gShowLddLike && !gShowLddFlat) || !gShowLddLike) {
							InspectorLibraryVector& ch = children_cache[it->file];

							if(!ProcessFile(it->file, ch)) {
								children_cache.erase(it->file);
							} else {
								it->children = &ch;
							}
						}

					}
				}
			}

			cache.close(pth);

			target.insert(target.end(), genLibs.begin(), genLibs.end());

			//
			// finally, pop ourselfs from the stack.
			//
			circleStack.pop_back();
			--indent;

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

		void DisplayItem(const InspectorLibraryVector* vec, bool top)
		{
			static int level = 0;
			static std::map<std::string, bool> unique;
			static std::map<std::string, bool> circular_check;
			parity::utils::Color col(parity::utils::Context::getContext().getColorMode());

			if(top) {
				top = 0;
				unique.clear();
				circular_check.clear();
			}

			++level;

			for(InspectorLibraryVector::const_iterator it = vec->begin(); it != vec->end(); ++it)
			{
				bool& bProc = circular_check[it->name];
				bool& bUnique = unique[it->name];

				if(gShowLddLike) {
					if(!bUnique) {
						bUnique = true;
						std::ostringstream oss;
						if(!it->file.get().empty()) {
							oss << " (0x" << std::hex << it->base << ")";
						}
						std::cout << "\t" << it->name << (it->native ? col.red("*") : "" ) << " => " << (it->file.get().empty() ? "not found" : (gShortFormat ? "found" : it->file.get())) << col.yellow(oss.str()) << std::endl;
					}
				} else {
					indent(level);
					std::cout << col.yellow("* ") << col.green(it->name) << (it->native ? col.red("*").c_str() : "" ) << ( bProc ? col.red(" {circle}") : "" ) << col.cyan(" (") << (it->file.get().empty() ? col.red("not found") : (gShortFormat ? col.green("found") : col.green("found, ") + col.magenta(it->file.get())));
					std::cout << ", direct dep.: " << ( it->children ? it->children->size() : 0 ) << ", imports: " << it->imports.size() << col.cyan(")") << std::endl;
				}

				if(!gShowLddLike && gShowSymbols)
				{
					for(InspectorImportVector::const_iterator sym = it->imports.begin(); sym != it->imports.end(); ++sym)
					{
						indent(level + 2);

						if(it->native) {
							std::cout << col.blue("[NAT, ");
						} else {
							if(sym->import == 0x00000000)
								std::cout << col.blue("[CODE, ");
							else if(sym->import == reinterpret_cast<void*>(0xbaadf00d))
								std::cout << col.blue("[DATA, ");
							else
								std::cout << col.blue("[LAZY, ");
						}

						if(sym->ordinal)
							std::cout << col.blue("ORD] ");
						else
							std::cout << col.blue("NAME] ");

						if(sym->ordinal)
							std::cout << col.yellow("{") << sym->ordinal << col.yellow("} ") << std::endl;

						if(!(it->native && sym->ordinal)) {
							std::cout << sym->name << std::endl;
						}
					}
				}

				if((gShowLddLike && !gShowLddFlat) || !gShowLddLike) {
					if(it->children && !it->children->empty()) {
						if(!bUnique || gShowLddLike) {
							bUnique = true;

							if(!bProc) {
								bProc = true;
								DisplayItem(it->children, false);
								bProc = false;
							}
						} else {
							indent(level + 2);
							std::cout << col.yellow("{ ") << it->children->size() << col.yellow(" direct ") << ( it->children->size() > 1 ? col.yellow("dependencies") : col.yellow("dependency") ) << col.yellow(" already shown }") << std::endl;
						}
					}
				}
			}

			--level;
		}

	}
}

