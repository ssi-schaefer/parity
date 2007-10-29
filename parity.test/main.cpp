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
	bool quiet;
} testcase_t;

using namespace parity;

//
// CONFIX:EXENAME('parity.test')
//

int main(int argc, char** argv)
{
	std::vector<std::string> arguments;
	bool bQuiet = false;
	bool bConfig = false;

	for(int i = 0; i < argc; ++i)
	{
		std::string arg(argv[i]);

		if(arg == "--quiet")
		{
			bQuiet = true;
			argv[i] = "";
		}

		if(bConfig)
		{
			bConfig = false;

			if(arg.empty())
			{
				utils::Log::error("missing argument to -config\n");
				exit(1);
			}

			utils::Path tmp(arg);
			utils::MappedFile file(tmp, utils::ModeRead);
			utils::Config::parseFile(utils::Context::getContext(), file);

			argv[i] = "";
		}

		if(arg == "--config")
		{
			bConfig = true;
			argv[i] = "";
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
		{ "parity::utils::Path", &parity::testing::TestSuite::testParityUtilsPath, true },
		{ "parity::utils::Environment", &parity::testing::TestSuite::testParityUtilsEnvironment, true },
		{ "parity::utils::Exception", &parity::testing::TestSuite::testParityUtilsException, true },
		{ "parity::utils::Config", &parity::testing::TestSuite::testParityUtilsConfig, true },
		{ "parity::utils::MappedFile", &parity::testing::TestSuite::testParityUtilsMappedFile, true},
		{ "parity::utils::MemoryFile", &parity::testing::TestSuite::testParityUtilsMemoryFile, true },
		{ "parity::utils::Task", &parity::testing::TestSuite::testParityUtilsTask, true },
		{ "parity::options::CommandLine", &parity::testing::TestSuite::testParityOptionsCommandLine, true },
		{ "parity::binary::Object", &parity::testing::TestSuite::testParityBinaryObject, true },
		{ "parity::binary::Object (write)", &parity::testing::TestSuite::testParityBinaryObjectWrite, true },
		{ "parity::binary::Image (write)", &parity::testing::TestSuite::testParityBinaryImageWrite, true },
		{ "parity.exe: compile", &parity::testing::TestSuite::testParityExeCompile, true },
		{ "parity.exe: link (and intermediate compile)", &parity::testing::TestSuite::testParityExeLink, true },
		{ "parity.exe: static import", &parity::testing::TestSuite::testParityExeStaticImport, true },
		{ "parity.exe: automatic export", &parity::testing::TestSuite::testParityExeAutoExport, true },
		{ "", NULL }
	};

	testcase_t* current = cases;
	unsigned long numPass = 0;
	unsigned long numFailed = 0;

	utils::Log::profile(col.magenta("\n   Running Tests...\n").c_str());
	utils::Log::profile(col.magenta("   ---------------------------------------------------\n").c_str());

	while(current->func)
	{
		if(!bQuiet)
		{
			if(current->quiet)
				utils::Log::setLevel(utils::Log::Profile);
			else
				utils::Log::setLevel(utils::Log::Verbose);

			utils::Log::verbose(col.magenta("   ---------------------------------------------------\n").c_str());
			utils::Log::verbose(" * Running %s verbosely\n", current->name.c_str());
		}

		utils::Timing::instance().start(current->name);

		if(current->quiet)
			utils::Log::profile(" * %-45s: \r", current->name.c_str());

		if((suite.*current->func)())
		{
			if(!current->quiet)
				utils::Log::verbose(col.magenta("   ---------------------------------------------------\n").c_str());

			utils::Log::profile(" * %-45s:   %s\n", current->name.c_str(), col.green("ok").c_str());
			++numPass;
		} else {
			if(!current->quiet)
				utils::Log::verbose(col.magenta("   ---------------------------------------------------\n").c_str());

			utils::Log::profile(" * %-45s: %s\n", current->name.c_str(), col.red("fail").c_str());
			++numFailed;
		}

		utils::Timing::instance().stop(current->name);

		++current;
	}

	utils::Log::profile(col.magenta("   ---------------------------------------------------\n").c_str());
	if(numFailed == 0)
	{
		std::cerr << "   All " << numPass << " Tests passed." << std::endl;
	} else {
		std::cerr << "   " << numFailed << " out of " << (numPass + numFailed) << " Tests failed." << std::endl;
	}
	utils::Log::profile(col.magenta("   ---------------------------------------------------\n").c_str());

	if(!bQuiet) {
		utils::Log::setLevel(utils::Log::Profile);
	}

	return numFailed;
}

