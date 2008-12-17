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
			if(ctx.getSharedLink()) {
				vec.push_back("/DLL");

				if(ctx.getFrontendType() == utils::ToolchainMicrosoft)
				{
					utils::Path dll = ctx.getOutputFile();

					dll.toNative();
					if(dll.exists())
						dll.remove();

					dll.toForeign();
					vec.push_back("/OUT:" + dll.get());

					out = dll;
				} else {
					utils::Path dll = ctx.getOutputFile().get() + ".dll";
					utils::Path lib = ctx.getOutputFile();

					dll.toNative();
					lib.toNative();

					if(dll.exists())
						dll.remove();
					
					if(lib.exists())
						lib.remove();

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

				out.toForeign();
				vec.push_back("/OUT:" + out.get());

				std::string outFile = out.file();
				task.addFilter(outFile.substr(0, outFile.rfind(".")) + ".lib", false);
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
			// change manifest name
			//
			if(ctx.getFrontendType() != utils::ToolchainMicrosoft)
			{
				utils::Path mf;

				if(ctx.getSharedLink())
					mf = out.get() + ".dll.mf";
				else
					mf = out.get() + ".mf";

				mf.toForeign();

				vec.push_back("/MANIFESTFILE:" + mf.get());
				task.addFilter("/MANIFESTFILE", false);
			}

			//
			// create command scripts
			//
			if(ctx.getUseCommandScripts())
				task.createCommandScript(vec);

			//
			// execute linker.
			//
			if(!task.execute(ctx.getLinkerExe(), vec))
				throw utils::Exception("cannot execute: %s", ctx.getLinkerExe().get().c_str());

			out.toNative();
			utils::Log::verbose("changing mode of %s.\n", out.get().c_str());

			if(out.exists())
				out.mode(0777);
			else
				throw utils::Exception("missing output from linker: %s", out.get().c_str());

			//
			// remove unwanted stuff.
			//
			out.toNative();

			if(ctx.getFrontendType() != utils::ToolchainMicrosoft)
			{
				std::string outFile = out.get();
				utils::Path exp;
				
				exp = outFile.substr(0, outFile.rfind('.')) + ".exp";

				if(exp.exists())
					ctx.getTemporaryFiles().push_back(exp);

				if(!ctx.getSharedLink())
				{
					//
					// executable may produce .lib file in some cases
					//
					utils::Path lib = outFile.substr(0, outFile.rfind('.')) + ".lib";
					if(lib.exists())
						ctx.getTemporaryFiles().push_back(lib);
				} else {
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

			if(manifest.exists() && ctx.getBackendType() == utils::ToolchainMicrosoft)
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
		}
	}
}

