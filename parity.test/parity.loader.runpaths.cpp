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
		static const char dataMainCFile[] = 
			"extern __declspec(dllimport) int extern_var;"
			"int ma""in(void) {"
			"	if(extern_var == 11) {"
			"		return 0;"
			"	} else {"
			"		return 1;"
			"	}"
			"}";
		static const char dataLibCFile[] =
			"int extern_var = 11;";

		static const char dataConf[] =
			"DebugLevel = verbose\n"
			"Colored = off\n"
			"GenerateLoader = off";

		bool TestSuite::testParityLoaderRunpaths()
		{
			try {
				utils::Task::ArgumentVector exearguments;
				utils::Task::ArgumentVector libarguments;
				utils::Path dir(".parity.testsuite.dir");

				if(dir.exists()) {
					if(dir.remove()) {
						dir.createPath();
					} else {
						utils::Log::warning("cannot remove %s\n", dir.get().c_str());
					}
				} else {
					dir.createPath();
				}
				
				utils::Path maincfile = utils::Path::getTemporary(".parity.testsuite.main.runpaths.XXXXXX.c");
				utils::Path libcfile = utils::Path::getTemporary(".parity.testsuite.library.runpaths.XXXXXX.c");
				utils::Path oexefile = utils::Path::getTemporary(".parity.testsuite.runpaths.XXXXXX.exe");
				utils::Path olibfile = utils::Path::getTemporary(".parity.testsuite.runpaths.XXXXXX.so");

				std::ofstream ofs(maincfile.get().c_str());
				ofs << dataMainCFile;
				ofs.close();
				ofs.open(libcfile.get().c_str());
				ofs << dataLibCFile;
				ofs.close();

				utils::Context::getContext().getTemporaryFiles().push_back(maincfile);
				utils::Context::getContext().getTemporaryFiles().push_back(libcfile);
				utils::Context::getContext().getTemporaryFiles().push_back(oexefile);
				utils::Context::getContext().getTemporaryFiles().push_back(olibfile);
				utils::Context::getContext().getTemporaryFiles().push_back(utils::Path(olibfile.get() + ".dll"));

				libarguments.push_back(libcfile.get());
				libarguments.push_back("-shared");
				libarguments.push_back("-o");
				libarguments.push_back(olibfile.get());

				exearguments.push_back(maincfile.get());
				exearguments.push_back("-o");
				exearguments.push_back(oexefile.get());
				exearguments.push_back(olibfile.get());
				exearguments.push_back("-Wl,-rpath");
				exearguments.push_back(dir.get());
				
				if(!executeParity(libarguments, false, dataConf))
					throw utils::Exception("cannot execute parity for test suite!");

				if(!olibfile.exists())
					throw utils::Exception("missing library file from compile!");

				if(!executeParity(exearguments, false, dataConf))
					throw utils::Exception("cannot execute parity for test suite!");

				if(!oexefile.exists())
					throw utils::Exception("missing executable file from compile!");

				utils::Task exe;
				if(!exe.execute(oexefile, utils::Task::ArgumentVector()))
					throw utils::Exception("cannot execute test result!");

				if(!utils::Path(olibfile.get() + ".dll").move(dir + (olibfile.file() + ".dll")))
					throw utils::Exception("cannot move library to test directory");

				if(!exe.execute(oexefile, utils::Task::ArgumentVector()))
					throw utils::Exception("cannot execute moved test result!");

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

