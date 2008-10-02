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

#ifndef __TESTSUITE_H__
#define __TESTSUITE_H__

#include <list>
#include <vector>
#include <iostream>

#include <Log.h>
#include <Path.h>
#include <Task.h>

namespace parity
{
	namespace testing
	{
		class TestSuite
		{
		public:
			TestSuite(std::vector<std::string> arguments) : arguments_(arguments) {}

			//
			// pure internal tests
			//

			bool testParityUtilsPath();
			bool testParityUtilsEnvironment();
			bool testParityUtilsException();
			bool testParityOptionsCommandLine();
			bool testParityUtilsConfig();
			bool testParityUtilsMappedFile();
			bool testParityUtilsMemoryFile();
			bool testParityUtilsTask();
			bool testParityUtilsThreading();
			bool testParityBinaryObject();
			bool testParityBinaryObjectWrite();
			bool testParityBinaryImageWrite();

			//
			// tests that call parity.exe
			//
			bool testParityExeCompile();
			bool testParityExeLink();
			bool testParityExeStaticImport();
			bool testParityExeAutoExport();
			bool testParityExeShared();
			bool testParityExeNoLoad();
			bool testParityExeStatistics();

			bool testParityTasksGathererDebugSwitch();

			//
			// tests that test only the resulting binaries
			// (i.e. the loader and the runtime).
			//
			bool testParityRuntimeDlfcn();
			bool testParityLoaderPreload();
			bool testParityLoaderRunpaths();

			//
			// tests for other executables from parity.
			//
			bool testParityInspectorExe();

			//
			// internal helper functions
			//
			parity::utils::Path getParityExecutable(std::string const& x = std::string("parity.gnu.gcc"));
			bool executeParity(const utils::Task::ArgumentVector& vec, bool quiet, char const* conf = NULL);
			void dumpStreams(std::string out, std::string err);

		private:
			std::vector<std::string> arguments_;
		};
	}
}

#endif

