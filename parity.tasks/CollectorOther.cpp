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

#include "CollectorOther.h"
#include "CollectorStubs.h"

#include "BinaryGatherer.h"

#include <Path.h>
#include <Context.h>
#include <Configuration.h>
#include <Timing.h>
#include <MappedFile.h>
#include <Log.h>
#include <Environment.h>
#include <Threading.h>
#include <Statistics.h>

#include <CoffObject.h>
#include <CoffFileHeader.h>
#include <CoffDirectiveSection.h>

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

namespace parity
{
	namespace tasks
	{
		typedef std::pair<utils::Path, bool> ConfigFilePair;
		typedef std::vector<ConfigFilePair> ConfigFileVector;
		typedef std::map<utils::Path, bool> LoadedFileMap;

		void runConfigurationLoading(int argc, char * const * argv)
		{
			utils::Context& context = utils::Context::getContext();
			utils::Timing::instance().start("Configuration loading");
			ConfigFileVector files;

			//
			// On Win32:
			//  1) try next to exe file
			//
			// On All others (UNIX like):
			//  1) try in configured PARITY_SYSCONFDIR/etc/
			//
			// And everywhere:
			//  2) try in current directory
			//  3) in path set by envoironment variable PARITY_CONFIG
			//
		#if defined(_WIN32) && !defined(PARITY_SYSCONFDIR)
			char fnBuffer[1024];
			GetModuleFileName(GetModuleHandle(NULL), fnBuffer, 1024);

			files.push_back(ConfigFileVector::value_type(utils::Path(fnBuffer).base(), true));
		#else
			utils::Path pth(PARITY_SYSCONFDIR);
			pth.toNative();
			files.push_back(ConfigFileVector::value_type(pth, true));
		#endif

			//
			// all optional and partial configuration files need to go after
			// the main configuration, so they don't get overridden by defaults.
			//
			files.push_back(ConfigFileVector::value_type(utils::Path("."), true));
			files.push_back(ConfigFileVector::value_type(utils::Path("."), false));
			files.push_back(ConfigFileVector::value_type(utils::Environment("PARITY_CONFIG").getPath(), false));

			//
			// This is set to true if at least one of the required
			// files has been loaded..
			//
			bool bLoaded = false;
			LoadedFileMap loaded;

			for(ConfigFileVector::iterator it = files.begin(); it != files.end(); ++it)
			{
				if(it->first.get().empty() || !it->first.exists())
					continue;

				utils::Path pth = it->first;

				if(pth.isDirectory())
					pth.append("parity.conf");

				pth.toNative();

				if(bLoaded && it->second)
					continue;

				if(loaded[it->first])
					continue;

				loaded[it->first] = true;

				if(pth.exists())
				{
					utils::MappedFile config(pth, utils::ModeRead);

					try {
						utils::Config::parseFile(context, config, argc, argv);
					} catch(const utils::Exception& e) {
						utils::Log::error("while parsing %s: %s\n", pth.get().c_str(), e.what());
						exit(1);
					}

					if(it->second)
						bLoaded = true;
				}
			}

			if(!bLoaded)
			{
				utils::Log::error("cannot find configuration in any of the following places. cannot continue!\n");
				for(ConfigFileVector::iterator it = files.begin(); it != files.end(); ++it)
				{
					if(!it->first.get().empty())
						utils::Log::error(" * %s\n", it->first.get().c_str());
				}
				exit(1);
			}

			//
			// Test for known bad configurations...
			//
			if(context.getGatherSystem() && context.getExportFromExe())
			{
				utils::Log::warning("There are known issues when gathering from system libraries, and exporting symbols from executables! Disabling export...\n");
				context.setExportFromExe(false);
			}

			utils::Timing::instance().stop("Configuration loading");
		}

		static void lookupParityRuntimeInclude()
		{
			utils::Context& ctx = utils::Context::getContext();
			utils::Log::verbose("trying to find suitable partity.runtime include directory...\n");

			if(ctx.getPCRTInclude().get().empty())
			{
				utils::Path location;

				#if defined(_WIN32) && !defined(PARITY_INCLUDEDIR)
					//
					// just try the default solution setup, and parallell to executable,
					// then give up, if not found.
					//
					char fnBuffer[1024];
					GetModuleFileName(GetModuleHandle(NULL), fnBuffer, 1024);
					
					location = utils::Path(fnBuffer);
					location = location.base();
					location = location.base();	// base 2 times to get rid of the "debug" or "release" dirs.
					location.append("parity.runtime");

					if(!location.exists() || !location.isDirectory()) {
						location = utils::Path(".");
						location.append("parity.runtime");
					}
				#else
					location = utils::Path(PARITY_INCLUDEDIR);
					location.toNative();
					location.append("parity.runtime");
				#endif

				if(location.exists() && location.isDirectory()) {
					//
					// i know this is slow, but vector can't add at front.
					//
					utils::PathVector vec;
					vec.push_back(location);
					vec.insert(vec.end(), ctx.getSysIncludePaths().begin(), ctx.getSysIncludePaths().end());
					ctx.setSysIncludePaths(vec);
				} else
					throw utils::Exception("cannot find parity.runtime include directory!");
			} else {
				//
				// i know this is slow, but vector can't add at front.
				//
				utils::PathVector vec;
				vec.push_back(ctx.getPCRTInclude());
				vec.insert(vec.end(), ctx.getSysIncludePaths().begin(), ctx.getSysIncludePaths().end());
				ctx.setSysIncludePaths(vec);
			}
		}

		void runCompilerStage()
		{
			utils::Threading threading;
			utils::Context& context = utils::Context::getContext();

			if(context.getBackendType() == utils::ToolchainMicrosoft) {
				try {
					lookupParityRuntimeInclude();
				} catch(const utils::Exception&) {
					utils::Log::error("cannot find suitable parity.runtime include directory!\n");
					exit(1);
				}
			}
			//
			// First task is the Dependency Tracker (in background)
			// With the GNU Backend, dependency tracking is a real side
			// effect of compilation.
			//
			if(context.getDependencyTracking() && !context.getSources().empty()
				&& context.getFrontendType() == utils::ToolchainInterixGNU && context.getBackendType() != utils::ToolchainInterixGNU)
			{

				if(context.getDependencyOnly())
				{
					//
					// run without threading to save some time
					//
					tasks::TaskStubs::runDependencyTracking(0);
					utils::Log::verbose("only dependency tracking requested, exiting.\n");
					exit(0);
				} else {
					threading.run(tasks::TaskStubs::runDependencyTracking, 0, true);
				}
			}

			//
			// compiler runs parallel to depdendency tracking in the best case
			// but itself in the foreground, since we need to block on it anyway!
			//
			if(!context.getSources().empty())
			{
				//threading.run(TaskStubs::runCompiler);
				if(TaskStubs::runCompiler(0) != 0)
				{
					utils::Log::error("error executing compiler!\n");
					exit(1);
				}
			}

			if(context.getCompileOnly())
			{
				utils::Log::verbose("only compilation requested, exiting.\n");
				threading.synchronize();
				exit(0);
			}

			if(context.getPreprocess())
			{
				utils::Log::verbose("only preprocessing requested, exiting.\n");
				threading.synchronize();
				exit(0);
			}
		}

		static void lookupParityRuntimeLibrary()
		{
			utils::Context& ctx = utils::Context::getContext();
			utils::Log::verbose("trying to find suitable partity.runtime library...\n");

			if(ctx.getPCRTLibrary().get().empty())
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
					location.append("parity.runtime.lib");
				#else
					location = utils::Path(PARITY_LIBDIR);
					location.toNative();
					location.append("libparity_parity.runtime.a");

					if(!location.exists())
					{
						//
						// try from non-installed structure?
						//
						utils::Log::warning("TODO: non-installed runtime library search.");
					}
				#endif

				if(location.exists())
					ctx.getObjectsLibraries().push_back(location);
				else
					throw utils::Exception("cannot lookup parity.runtime library, please set in configuration!");
			} else {
				ctx.getObjectsLibraries().push_back(ctx.getPCRTLibrary());
			}
		}

		void runLinkerStage()
		{
			utils::Context& ctx = utils::Context::getContext();
			utils::Threading threading;
			//
			// The first part of linking is gathering all the symbols for all
			// the given libraries and objects. As a side effect this decides
			// wether to link debugable. This must be done in foreground.
			// This also cannot be swapped to a threading function that is
			// called directly (like done with some other tasks), because
			// this is the only task where data needs to be taken from after
			// processing.
			//

			if(ctx.getBackendType() == utils::ToolchainInterixMS && ctx.getSubsystem() == utils::SubsystemPosixCui
				&& ctx.getSharedLink()) {
					utils::Log::error("cannot link shared libraries with POSIX subsystem!\n");
					exit(1);
			}

			if(ctx.getBackendType() == utils::ToolchainMicrosoft
				|| (ctx.getBackendType() == utils::ToolchainInterixMS && ctx.getSubsystem() != utils::SubsystemPosixCui)) {
				utils::Timing::instance().start("Symbol gathering");

				binary::Symbol::SymbolVector exportedSymbols;
				binary::Symbol::SymbolVector staticImports;
				binary::Symbol::SymbolVector localSymbols;
				tasks::BinaryGatherer::ImportHybridityMap loadedImports;

				try {
					tasks::BinaryGatherer gatherer;
					gatherer.doWork();

					exportedSymbols	= gatherer.getExportedSymbols();
					staticImports	= gatherer.getStaticImports();
					loadedImports	= gatherer.getLoadedImports();
					localSymbols	= gatherer.getLocalSymbols();

				} catch(const utils::Exception& e) {
					utils::Log::error("while gathering from binaries: %s\n", e.what());
					exit(1);
				}

				//
				// we can't get here if Subsystem = POSIX, so we don't need to check...
				//
				if(ctx.getSharedLink() && ctx.getBackendType() == utils::ToolchainInterixMS) {
					//
					// Need to eventually generate a DllMain@12
					//
					bool bHaveDllMain = false;
					for(binary::Symbol::SymbolVector::iterator it = exportedSymbols.begin(); it != exportedSymbols.end(); ++it) {
						if(it->getName().compare(0, strlen("_DllMain@12"), "_DllMain@12") == 0) {
							utils::Log::verbose("found existing DllMain function, skipping generation!\n");
							bHaveDllMain = true;
						}
					}

					if(!bHaveDllMain) {
						utils::Log::verbose("generating DllMain stub function\n");

						binary::Object obj;
						binary::FileHeader& hdr = obj.getHeader();
						binary::Section& txtSect = hdr.addSection(".text");

						txtSect.setCharacteristics( binary::Section::CharAlign16Bytes | binary::Section::CharMemoryExecute | binary::Section::CharMemoryRead | binary::Section::CharContentCode);

						binary::Symbol& dllmain = hdr.addSymbol("_DllMain@12");
						txtSect.markSymbol(dllmain);
						dllmain.setStorageClass(binary::Symbol::ClassExternal);

						//
						// This means:
						//   mov eax, 1
						//   ret 0Ch
						// which should be enough for a dllmain stub
						//
						const unsigned char dllmain_data[] = { 0xB8, 0x01, 0x0, 0x0, 0x0, 0xC2, 0x0C, 0x0 };
						txtSect.addData(dllmain_data, sizeof(dllmain_data));

						txtSect.padSection();

						utils::MemoryFile mem;
						obj.update(mem);

						utils::Path f = utils::Path::getTemporary(".parity.dllmain.XXXXXX.o");
						mem.save(f);

						ctx.getObjectsLibraries().push_back(f);
						ctx.getTemporaryFiles().push_back(f);
					}
				}

				utils::Timing::instance().stop("Symbol gathering");

				//
				// The second part is generating the exports for symbol exports
				// by a DLL (if linking shared). This can be done in background.
				//
				if(!exportedSymbols.empty())
					threading.run(TaskStubs::runMsExportGenerator, &exportedSymbols, false);

				//
				// generate the symbol table.
				//
				if(!localSymbols.empty() && ctx.getGenerateSymbols())
					threading.run(TaskStubs::runMsSymbolTableGenerator, &localSymbols, false);

				//
				// The third part is generating the import symbols for all static
				// libraries involved in linking. This can be done in Background too.
				// This should only generate symbols for things requested from
				// somewhere.
				//
				if(!staticImports.empty())
					threading.run(TaskStubs::runMsStaticImportGenerator, &staticImports, false);

				if(ctx.getBackendType() == utils::ToolchainMicrosoft) {
					//
					// The fourth part is generating the binaries for the shared library
					// loader. This can be done in Background, since it does not require
					// any information from the exports generator and the import generator.
					//
					// This is run *always* because parts of parity.runtime depend on
					// code from parity.loader. this means that it must be present
					// always, even if it's not required.
					//
					threading.run(TaskStubs::runMsLoaderGenerator, &loadedImports, false);

					//
					// Now we generate a new Entrypoint function, that initializes the
					// parity.runtime library (sets up exception handling, etc).
					//
					if(ctx.getGeneratePCRTEntry())
						threading.run(TaskStubs::runMsPcrtInitEntryGenerator, NULL, false);

					//
					// need the parity.runtime library. for this to work, one need to set
					// the system include directories to include the parity.runtime include
					// directory as first one.
					//
					try {
						lookupParityRuntimeLibrary();
					} catch(const utils::Exception&) {
						utils::Log::error("cannot find suitable parity.runtime library!\n");
						exit(1);
					}
				}

				//
				// The last part finally is linking itself. Before doing this, all
				// background tasks must be synchronized, since the linker needs
				// all created files and informations.
				//
				threading.synchronize();
			}

			if(TaskStubs::runLinker(0) != 0)
			{
				utils::Log::error("cannot run linker!\n");
				exit(1);
			}
		}
	}
}

