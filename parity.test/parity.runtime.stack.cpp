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

#include "TestSuite.h"

#include <Exception.h>
#include <Environment.h>
#include <Context.h>
#include <Log.h>
#include <Task.h>
#include <fstream>
#include <sstream>

namespace parity
{
	namespace testing
	{
		//
		// the "" in the function name is to prevent confix from thinking that
		// it should build this as executable.
		//
		static const char dataCFile[] = 
			"#include <internal/diagnose.h>\n"
			"void func() {\n"
			"	stackframe_t* trace = PcrtGetStackTrace();\n"
			"	if(!trace) return 1;\n"
			"	while(trace) {\n"
			"		printf(\"%p %s\\n\", trace->eip, trace->sym.name);\n"
			"		if(!trace->next) break;\n"
			"		trace = trace->next;\n"
			"	}\n"
			"}\n"
			"int ma""in(int argc, char** argv) {\n"
			"	func();\n"
			"	return 0;\n"
			"}\n";

		bool TestSuite::testParityRuntimeStack()
		{
			try {
				utils::Task::ArgumentVector arguments;

				utils::Path cfile = utils::Path::getTemporary(".parity.testsuite.runtime.stack.XXXXXX.c");
				utils::Path ofile = utils::Path::getTemporary(".parity.testsuite.runtime.stack.XXXXXX.exe");

				std::ofstream ofs(cfile.get().c_str());
				ofs << dataCFile;
				ofs.close();

				utils::Context::getContext().getTemporaryFiles().push_back(cfile);
				utils::Context::getContext().getTemporaryFiles().push_back(ofile);

				arguments.push_back(cfile.get());
				arguments.push_back("-o");
				arguments.push_back(ofile.get());

				if(!executeParity(arguments, false))
					throw utils::Exception("cannot execute parity for test suite!");

				if(!ofile.exists())
					throw utils::Exception("missing executable file from compile!");

				utils::Task exe;

				std::ostringstream os;
				std::ostringstream es;

				exe.setOutStream(os);
				exe.setErrStream(es);

				if(!exe.execute(ofile, utils::Task::ArgumentVector()))
					throw utils::Exception("cannot execute test result!");

				utils::Environment dbg("PCRT_DEBUG_SYMBOLS");
				dbg.set("1");

				if(!exe.execute(ofile, utils::Task::ArgumentVector()))
					throw utils::Exception("cannot execute test result!");

				dbg.clear();

				dumpStreams(os.str(), es.str());

				return true;
			} catch(const utils::Exception& e)
			{
				utils::Log::error("caught exception: %s\n", e.what());
				return false;
			}

			return false;
		}
	}
}
