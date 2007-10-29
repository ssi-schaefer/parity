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
#include <Context.h>
#include <Log.h>
#include <Task.h>
#include <CoffArchive.h>
#include <CoffFile.h>
#include <MappedFile.h>

#include <fstream>

namespace utils = parity::utils;
namespace binary = parity::binary;

namespace parity
{
	namespace testing
	{
		static const char dataCFile[] = 
			"int function() {"
			"	return 1;"
			"}";

		bool TestSuite::testParityExeAutoExport()
		{
			try {
				utils::Task::ArgumentVector arguments;
				
				utils::Path cfile = utils::Path::getTemporary(".parity.testsuite.export.XXXXXX.c");
				utils::Path ofile = utils::Path::getTemporary(".parity.testsuite.export.XXXXXX.so");

				std::ofstream ofs(cfile.get().c_str());
				ofs << dataCFile;
				ofs.close();

				utils::Context::getContext().getTemporaryFiles().push_back(cfile);
				utils::Context::getContext().getTemporaryFiles().push_back(ofile);
				utils::Path dll = ofile.get() + ".dll";
				utils::Context::getContext().getTemporaryFiles().push_back(dll);

				arguments.push_back(cfile.get());
				arguments.push_back("-shared");
				arguments.push_back("-o");
				arguments.push_back(ofile.get());
				
				if(!executeParity(arguments, false))
					throw utils::Exception("cannot execute parity for test suite!");

				if(!ofile.exists())
					throw utils::Exception("missing shared library file from compile!");

				utils::MappedFile mapping(ofile, utils::ModeRead);

				if(binary::File::getType(&mapping) != binary::File::TypeLibrary)
					throw utils::Exception("file is not a library!");

				binary::Archive lib(&mapping);
				binary::Archive::ImportMap imports = lib.getImports();
				bool foundSym = false;

				for(binary::Archive::ImportMap::iterator it = imports.begin(); it != imports.end(); ++it)
				{
					if(it->second.getSymbolName() == "_function")
						foundSym = true;
				}

				mapping.close();

				if(!foundSym)
					throw utils::Exception("cannot find expected symbol in import library!");

				return true;
			} catch(const utils::Exception& e)
			{
				utils::Log::error("caught exception: %s\n", e.what());
				return false;
			}

			return false;
		}
	}
}

