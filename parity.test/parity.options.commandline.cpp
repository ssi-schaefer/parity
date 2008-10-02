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
#include <CommandLine.h>

namespace utils = parity::utils;

namespace parity
{

	namespace testing
	{
		bool CompileOnlyOption(const char * OPT_UNUSED(option), const char* OPT_UNUSED(argument), bool& OPT_UNUSED(used)) {
			return true;
		}

		bool ExceptionOption(const char *  OPT_UNUSED(option), const char* OPT_UNUSED(argument), bool&  OPT_UNUSED(used)) {
			return true;
		}

		bool OutputOption(const char *  OPT_UNUSED(option), const char* OPT_UNUSED(argument), bool& used) {
			used = true;
			return true;
		}

		bool FileOption(const char * option, const char * OPT_UNUSED(argument), bool&  OPT_UNUSED(used)) {
			if(option[strlen(option) - 1] == 'o') {
				return true;
			} else {
				return false;
			}
		}

		bool SourceOption(const char*  OPT_UNUSED(option), const char* OPT_UNUSED(argument), bool&  OPT_UNUSED(used)) {
			return true;
		}

		static options::ArgumentTableEntry options[] = {
			{ "-c", CompileOnlyOption },
			{ "-o", OutputOption },
			{ "", FileOption },
			{ ".cc", SourceOption },
			{ "-fexc", ExceptionOption },
			{ NULL, NULL },
		};

		bool TestSuite::testParityOptionsCommandLine()
		{
			try {
				int argc = 6;
				char const * argv[] = {
					"-o",
					"markus.o",
					"-fexceptions",
					"other.o",
					"-c",
					"something.cc"
				};

				if(parity::options::CommandLine::process(argc, (char **)argv, options, 0))
					return true;
			} catch(const utils::Exception&)
			{
				return false;
			}

			return false;
		}
	}
}

