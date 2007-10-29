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

#include <Configuration.h>
#include <Exception.h>
#include <Context.h>

namespace utils = parity::utils;

const char* config = "\n"
	"\n"
	"#some comment\n"
	" DebugLevel=	profile\n"
	"#and now bogus value:\n"
	"	DebugLevel=invalid level with spaces		# comment\n";

namespace parity
{
	namespace testing
	{
		bool TestSuite::testParityUtilsConfig()
		{
			try {
				utils::Context& ctx = utils::Context::getContext();
				utils::Config::parseString(ctx, config, strlen(config));
				return true;
			} catch(const utils::Exception& e)
			{
				utils::Log::verbose("caught: %s\n", e.what());

				if(std::string(e.what()).find("Invalid Debug Level") != std::string::npos)
					return true;
			}

			return false;
		}
	}
}

