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

#include <MappedFile.h>
#include <Exception.h>
#include <stdio.h>

namespace utils = parity::utils;

#define TESTFILE "mapped_file_test.tmp"
#define TESTSTR "hallo file"

namespace parity
{
	namespace testing
	{
		bool TestSuite::testParityUtilsMappedFile()
		{
			try {
				utils::Path pth(TESTFILE);

				FILE* fp = fopen(pth.get().c_str(), "w");
				fputs(TESTSTR, fp);
				fclose(fp);

				utils::MappedFile file(pth, utils::ModeRead);

				if(strncmp(TESTSTR, (const char*)file.getBase(), strlen(TESTSTR)) != 0)
					throw utils::Exception("file content does not match!");

				file.close();

				if(!pth.remove())
					utils::Log::warning("cannot remove test file: %s\n", TESTFILE);

				return true;
			} catch(const utils::Exception& e)
			{
				utils::Log::warning("catched: %s\n", e.what());
			}

			return false;
		}
	}
}

