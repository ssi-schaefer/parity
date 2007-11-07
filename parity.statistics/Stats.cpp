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

#include "Stats.h"

#include <Log.h>
#include <Statistics.h>

#include <fstream>
#include <time.h>

//
// CONFIX:EXENAME('parity.statistics')
//
int main(int argc, char** argv)
{
	for(int i = 1; i < argc; ++i)
	{
		if(std::string(argv[i]) == "--verbose")
		{
			parity::utils::Log::setLevel(parity::utils::Log::Verbose);
			continue;
		}

		parity::utils::Path pth(argv[i]);
		pth.toNative();
		if(!pth.exists())
		{
			parity::utils::Log::error("cannot find statistics file: %s\n", pth.get().c_str());
			exit(1);
		}

		parity::statistics::ProcessFile(pth);
	}
}

namespace parity
{
	namespace statistics
	{
		void ProcessFile(const utils::Path& path)
		{
			
		}
	}
}

