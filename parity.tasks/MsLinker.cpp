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

#include "MsLinker.h"
#include "MsCompiler.h"

#include <Exception.h>
#include <Context.h>
#include <Environment.h>
#include <Log.h>

#include <Path.h>
#include <CoffObject.h>
#include <CoffDirectiveSection.h>

#include <sstream>

namespace parity
{
	namespace tasks
	{
		MsLinker::MsLinker()
		{
			if(utils::Context::getContext().getObjectsLibraries().empty())
				throw utils::Exception("no objects or libraries given to link!");
		}

		static std::string symbolifyName(std::string str)
		{
			for(std::string::iterator it = str.begin(); it != str.end(); ++it)
				if(!isalnum(*it))
					*it = '_';

			return str;
		}

		void MsLinker::doWork()
		{
			utils::Context& ctx = utils::Context::getContext();
			utils::Task::ArgumentVector vec;

			//
			// Create the task
			//
			utils::Task task;

			if(ctx.getLinkerExe().get().empty()) {
				utils::Log::error("Linker executable not set, cannot link objects!\n");
				exit(1);
			}

			//
			// prepare argument vector
			//

			//
			// add pass default and through arguments.
			//
			if(ctx.getSharedLink())
				MsCompiler::vectorize(ctx.getLinkerSharedDefaults(), vec);
			else
				MsCompiler::vectorize(ctx.getLinkerDefaults(), vec);

			MsCompiler::vectorize(ctx.getLinkerPassThrough(), vec);

			//
			// determine wether to link a dll and output filenames.
			//
			utils::Path out;
			utils::Path exp;
			if(ctx.getSharedLink()) {
				vec.push_back("/DLL");

				if (ctx.getFrontendType() == utils::ToolchainMicrosoft
				 || !ctx.getOutImplib().get().empty())
				{
					utils::Path dll = ctx.getOutputFile();

					dll.toNative();
					if(dll.exists())
						dll.remove();

					exp.set(dll.base());
					exp.append(dll.file().substr(0, dll.file().rfind('.')) + ".exp");

					dll.toForeign();
					vec.push_back("/OUT:" + dll.get());

					out = dll;

					if (!ctx.getOutImplib().get().empty()) {
						utils::Path lib = ctx.getOutImplib();
						lib.toNative();
						if (lib.exists())
							lib.remove();

						exp.set(lib.base());
						exp.append(lib.file().substr(0, lib.file().rfind('.')) + ".exp");

						lib.toForeign();
						vec.push_back("/IMPLIB:" + lib.get());
					}
				} else {
					utils::Path dll = ctx.getOutputFile().get() + ".dll";
					utils::Path lib = ctx.getOutputFile();

					dll.toNative();
					lib.toNative();

					if(dll.exists())
						dll.remove();
					
					if(lib.exists())
						lib.remove();

					exp.set(lib.base());
					exp.append(lib.file().substr(0, lib.file().rfind('.')) + ".exp");

					dll.toForeign();
					lib.toForeign();

					//
					// allow overriding from commandline by explicitly passing /OUT:blah.dll
					// this is required during parity build in future versions where there
					// will be a ntdll.dll stub library (which of course _must_ be named
					// ntdll.dll) to be able to link against it without DDK.
					//
					if(ctx.getLinkerPassThrough().find("/OUT:") == std::string::npos) {
						vec.push_back("/OUT:" + dll.get());
						out = lib;
					} else {
						utils::Log::warning("output filename overridden. this will break manifest embedding (so do it manually)\n");
					}

					vec.push_back("/IMPLIB:" + lib.get());

					task.addFilter(lib.file(), false);
				}
			} else {
				out = ctx.getOutputFile();

				out.toNative();
				if(out.exists())
					out.remove();

				exp.set(out.base());
				exp.append(out.file().substr(0, out.file().rfind(".")) + ".exp");

				out.toForeign();
				vec.push_back("/OUT:" + out.get());

				std::string outFile = out.file();
				task.addFilter(outFile.substr(0, outFile.rfind(".")) + ".lib", false);
			}

			for(utils::SourceMap::iterator it = ctx.getSources().begin(); it != ctx.getSources().end(); ++it) {
				if (it->second == utils::LanguageModuleDefinition) {
					utils::Path pth(it->first);
					pth.toForeign();
					vec.push_back("/DEF:" + pth.get());
				}
			}

			//
			// determine wether to link debugable.
			//
			if(ctx.getDebugable())
				vec.push_back("/DEBUG");

			//
			// determine wether to force linking.
			//
			if(ctx.getForceLink()) {
				vec.push_back("/FORCE");

				//
				// Add filter to suppress warnings about duplicate symbols.
				//
				task.addFilter("LNK4006", false);
			}

			//
			// determine subsystem for which to link
			//
			switch(ctx.getSubsystem())
			{
			case utils::SubsystemPosixCui:
				vec.push_back("/SUBSYSTEM:POSIX");
				break;
			case utils::SubsystemWindowsCeGui:
				vec.push_back("/SUBSYSTEM:WINDOWSCE");
				break;
			case utils::SubsystemWindowsCui:
				vec.push_back("/SUBSYSTEM:CONSOLE");
				break;
			case utils::SubsystemWindowsGui:
				vec.push_back("/SUBSYSTEM:WINDOWS");
				break;
			default:
				throw utils::Exception("subsystem %d not supported!", ctx.getSubsystem());
			}

			//
			// add entry point if set.
			//
			if(ctx.getSharedLink()) {
				if(!ctx.getSharedEntryPoint().empty())
					vec.push_back("/ENTRY:" + ctx.getSharedEntryPoint());
			} else {
				if(!ctx.getEntryPoint().empty())
					vec.push_back("/ENTRY:" + ctx.getEntryPoint());
			}

			//
			// add library paths
			//
			for(utils::PathVector::iterator it = ctx.getLibraryPaths().begin(); it != ctx.getLibraryPaths().end(); ++it)
			{
				it->toForeign();

				vec.push_back("/LIBPATH:" + it->get());
			}

			//
			// add library paths from environment
			//
			utils::PathVector envPaths;

			utils::Environment envLdLibrary("LD_LIBRARY_PATH");
			utils::Environment envLibrary("LIBRARY_PATH");

			utils::PathVector vecLd = envLdLibrary.getPathVector();
			utils::PathVector vecLib = envLibrary.getPathVector();

			envPaths.insert(envPaths.end(), vecLd.begin(), vecLd.end());
			envPaths.insert(envPaths.end(), vecLib.begin(), vecLib.end());

			for(utils::PathVector::iterator it = envPaths.begin(); it != envPaths.end(); ++it)
			{
				it->toForeign();

				vec.push_back("/LIBPATH:" + it->get());
			}

			//
			// add system library paths (those set in configuration files)
			// those are not checked for bad paths.
			//
			for(utils::PathVector::iterator it = ctx.getSysLibraryPaths().begin(); it != ctx.getSysLibraryPaths().end(); ++it)
			{
				it->toForeign();
				vec.push_back("/LIBPATH:" + it->get());
			}

			//
			// finally add objects and libraries.
			//
			for(utils::PathVector::iterator it = ctx.getObjectsLibraries().begin(); it != ctx.getObjectsLibraries().end(); ++it)
			{
				it->toForeign();

				vec.push_back(it->get());
			}

			//
			// if there is not a single exported symbol when linking
			// shared, the import library will be missing, so we need
			// to make sure there is at laest one symbol!
			//
			if(ctx.getSharedLink())
			{
				const unsigned char dataEmpty[] = { 0x00, 0x00, 0x00, 0x00 };

				binary::Object obj;
				binary::FileHeader& hdr = obj.getHeader();

				binary::Section& rdataSect = hdr.addSection(".rdata");
				binary::Section& drectveSect = hdr.addSection(".drectve");

				rdataSect.setCharacteristics(binary::Section::CharAlign4Bytes | binary::Section::CharContentInitData | binary::Section::CharMemoryRead);
				drectveSect.setCharacteristics(binary::Section::CharAlign1Bytes | binary::Section::CharLinkInfo | binary::Section::CharLinkRemove);

				binary::DirectiveSection drectveWrapper(drectveSect);

				//
				// those many underscores are intentionally to (hopefully) make the
				// linker insert the import symbol quite at the beginning (if sorted
				// alphabetically), so that the BinaryGatherer can find it quickly.
				//
				std::string symName = "____parity_dummy_for_" + symbolifyName(out.file());

				binary::Symbol& symDataSym	= hdr.addSymbol("$DAT" + symName);
				binary::Symbol& symPtrSym	= hdr.addSymbol(symName);

				symDataSym.setStorageClass(binary::Symbol::ClassStatic);
				symPtrSym.setStorageClass(binary::Symbol::ClassExternal);

				rdataSect.markSymbol(symDataSym);
				rdataSect.addData(out.get().c_str(), out.get().length() + 1);
				rdataSect.padSection();

				rdataSect.markSymbol(symPtrSym);
				rdataSect.markRelocation(symDataSym, binary::Relocation::i386Direct32);
				rdataSect.addData(dataEmpty, sizeof(dataEmpty));

				drectveWrapper.addDirective("/EXPORT:" + symName + ",DATA");

				utils::Path f = utils::Path::getTemporary(".parity.linker.XXXXXX.o");
				f.toNative();

				utils::MemoryFile mem;
				obj.update(mem);
				mem.save(f);

				f.toForeign();
				vec.push_back(f.get());

				ctx.getTemporaryFiles().push_back(f);
			}

			//
			// set up any additional filters.
			//
			task.addFilter("LNK4049", false);
			task.addFilter("LNK4217", false);

			//
			// set stack and heap sizes.
			//
			std::ostringstream oss;
			oss << "/STACK:" << ctx.getStackReserve() << "," << ctx.getStackCommit();
			vec.push_back(oss.str());
			oss.str("");
			oss << "/HEAP:" << ctx.getHeapReserve() << "," << ctx.getHeapCommit();
			vec.push_back(oss.str());

			//
			// always create the manifest, as known file name
			//

			utils::Path mf;
			if(ctx.getSharedLink() && ctx.getOutImplib().get().empty())
				mf = out.get() + ".dll.mf";
			else
				mf = out.get() + ".mf";
			mf.toForeign();

			vec.push_back("/MANIFEST");
			vec.push_back("/MANIFESTFILE:" + mf.get());
			task.addFilter("/MANIFESTFILE", false);

			//
			// create command scripts
			//
			if(ctx.getUseCommandScripts())
				task.createCommandScript(vec);


			out.toNative();

			//
			// execute linker.
			//
			if(!task.execute(ctx.getLinkerExe(), vec)) {
				utils::Log::verbose("cleaning up after failed link!");
				out.remove();

				exp.remove();
				
				utils::Path mf;
				if(ctx.getSharedLink())
					mf = out.get() + ".dll.mf";
				else
					mf = out.get() + ".mf";

				mf.remove();

				throw utils::Exception("cannot execute: %s", ctx.getLinkerExe().get().c_str());
			}

			//
			// remove unwanted stuff.
			//
			out.toNative();

			if(!out.waitForAppearance())
				throw utils::Exception("missing output from linker: %s", out.get().c_str());

			if(ctx.getFrontendType() != utils::ToolchainMicrosoft)
			{
				std::string outFile = out.get();
				
				if(exp.waitForAppearance())
					ctx.getTemporaryFiles().push_back(exp);

				if(!ctx.getSharedLink())
				{
					//
					// executable may produce .lib file in some cases
					//
					utils::Path lib;
					lib.set(out.base());
					lib.append(out.file().substr(0, out.file().rfind('.')) + ".lib");
					if(lib.waitForAppearance())
						ctx.getTemporaryFiles().push_back(lib);
				} else
				if (ctx.getOutImplib().get().empty()) {
					//
					// above out is set to the import lib, from here on this
					// needs the .dll attached. (only if not microsoft frontend
					// since this would append it anyway)
					//
					out.appendDirect(".dll");
				}
			}

			//
			// lookup and embed manifest.
			//
			utils::Path manifest = out.get() + ".mf";
			utils::Log::verbose("looking for manifest: %s\n", manifest.get().c_str());

			if(manifest.waitForAppearance() && ctx.getBackendType() == utils::ToolchainMicrosoft)
			{
				//
				// embed manifest
				//

				if(ctx.getManifestExe().get().empty())
				{
					utils::Log::warning("manifest tool executable not set in configuration, embed manually!\n");
				} else {
					utils::Task embed;
					vec.clear();

					out.toForeign();
					manifest.toForeign();

					MsCompiler::vectorize(ctx.getManifestDefaults(), vec);

					vec.push_back("-manifest");
					vec.push_back(manifest.get());
					vec.push_back("-outputresource:" + out.get());

					if(!task.execute(ctx.getManifestExe(), vec))
					{
						utils::Log::warning("cannot embed manifest, please embed manually!\n");
					} else {
						ctx.getTemporaryFiles().push_back(manifest);
					}
				}
			}

            out.toNative();

            utils::Log::verbose("changing mode of %s.\n", out.get().c_str());
            out.mode(0777);
		}
	}
}

