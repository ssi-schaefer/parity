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

#include "MsCompiler.h"
#include <Exception.h>
#include <Environment.h>
#include <Log.h>

namespace parity
{
	namespace tasks
	{
		MsCompiler::MsCompiler()
		{
			if(utils::Context::getContext().getSources().empty())
				throw utils::Exception("No Source files given as input for Compiler!");
		}

		void MsCompiler::doWork()
		{
			utils::Context& ctx = utils::Context::getContext();
			utils::Task::ArgumentVector arguments;
			prepareGeneric(arguments);

			for(utils::SourceMap::iterator it = ctx.getSources().begin(); it != ctx.getSources().end(); ++it)
			{
				utils::Path native(it->first);
				native.toNative();

				utils::Task::ArgumentVector specialized(arguments);
				prepareGenericFile(native, specialized, it->second);

				switch(it->second)
				{
				case utils::LanguageAsssembler:
					compileGeneric(native, ctx.getAssemblerExe(), specialized);
					break;
				case utils::LanguageC:
				case utils::LanguageCpp:
					processCOrCppFile(specialized);
					compileGeneric(native, ctx.getCompilerExe(), specialized);
					break;
				default:
					throw utils::Exception("unsupported language type for %s", native.get().c_str());
				}
			}
		}

		void MsCompiler::processCOrCppFile(utils::Task::ArgumentVector& vec)
		{
			utils::Context& ctx = utils::Context::getContext();

			//
			// Decide about which Runtime to use
			//
			switch(ctx.getRuntime())
			{
				case utils::RuntimeDynamic:
					vec.push_back("/MD");
					break;
				case utils::RuntimeStatic:
					vec.push_back("/MT");
					break;
				case utils::RuntimeDynamicDebug:
					vec.push_back("/MDd");
					break;
				case utils::RuntimeStaticDebug:
					vec.push_back("/MTd");
					break;
				default:
					throw utils::Exception("invalid runtime type specified!");
			}

			//
			// Decide about exception handling mechanism.
			//
			if(ctx.getExceptionHandling())
				vec.push_back("/EHsc");

			//
			// Decide about runtime type information
			//
			if(ctx.getRuntimeTypes())
				vec.push_back("/GR");

			//
			// Decide about time_t size
			//
			if(ctx.getTimeT32Bit())
				vec.push_back("/D_USE_32BIT_TIME_T");
		}

		void MsCompiler::compileGeneric(const utils::Path& file, utils::Path executable, utils::Task::ArgumentVector& vec)
		{
			utils::Task tsk;

			executable.toNative();

			//
			// Generate an output filename option for this source file
			//
			utils::Context& ctx = utils::Context::getContext();
			utils::Path output;
			std::string base = file.file();

			if(!ctx.getPreprocess())
			{
				if(ctx.getDefaultOutput() != ctx.getOutputFile() && ctx.getCompileOnly())
				{
					output = ctx.getOutputFile();
					output.toForeign();

					vec.push_back("/Fo" + output.get());
				} else if(!ctx.getCompileOnly()) {
					//
					// object is only temporary for linking.
					//
					output = utils::Path::getTemporary(".parity.object.XXXXXX.o");
					output.toForeign();
					vec.push_back("/Fo" + output.get());
					ctx.getTemporaryFiles().push_back(output);
				} else {
					//
					// Output filename in this case allways generated from source filename
					//
					output = base.substr(0, base.rfind('.')).append(".o");
					output.toForeign();
					vec.push_back("/Fo" + output.get());
				}
			}
			
			//
			// Publish object information for the linker
			//
			ctx.getObjectsLibraries().push_back(output);

			if(utils::Context::getContext().getUseCommandScripts())
				tsk.createCommandScript(vec);

			if(prepOut_.is_open())
				tsk.setOutStream(prepOut_);

			tsk.addFilter(file.file(), true);

			tsk.addFilter("C4290", false);		// warning about exception specifiers
			tsk.addFilter("D4029", false);		// warning about optimization not supported in standard compiler.

			if(!tsk.execute(executable, vec))
				throw utils::Exception("cannot execute %s", executable.get().c_str());

			output.toNative();
			output.mode(0777);
		}

		void MsCompiler::prepareGenericFile(utils::Path file, utils::Task::ArgumentVector& vec, utils::LanguageType lang)
		{
			utils::Context& ctx = utils::Context::getContext();
			std::string base = file.file();
			//
			// Preprocessing works allways the same too.
			//
			if(ctx.getPreprocess())
			{
				utils::Path outfile = ctx.getOutputFile();
				std::string out;

				if(ctx.getDefaultOutput() != outfile)
				{
					outfile.toForeign();
					out = outfile.get();
				}

				//
				// if preprocessing multiple files, put them into a
				// file.i or file.ii depending on the input language
				//
				if(ctx.getSources().size() > 1)
				{
					if(lang == utils::LanguageCpp)
						out = base.substr(0, base.rfind('.')).append(".ii");
					else
						out = base.substr(0, base.rfind('.')).append(".i");
				}

				if(!out.empty())
				{
					prepOut_.open(out.c_str());
				}

				//
				// Only difference to assembler: /EP given to the compiler means
				// that the preprocessor output does not have #line's.
				//
				if(lang == utils::LanguageAsssembler)
					vec.push_back("/EP");
				else
					vec.push_back("/E");
			} else {
				//
				// parity allways links at another location, so we allways "compile only"
				//
				vec.push_back("/c");
			}

			//
			// Based on Language decide input file switch
			//
			file.toForeign();

			switch(lang)
			{
			case utils::LanguageAsssembler:
				vec.push_back("/Ta" + file.get());
				break;
			case utils::LanguageC:
				vec.push_back("/Tc" + file.get());
				break;
			case utils::LanguageCpp:
				vec.push_back("/Tp" + file.get());
				break;
			default:
				throw utils::Exception("unknown language type, or language not supported by backend!");
			}

			//
			// Add the default values for compiler or assembler
			//
			switch(lang)
			{
			case utils::LanguageAsssembler:
				{
					utils::Task::ArgumentVector temp;
					vectorize(ctx.getAssemblerDefaults(), temp);

					vec.insert(vec.begin(), temp.begin(), temp.end());
				}
				break;
			case utils::LanguageC:
			case utils::LanguageCpp:
				{
					utils::Task::ArgumentVector temp;
					vectorize(ctx.getCompilerDefaults(), temp);
					vectorize(ctx.getCompilerPassThrough(), vec);

					vec.insert(vec.begin(), temp.begin(), temp.end());
				}
				break;
			default:
				throw utils::Exception("unknown language type, or language not supported by backend!");
			}
		}

		void MsCompiler::prepareGeneric(utils::Task::ArgumentVector& vec)
		{
			utils::Context& ctx = utils::Context::getContext();
			utils::PathVector& incPaths = ctx.getIncludePaths();
			utils::PathVector& sysPaths = ctx.getSysIncludePaths();

			utils::PathVector envPaths;

			switch(ctx.getFrontendType())
			{
			case utils::ToolchainInterixGNU:
				{
					utils::Environment envCPath("CPATH");
					envPaths = envCPath.getPathVector();
				}
				break;
			case utils::ToolchainMicrosoft:
				//
				// need not do anything, since cl.exe knows how to handle
				//
				break;
			default:
				throw utils::Exception("don't know how to handle compiler environment for set frontend!");
			}

			//
			// First include paths given on the command line
			//
			for(utils::PathVector::iterator it = incPaths.begin(); it != incPaths.end(); ++it)
			{
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
			for(utils::PathVector::iterator it = envPaths.begin(); it != envPaths.end(); ++it)
			{
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
			if(ctx.getNoStdIncludes())
			{
				vec.push_back("/X");
			} else {
				for(utils::PathVector::iterator it = sysPaths.begin(); it != sysPaths.end(); ++it)
				{
					utils::Path foreign(*it);
					foreign.toForeign();

					vec.push_back("/I" + foreign.get());
				}
			}

			//
			// Decide about debug switch
			//
			if(ctx.getDebugable())
				vec.push_back("/Z7");

			//
			// Set shared library defines (also for assembler)
			//
			if(ctx.getSharedLink())
			{
				vec.push_back("/DPIC");
				vec.push_back("/D_DLL");
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

			for(utils::DefineMap::const_iterator it = defines.begin(); it != defines.end(); ++it)
			{
				std::string def = "/D" + it->first;

				if(!it->second.empty())
				{
					std::string val = it->second;
					std::string::size_type pos = 0;
					
					while((pos = val.find('"', (pos ? pos + 2 : 0))) != std::string::npos)
					{
						if(pos == 0 || (pos > 0 && val[pos-1] != '\\'))
							val.replace(pos, 1, "\\\"");
					}

					def += "=";

					if(!ctx.getUseCommandScripts()) {
						if(val.find(' ') != std::string::npos)
							def += "\"" + val + "\"";
					} else {
						def += val;
					}

					utils::Log::verbose("define: %s\n", def.c_str());
				}

				vec.push_back(def);
			}
		}

		void MsCompiler::vectorize(const std::string& str, utils::Task::ArgumentVector& vec)
		{
			std::string::size_type start = 0;
			std::string::size_type end = 0;

			while(end < str.size())
			{
				while(str[end] != ' ' && str[end] != '\0') end++;
				vec.push_back(std::string(str, start, end - start));
				start = ++end;
			}
		}

	}
}

