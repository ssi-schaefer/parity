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
#include <MappedFile.h>
#include <MemoryFile.h>

namespace utils = parity::utils;

#define TESTFILE "memory_file_test.tmp"
#define TESTSTR "hallo file"

namespace parity
{
	namespace testing
	{
		bool TestSuite::testParityUtilsMemoryFile()
		{
			try {
				utils::Path pth(TESTFILE);

				FILE* fp = fopen(pth.get().c_str(), "w");
				fputs(TESTSTR, fp);
				fclose(fp);

				utils::MappedFile file(pth, utils::ModeRead);
				utils::MemoryFile mem(file);

				file.close();

				if(!pth.remove())
					utils::Log::warning("cannot remove test file: %s\n", TESTFILE);

				if(strncmp(TESTSTR, reinterpret_cast<const char*>(mem.getBase()), strlen(TESTSTR)) != 0)
					throw utils::Exception("file content does not match!");

				mem.close();

				std::string buf(TESTSTR);

				utils::MemoryFile mem1(const_cast<char*>(buf.c_str()), buf.length());

				if(strncmp(TESTSTR, reinterpret_cast<const char*>(mem1.getBase()), strlen(TESTSTR)) != 0)
					throw utils::Exception("file content does not match!");

				mem1.close();

				utils::MemoryFile mem2(buf.length());
				strcpy(reinterpret_cast<char*>(mem2.getBase()), buf.c_str());

				if(strncmp(TESTSTR, reinterpret_cast<const char*>(mem2.getBase()), strlen(TESTSTR)) != 0)
					throw utils::Exception("file content does not match!");

				mem2.close();

				return true;
			} catch(const utils::Exception& e)
			{
				utils::Log::warning("catched: %s\n", e.what());
			}

			return false;
		}
	}
}

