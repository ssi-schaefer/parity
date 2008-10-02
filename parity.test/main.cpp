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

#include <Timing.h>
#include <CommandLine.h>
#include <Configuration.h>
#include <MappedFile.h>
#include <GenericFile.h>
#include <TableGnuGcc.h>
#include <Path.h>
#include <Color.h>

#include <iostream>
#include <sstream>

typedef struct {
	std::string name;
	bool (parity::testing::TestSuite::*func)();
} testcase_t;

using namespace parity;

//
// CONFIX:EXENAME('parity.test')
//

int main(int argc, char** argv)
{
	std::vector<std::string> arguments;
	bool bQuiet = true;
	bool bConfig = false;
	std::map<int, bool> to_run;

	for(int i = 0; i < argc; ++i)
	{
		std::string arg(argv[i]);

		if(arg == "--verbose" || arg == "-v")
		{
			bQuiet = false;
			argv[i] = const_cast<char*>("");
		}

		if(arg == "--help" || arg == "-h") {
			std::cout << "usage: " << argv[0] << " [--verbose|-v] [--help|-h]" << std::endl << "       [--config <file>] [<testnum> ...] [<parity args>]" << std::endl;
			std::cout << std::endl << " --verbose | -v      verbose output (may be very much)." << std::endl;
			std::cout <<              " --help | -h         print this help message and exit." << std::endl;
			std::cout <<              " --config <file>     load configuration from file. the values loaded from this" << std::endl;
			std::cout <<              "                     file are used to configure the test environment, not the" << std::endl;
			std::cout <<              "                     parity instances started by the testsuite." << std::endl;
			std::cout <<              "                     This means that some settings may not have any effect, " << std::endl;
			std::cout <<              "                     while others influence the internal unit tests." << std::endl;
			std::cout <<              " <testnum> ...       test numbers can be specified on the command line, to" << std::endl;
			std::cout <<              "                     only run those tests. multiple test numbers can be" << std::endl;
			std::cout <<              "                     specified, each as one argument, seperated by space." << std::endl;

			exit(1);
		}

		if(isdigit(arg[0])) {
			to_run[atoi(arg.c_str())] = true;
			argv[i] = const_cast<char*>("");
		}

		if(bConfig)
		{
			bConfig = false;

			if(arg.empty())
			{
				utils::Log::error("missing argument to --config\n");
				exit(1);
			}

			utils::Path tmp(arg);
			utils::MappedFile file(tmp, utils::ModeRead);
			utils::Config::parseFile(utils::Context::getContext(), file, argc, argv);

			argv[i] = const_cast<char*>("");
		}

		if(arg == "--config")
		{
			bConfig = true;
			argv[i] = const_cast<char*>("");
		}

		arguments.push_back(arg);
	}

	utils::Context::getContext().setTimingShowTasks(!bQuiet);
	utils::Color col(utils::Context::getContext().getColorMode());

	testing::TestSuite suite(arguments);
	utils::Log::setLevel(bQuiet ? utils::Log::Profile : utils::Log::Verbose);

	utils::Log::verbose("preparing context from arguments.\n");
	options::CommandLine::process(argc - 1, &argv[1], options::OptionTableGnuGcc, 0);

	testcase_t cases[] = {
		{ "parity::utils::Path", &parity::testing::TestSuite::testParityUtilsPath },
		{ "parity::utils::Environment", &parity::testing::TestSuite::testParityUtilsEnvironment },
		{ "parity::utils::Exception", &parity::testing::TestSuite::testParityUtilsException },
		{ "parity::utils::Config", &parity::testing::TestSuite::testParityUtilsConfig },
		{ "parity::utils::MappedFile", &parity::testing::TestSuite::testParityUtilsMappedFile },
		{ "parity::utils::MemoryFile", &parity::testing::TestSuite::testParityUtilsMemoryFile },
		{ "parity::utils::Task", &parity::testing::TestSuite::testParityUtilsTask },
		{ "parity::utils::Threading", &parity::testing::TestSuite::testParityUtilsThreading },
		{ "parity::options::CommandLine", &parity::testing::TestSuite::testParityOptionsCommandLine },
		{ "parity::tasks::BinaryGatherer (switch to debug)", &parity::testing::TestSuite::testParityTasksGathererDebugSwitch },
		{ "parity::binary::Object", &parity::testing::TestSuite::testParityBinaryObject },
		{ "parity::binary::Object (write)", &parity::testing::TestSuite::testParityBinaryObjectWrite },
		{ "parity::binary::Image (write)", &parity::testing::TestSuite::testParityBinaryImageWrite },
		{ "parity::runtime::dlfcn", &parity::testing::TestSuite::testParityRuntimeDlfcn },
		{ "parity::loader::preload", &parity::testing::TestSuite::testParityLoaderPreload },
		{ "parity.exe: compile", &parity::testing::TestSuite::testParityExeCompile },
		{ "parity.exe: link (and intermediate compile)", &parity::testing::TestSuite::testParityExeLink },
		{ "parity.exe: shared link", &parity::testing::TestSuite::testParityExeShared },
		{ "parity.exe: shared link (no loader)", &parity::testing::TestSuite::testParityExeNoLoad },
		{ "parity.exe: static import", &parity::testing::TestSuite::testParityExeStaticImport },
		{ "parity.exe: automatic export", &parity::testing::TestSuite::testParityExeAutoExport },
		{ "parity.exe: statistics", &parity::testing::TestSuite::testParityExeStatistics },
		{ "parity.exe: runpaths", &parity::testing::TestSuite::testParityExeStatistics },
		{ "parity.inspector.exe: basic functionality", &parity::testing::TestSuite::testParityInspectorExe },
		{ "", NULL }
	};

	testcase_t* current = cases;
	unsigned long numPass = 0;
	unsigned long numFailed = 0;
	unsigned long numCur = 0;

	utils::Log::profile(col.magenta("\n   Running Tests...\n").c_str());
	utils::Log::profile(col.magenta("   ----------------------------------------------------------------------------\n").c_str());

	while(current->func)
	{
		++numCur;

		if((!to_run.empty() && to_run[numCur]) || to_run.empty())
		{
			if(!bQuiet)
			{
				utils::Log::setLevel(utils::Log::Verbose);

				utils::Log::verbose(col.magenta("   ----------------------------------------------------------------------------\n").c_str());
				utils::Log::verbose(" * Running %s verbosely\n", col.yellow(current->name).c_str());
			}

			utils::Timing::instance().start(current->name);

			if(bQuiet)
				utils::Log::profile(" %s [%2d] %-65s: \r", col.cyan("*").c_str(), numCur, current->name.c_str());

			//
			// flush to let the user know what happens
			//
			std::cerr.flush();
			std::cout.flush();

			if((suite.*current->func)())
			{
				if(!bQuiet)
					utils::Log::verbose(col.magenta("   ----------------------------------------------------------------------------\n").c_str());

				utils::Log::profile(" %s [%2d] %-65s:   %s\n", col.cyan("*").c_str(), numCur, current->name.c_str(), col.green("ok").c_str());
				++numPass;
			} else {
				if(!bQuiet)
					utils::Log::verbose(col.magenta("   ----------------------------------------------------------------------------\n").c_str());

				utils::Log::profile(" %s [%2d] %-65s: %s\n", col.cyan("*").c_str(), numCur, current->name.c_str(), col.red("fail").c_str());
				++numFailed;
			}

			utils::Timing::instance().stop(current->name);
		}

		++current;
	}

	utils::Log::profile(col.magenta("   ----------------------------------------------------------------------------\n").c_str());
	if(numFailed == 0)
	{
		std::cerr << "   All " << numPass << " Tests passed." << std::endl;
	} else {
		std::cerr << "   " << numFailed << " out of " << (numPass + numFailed) << " Tests failed." << std::endl;
	}
	utils::Log::profile(col.magenta("   ----------------------------------------------------------------------------\n").c_str());

	if(!bQuiet) {
		utils::Log::setLevel(utils::Log::Profile);
	}

	//
	// set the output format of the timing class to match what we have
	// here (80 chars);
	//
	utils::Timing::instance().setOutputWidth(80);

	//
	// do some more cleanup for crashed/failed tests.
	// find .parity.* files in the current dir, and delete them.
	//

	return numFailed;
}

