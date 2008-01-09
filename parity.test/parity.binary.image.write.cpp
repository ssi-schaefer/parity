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

#include <MemoryFile.h>
#include <Exception.h>
#include <CoffFile.h>
#include <CoffFileHeader.h>
#include <CoffImage.h>
#include <CoffDirectiveSection.h>

#include <Timing.h>

#include <errno.h>

namespace utils = parity::utils;
using namespace parity::binary;

namespace parity
{
	namespace testing
	{
		//
		// the "" in the function name is to prevent confix from thinking that
		// it should build this as executable.
		//
		static const char dataCFile[] = 
			"int ma""in(void) {"
			"	return 0;"
			"}";

		extern void testFileHeader(binary::FileHeader& hdr);

		bool TestSuite::testParityBinaryImageWrite()
		{
			try {
				utils::Task::ArgumentVector arguments;
				
				utils::Path cfile = utils::Path::getTemporary(".parity.testsuite.link.XXXXXX.c");
				utils::Path ofile = utils::Path::getTemporary(".parity.testsuite.link.XXXXXX.exe");

				std::ofstream ofs(cfile.get().c_str());
				ofs << dataCFile;
				ofs.close();

				utils::Context::getContext().getTemporaryFiles().push_back(cfile);
				utils::Context::getContext().getTemporaryFiles().push_back(ofile);

				arguments.push_back(cfile.get());
				arguments.push_back("-o");
				arguments.push_back(ofile.get());
				
				if(!executeParity(arguments, false))
					throw utils::Exception("cannot execute parity for test suite!");

				if(!ofile.exists())
					throw utils::Exception("missing executable file from compile!");

				utils::Log::verbose("doing parsed copy of %s...\n", ofile.get().c_str());

				utils::MappedFile mapping(ofile, utils::ModeRead);
				//utils::MappedFile mapping(utils::Path("test.exe"), utils::ModeRead);
				binary::Image img(&mapping);

				utils::Log::verbose("testing original file:\n");
				testFileHeader(img.getHeader());

				utils::Path temp = utils::Path::getTemporary(".parity.testsuite.image.XXXXXX.exe");
				utils::Context::getContext().getTemporaryFiles().push_back(temp);
				utils::MemoryFile file;

				img.update(file);
				file.save(temp);

				utils::MappedFile check_mapping(temp, utils::ModeRead);
				binary::Image check_img(&check_mapping);

				utils::Log::verbose("testing parsed copy of executable:\n");
				testFileHeader(check_img.getHeader());

				return true;
			} catch(const utils::Exception& e)
			{
				utils::Log::warning("catched: %s\n", e.what());
			}

			return false;
		}
	}
}

