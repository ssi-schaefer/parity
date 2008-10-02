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
			"int ma""in(void) {"
			"	return 0;"
			"}";

		static const char dataConf[] =
			"StatisticsFile = .parity.test.statistic.out\n"
			"Colored = off\n";

		bool TestSuite::testParityExeStatistics()
		{
			try {
				utils::Task::ArgumentVector exearguments;
				utils::Path stat(".parity.test.statistic.out");

				if(stat.exists())
					stat.remove();

				utils::Path maincfile = utils::Path::getTemporary(".parity.testsuite.main.no.load.XXXXXX.c");
				utils::Path oexefile = utils::Path::getTemporary(".parity.testsuite.no.load.XXXXXX.exe");

				std::ofstream ofs(maincfile.get().c_str());
				ofs << dataMainCFile;
				ofs.close();

				utils::Context::getContext().getTemporaryFiles().push_back(maincfile);
				utils::Context::getContext().getTemporaryFiles().push_back(oexefile);

				exearguments.push_back(maincfile.get());
				exearguments.push_back("-o");
				exearguments.push_back(oexefile.get());

				if(!executeParity(exearguments, false, dataConf))
					throw utils::Exception("cannot execute parity for test suite!");

				if(!oexefile.exists())
					throw utils::Exception("missing executable file from compile!");

				utils::Task exe;
				if(!exe.execute(oexefile, utils::Task::ArgumentVector()))
					throw utils::Exception("cannot execute test result!");

				if(!stat.exists())
					throw utils::Exception("statistical information missing!");

				stat.remove();

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

