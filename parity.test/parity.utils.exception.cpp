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

namespace utils = parity::utils;

namespace parity
{
	namespace testing
	{
		bool TestSuite::testParityUtilsException()
		{
			try {
				utils::Log::verbose("throwing Exception(\"test text %s\", \"test insertion\") ... \n");
				throw utils::Exception("test text %s", "test insertion");
			} catch(const utils::Exception& e)
			{
				utils::Log::verbose("catched: %s\n", e.what());
				if(std::string(e.what()) == "test text test insertion")
					return true;
			}

			return false;
		}
	}
}

