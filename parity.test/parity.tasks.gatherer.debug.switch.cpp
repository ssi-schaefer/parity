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
#include <BinaryGatherer.h>

namespace parity
{
	namespace testing
	{
		//
		// the "" in the function name is to prevent confix from thinking that
		// it should build this as executable.
		//
		static const char dataCFile[] = 
			"int ma""in(void) {"
			"	return 0;"
			"}";

		bool TestSuite::testParityTasksGathererDebugSwitch()
		{
			try {
				utils::Task::ArgumentVector arguments;

				utils::Path cfile = utils::Path::getTemporary(".parity.testsuite.gatherer.debug.switch.XXXXXX.c");
				utils::Path ofile = utils::Path::getTemporary(".parity.testsuite.gatherer.debug.switch.XXXXXX.o");

				std::ofstream ofs(cfile.get().c_str());
				ofs << dataCFile;
				ofs.close();

				utils::Context::getContext().getTemporaryFiles().push_back(cfile);
				utils::Context::getContext().getTemporaryFiles().push_back(ofile);

				arguments.push_back("-g");
				arguments.push_back("-c");
				arguments.push_back(cfile.get());
				arguments.push_back("-o");
				arguments.push_back(ofile.get());

				//
				// we need to append this to prevent a warning about being unable to
				// find an implicit dependency from the directives section.
				//
				if(utils::Context::getContext().getBackendType() == utils::ToolchainMicrosoft) {
					arguments.push_back("-X");
					arguments.push_back("/Zl");
				}
				
				if(!executeParity(arguments, false))
					throw utils::Exception("cannot execute parity for test suite!");

				if(!ofile.exists())
					throw utils::Exception("missing object file from compile!");

				utils::Context::getContext().getObjectsLibraries().push_back(ofile);
				utils::Context::getContext().setDebugable(false);

				tasks::BinaryGatherer gatherer;
				gatherer.doWork();

				utils::Context::getContext().getObjectsLibraries().clear();

				if(!utils::Context::getContext().getDebugable()) {
					throw utils::Exception("context not set to debugable linking.");
				}

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

