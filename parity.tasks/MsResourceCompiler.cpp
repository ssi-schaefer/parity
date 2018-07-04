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

#include "MsResourceCompiler.h"
#include "MsCompiler.h"
#include <Exception.h>
#include <Environment.h>
#include <Log.h>

namespace parity
{
	namespace tasks
	{
		void MsResourceCompiler::doWork()
		{
			utils::Context& ctx = utils::Context::getContext();

			utils::SourceMap::iterator source = ctx.getSources().find(sourceFile_);
			if (source == ctx.getSources().end())
				throw utils::Exception("No Source file given as input for Resource Compiler!");
			if (source->second != utils::LanguageResource)
				throw utils::Exception("Invalid Source file given as input for Resource Compiler!");

			utils::Task::ArgumentVector arguments;
			prepareGeneric(arguments);

			utils::Path native(source->first);
			native.toNative();

			prepareGenericFile(native, arguments);

			if(ctx.getResourceCompilerExe().get().empty()) {
				utils::Log::error("Resource Compiler executable not set, cannot compile resource file!\n");
				exit(1);
			}

			compileGeneric(native, ctx.getResourceCompilerExe(), arguments);
		}

		void MsResourceCompiler::compileGeneric(const utils::Path& file, utils::Path executable, utils::Task::ArgumentVector& vec)
		{
			utils::Context& ctx = utils::Context::getContext();
			utils::Task tsk;
			utils::Path output(outputFile_);

			executable.toNative();

			/* rc.exe does not accept @commandfile
			if(utils::Context::getContext().getUseCommandScripts())
				tsk.createCommandScript(vec);
			*/

#if 0
			tsk.addFilter(file.file(), true);

			tsk.addFilter("C4290", false);		// warning about exception specifiers
			tsk.addFilter("D4029", false);		// warning about optimization not supported in standard compiler.
#endif

			if(!tsk.execute(executable, vec))
				throw utils::Exception("cannot execute %s", executable.get().c_str());

			output.toNative();
			if(!output.waitForAppearance()) {
				throw utils::Exception("cannot await appearance of %s\n", output.get().c_str());
			}
			output.mode(0777);
		}

		void MsResourceCompiler::prepareGenericFile(utils::Path file, utils::Task::ArgumentVector& vec)
		{
			utils::Context& ctx = utils::Context::getContext();
			std::string base = file.file();

			std::string out(outputFile_);

			if (out.empty()) {
				out = base.substr(0, base.rfind('.')).append(".res");
			}

			utils::Path outfile(out);
			outfile.toForeign();

			file.toForeign();

			vec.push_back("/fo" + outfile.get());
			vec.push_back(file.get());
		}

		void MsResourceCompiler::prepareGeneric(utils::Task::ArgumentVector& vec)
		{
			utils::Context& ctx = utils::Context::getContext();
			utils::PathVector& incPaths = ctx.getIncludePaths();
			utils::PathVector& sysPaths = ctx.getSysIncludePaths();

			utils::PathVector envPaths;

			switch(ctx.getFrontendType()) {
			case utils::ToolchainInterixGNU:
				{
					utils::Environment envCPath("CPATH");
					envPaths = envCPath.getPathVector();
				}
				break;
			case utils::ToolchainMicrosoft:
				//
				// need not do anything, since rc.exe knows how to handle
				//
				break;
			default:
				throw utils::Exception("don't know how to handle compiler environment for set frontend!");
			}

			//
			// First include paths given on the command line
			//
			for(utils::PathVector::iterator it = incPaths.begin(); it != incPaths.end(); ++it) {
				it->toForeign();

#if 0
				if(!ctx.isBadCompilerPath(*it))
					vec.push_back("/I" + it->get());
				else
					utils::Log::verbose("skipping bad include path %s\n", it->get().c_str());
#else
				vec.push_back("/I" + it->get());
#endif
			}

			//
			// Now include paths given through environments
			//
			for(utils::PathVector::iterator it = envPaths.begin(); it != envPaths.end(); ++it) {
				it->toForeign();

#if 0
				if(!ctx.isBadCompilerPath(*it))
					vec.push_back("/I" + it->get());
				else
					utils::Log::verbose("skipping bad include path (from environment) %s\n", it->get().c_str());
#else
				vec.push_back("/I" + it->get());
#endif
			}

			//
			// Last are system include paths (those set in parity.conf)
			// They get added only if not ignoring default include paths
			// They are not checked for a bad path.
			//
			// GNU windres does not have the /X option.
			//
			if(ctx.getFrontendType() == utils::ToolchainMicrosoft && ctx.getNoStdIncludes()) {
				vec.push_back("/X");
			} else {
				for(utils::PathVector::iterator it = sysPaths.begin(); it != sysPaths.end(); ++it) {
					utils::Path foreign(*it);
					foreign.toForeign();

					vec.push_back("/I" + foreign.get());
				}
			}

			//
			// Add define to disable deprecation of CRT functions for VS 2005
			//
			vec.push_back("/D_CRT_SECURE_NO_DEPRECATE");
			vec.push_back("/D_CRT_NONSTDC_NO_DEPRECATE");

			//
			// Add all defines given on the command line
			//
			utils::DefineMap& defines = ctx.getDefines();

			for(utils::DefineMap::const_iterator it = defines.begin(); it != defines.end(); ++it) {
				std::string def = "/D" + it->first + "=";

				if(!it->second.empty()) {
					std::string val = it->second;
					std::string::size_type pos = 0;

					while((pos = val.find('"', (pos ? pos + 2 : 0))) != std::string::npos) {
						if(pos == 0 || (pos > 0 && val[pos-1] != '\\'))
							val.replace(pos, 1, "\\\"");
					}

					while((pos = val.find('%', (pos ? pos + 2 : 0))) != std::string::npos) {
						if(pos == 0 || (pos > 0 && val[pos-1] != '\\'))
							val.replace(pos, 1, "%%");
					}

					if(!ctx.getUseCommandScripts()) {
						if(val.find(' ') != std::string::npos)
							def += "\"" + val + "\"";
						else
							def += val;
					} else {
						def += val;
					}
				}

				utils::Log::verbose("define: %s\n", def.c_str());

				vec.push_back(def);
			}

			if (!ctx.getCodepage().empty()) {
				vec.push_back("/c" + ctx.getCodepage());
			}

			if (!ctx.getLanguage().empty()) {
				vec.push_back("/l" + ctx.getLanguage());
			}

			if (!ctx.getCompilerPassThrough().empty()) {
				MsCompiler::vectorize(ctx.getCompilerPassThrough(), vec);
			}
		}

	}
}

