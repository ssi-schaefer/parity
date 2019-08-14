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

#include "MsResourceConverter.h"
#include "MsCompiler.h"
#include <Exception.h>
#include <Environment.h>
#include <Log.h>

namespace parity
{
	namespace tasks
	{
		void MsResourceConverter::doWork()
		{
			utils::Context& ctx = utils::Context::getContext();

			utils::SourceMap::iterator source = ctx.getSources().find(sourceFile_);
			if (source == ctx.getSources().end())
				throw utils::Exception("No Source file given as input for Resource Converter!");
			if (source->second != utils::LanguageCompiledResource)
				throw utils::Exception("Invalid Source file given as input for Resource Converter!");

			utils::Task::ArgumentVector arguments;
			prepareGeneric(arguments);

			utils::Path native(source->first);
			native.toNative();

			prepareGenericFile(native, arguments);

			if(ctx.getResourceConverterExe().get().empty()) {
				utils::Log::error("Resource Converter executable not set, cannot convert resource file!\n");
				exit(1);
			}

			compileGeneric(native, ctx.getResourceConverterExe(), arguments);
		}

		void MsResourceConverter::compileGeneric(const utils::Path& file, utils::Path executable, utils::Task::ArgumentVector& vec)
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

		void MsResourceConverter::prepareGenericFile(utils::Path file, utils::Task::ArgumentVector& vec)
		{
			utils::Context& ctx = utils::Context::getContext();
			std::string base = file.file();

			std::string out(outputFile_);

			if (out.empty()) {
				out = base.substr(0, base.rfind('.')).append(".o");
			}

			utils::Path outfile(out);
			outfile.toForeign();

			file.toForeign();

			vec.push_back("/OUT:" + outfile.get());
			vec.push_back(file.get());
		}

		void MsResourceConverter::prepareGeneric(utils::Task::ArgumentVector& vec)
		{
			utils::Context& ctx = utils::Context::getContext();

			vec.push_back("/NOLOGO");

			if (ctx.getBfdTarget().find("-x86-64") != std::string::npos) {
				vec.push_back("/MACHINE:X64");
			} else
			if (ctx.getBfdTarget().find("-i386") != std::string::npos) {
				vec.push_back("/MACHINE:X86");
			} else {
				switch (ctx.getMachine()) {
				case utils::MachineAMD64:
					vec.push_back("/MACHINE:X64");
					break;
				case utils::MachineI386:
					vec.push_back("/MACHINE:X86");
					break;
				}
			}
		}

	}
}

