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

namespace parity
{
	namespace testing
	{
		//
		// the "" in the function name is to prevent confix from thinking that
		// it should build this as executable.
		//
		static const char dataCFile[] = 
			"extern int func();\n"
			"int ma""in(int argc, char** argv) {\n"
			"	if(func() != 2) { return 1; }\n"
			"	return 0;\n"
			"}\n";

		static const char dataLibFile[] =
			"int func() { return 1; }";

		static const char dataPreloadFile[] =
			"int func() { return 2; }";

		bool TestSuite::testParityLoaderPreload()
		{
			try {
				utils::Task::ArgumentVector arguments;

				utils::Path cfile = utils::Path::getTemporary(".parity.testsuite.loader.preload.XXXXXX.c");
				utils::Path libcfile = utils::Path::getTemporary(".parity.testsuite.loader.preload.l.XXXXXX.c");
				utils::Path precfile = utils::Path::getTemporary(".parity.testsuite.loader.preload.p.XXXXXX.c");
				utils::Path ofile = utils::Path::getTemporary(".parity.testsuite.loader.preload.XXXXXX.exe");
				utils::Path libofile = utils::Path::getTemporary(".parity.testsuite.loader.preload.l.XXXXXX.so");
				utils::Path preofile = utils::Path::getTemporary(".parity.testsuite.loader.preload.p.XXXXXX.so");

				std::ofstream ofs(cfile.get().c_str());
				ofs << dataCFile;
				ofs.close();

				ofs.open(libcfile.get().c_str());
				ofs << dataLibFile;
				ofs.close();

				ofs.open(precfile.get().c_str());
				ofs << dataPreloadFile;
				ofs.close();

				utils::Context::getContext().getTemporaryFiles().push_back(cfile);
				utils::Context::getContext().getTemporaryFiles().push_back(libcfile);
				utils::Context::getContext().getTemporaryFiles().push_back(precfile);
				utils::Context::getContext().getTemporaryFiles().push_back(ofile);
				utils::Context::getContext().getTemporaryFiles().push_back(libofile);
				utils::Context::getContext().getTemporaryFiles().push_back(preofile);
				utils::Context::getContext().getTemporaryFiles().push_back(libofile.get() + ".dll");
				utils::Context::getContext().getTemporaryFiles().push_back(preofile.get() + ".dll");

				arguments.push_back("-shared");
				arguments.push_back(libcfile.get());
				arguments.push_back("-o");
				arguments.push_back(libofile.get());

				if(!executeParity(arguments, false))
					throw utils::Exception("cannot execute parity for test suite!");

				arguments.clear();
				arguments.push_back("-shared");
				arguments.push_back(precfile.get());
				arguments.push_back("-o");
				arguments.push_back(preofile.get());

				if(!executeParity(arguments, false))
					throw utils::Exception("cannot execute parity for test suite!");

				arguments.clear();
				arguments.push_back(cfile.get());
				arguments.push_back("-o");
				arguments.push_back(ofile.get());
				arguments.push_back(libofile.get());

				if(!executeParity(arguments, false))
					throw utils::Exception("cannot execute parity for test suite!");

				if(!ofile.exists() || !libofile.exists())
					throw utils::Exception("missing executable file from compile!");

				utils::Task exe;
				if(exe.execute(ofile, utils::Task::ArgumentVector()))
					throw utils::Exception("unexpected pass of test executable!");

				utils::Environment preload("LD_PRELOAD");
				std::string value = preload.getValue();

				preload.set(value + " " + preofile.get() + ".dll");
				
				utils::Task exe2;
				if(!exe2.execute(ofile, utils::Task::ArgumentVector()))
					throw utils::Exception("cannot execute preloaded binary");
				
				if(value == "")
					preload.clear();
				else
					preload.set(value);

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

