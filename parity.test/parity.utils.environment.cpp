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

#include <Environment.h>

namespace utils = parity::utils;

namespace parity
{
	namespace testing
	{
		bool TestSuite::testParityUtilsEnvironment()
		{
			utils::Environment single_path = utils::Environment::fromString("/dev");
			utils::Environment multi_path  = utils::Environment::fromString("/dev:/usr:/something");
			utils::Environment wsingle_path= utils::Environment::fromString("C:\\");
			utils::Environment wmulti_path = utils::Environment::fromString("C:\\;C:\\Windows");

			utils::Path u_single = single_path.getPath();
			utils::Path w_single = wsingle_path.getPath();
			utils::PathVector u_multi = multi_path.getPathVector();
			utils::PathVector w_multi = multi_path.getPathVector();

			u_single.toNative();
			w_single.toNative();

			u_single.toForeign();
			w_single.toForeign();

			for(utils::PathVector::iterator it = u_multi.begin(); it != u_multi.end(); ++it)
			{
				utils::Path pth = *it;
				pth.toNative();
				pth.toForeign();
			}

			for(utils::PathVector::iterator it = w_multi.begin(); it != w_multi.end(); ++it)
			{
				utils::Path pth = *it;
				pth.toNative();
				pth.toForeign();
			}

			u_multi = single_path.getPathVector();
			w_multi = wsingle_path.getPathVector();

			for(utils::PathVector::iterator it = u_multi.begin(); it != u_multi.end(); ++it)
			{
				utils::Path pth = *it;
				pth.toNative();
				pth.toForeign();
			}

			for(utils::PathVector::iterator it = w_multi.begin(); it != w_multi.end(); ++it)
			{
				utils::Path pth = *it;
				pth.toNative();
				pth.toForeign();
			}

			utils::Environment to_set("PARITY_TEST_ENV");
			to_set.set("test_value");

			utils::Environment to_get("PARITY_TEST_ENV");
			if(to_get.getValue() != "test_value") {
				utils::Log::error("environment variable cannot be set and re-read again\n");
				return false;
			}

			return true;
		}
	}
}

