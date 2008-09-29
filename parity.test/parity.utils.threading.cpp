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
#include <Task.h>
#include <Log.h>
#include <Threading.h>

#include <sstream>

static unsigned int THREADINGAPI runThread1(void* p) {
	parity::utils::Log::verbose("in runThread1: %p\n", p);
	return 0;
}

static unsigned int THREADINGAPI runThread2(void* p) {
	parity::utils::Log::verbose("in runThread2: %p\n", p);
	return 1;
}

namespace parity
{
	namespace testing
	{
		bool TestSuite::testParityUtilsThreading()
		{
			try {
				utils::Threading thr;
					
				if(thr.run(runThread1, NULL, false) < 1)
					throw utils::Exception("cannot start thread 1");

				if(thr.run(runThread2, NULL, true) < 1)
					throw utils::Exception("cannot start thread 2");

				try {
					thr.synchronize();

					utils::Log::verbose("unexpected pass of synchronize!\n");
					return false;
				} catch (utils::Exception& e) {
					// expected :)
					utils::Log::verbose("exception from synchronize (expected): %s\n", e.what());
					if(std::string(e.what()).find("exited abnormally") != std::string::npos)
						return true;
				}
			} catch(const utils::Exception& e)
			{
				utils::Log::warning("catched: %s\n", e.what());
			}

			return false;
		}
	}
}

