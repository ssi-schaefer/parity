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
#include <sstream>

#include <Exception.h>
#include <Environment.h>

namespace parity
{
	namespace testing
	{
		utils::Path TestSuite::getParityExecutable()
		{
#ifdef _WIN32
			char buffer[1024];
			GetModuleFileName(GetModuleHandle(0), buffer, 1024);

			utils::Path pth(buffer);
#else
			//
			// argv[0] needs to be relative or absolute.
			// don't start from PATH.
			//
			utils::Path pth(arguments_[0]);
#endif
			pth = pth.base();

			pth.append("parity.gnu.gcc.exe");

			if(!pth.exists())
			{
				pth = pth.base();
				pth.append("parity.gnu.gcc");

				//
				// TODO: search in PATH.
				//

				if(!pth.exists())
					throw utils::Exception("cannot find parity.exe (for non-win32 don't start from PATH)!");
			}

			return pth;
		}

		bool TestSuite::executeParity(const utils::Task::ArgumentVector& vec, bool quiet, char const* conf)
		{
			utils::Task task;
			utils::Path parity = getParityExecutable();
			utils::Environment env("PARITY_CONFIG");
			utils::Color col(utils::Context::getContext().getColorMode());

			if(conf) {
				utils::Path cf = utils::Path::getTemporary(".parity.testsuite.XXXXXX.conf");
				std::ofstream of;
				of.open(cf.get().c_str());

				of << conf << std::endl;
				of.close();

				env.set(cf.get().c_str());

				utils::Context::getContext().getTemporaryFiles().push_back(cf);
			} else {
				env.clear();
			}

			std::ostringstream ossOut;
			std::ostringstream ossErr;

			task.setOutStream(ossOut);
			task.setErrStream(ossErr);

			bool ret = task.execute(parity, vec);

			if(!quiet)
			{
				std::string out = ossOut.str();
				std::string err = ossErr.str();

				if(out == "\n")
					out = "";
				if(err == "\n")
					err = "";

				std::string::size_type pos = 0;

				if(!out.empty())
				{
					while((pos = out.find("\n", pos ? pos + 5 : 0)) != std::string::npos)
						out.replace(pos, 1, "\n   | ");

					utils::Log::verbose(col.magenta("   ----------------------------------------------------------------------------\n").c_str());
					utils::Log::verbose(" %s %s:\n", col.cyan("*").c_str(), col.yellow("parity task stdout").c_str());
					utils::Log::verbose(col.magenta("   ----------------------------------------------------------------------------\n").c_str());
					utils::Log::verbose("   |\n");
					utils::Log::verbose("   | ");
					utils::Log::verbose(out.c_str());
					utils::Log::verbose("\n");
				}

				if(!err.empty())
				{
					while((pos = err.find("\n", pos ? pos + 5 : 0)) != std::string::npos)
						err.replace(pos, 1, "\n   | ");

					utils::Log::verbose(col.magenta("   ----------------------------------------------------------------------------\n").c_str());
					utils::Log::verbose(" %s %s:\n", col.cyan("*").c_str(), col.yellow("parity task stderr").c_str());
					utils::Log::verbose(col.magenta("   ----------------------------------------------------------------------------\n").c_str());
					utils::Log::verbose("   |\n");
					utils::Log::verbose("   | ");
					utils::Log::verbose(err.c_str());
					utils::Log::verbose("\n");
				}
			}

			env.clear();

			return ret;
		}

	}
}

