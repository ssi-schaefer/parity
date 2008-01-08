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

#include <Path.h>

namespace utils = parity::utils;

namespace parity
{
	namespace testing
	{
		bool TestSuite::testParityUtilsPath()
		{
			utils::Path ux("/dev");
			utils::Path win("/dev");

			if(!ux.isNative())
			{
				ux.toNative();
			}

			/* Foreign paths are allways Windows, so this is what we want here */
			win.toForeign();

			if(!win.isForeign())
			{
				utils::Log::warning("Windows path refused conversion to foreign!\n");
				return false;
			}

			if(!ux.exists())
			{
				utils::Log::warning("ux path (%s) doesn't exist, although it is native, is SFU/SUA installed?\n", ux.get().c_str());
				return false;
			}

			if(win.isNative() && !win.exists())
			{
				utils::Log::warning("Windows path (%s) doesn't exist, although it is native, is SFU/SUA installed?\n", win.get().c_str());
				return false;
			}

			ux.append("file");
			win.append("file");

			if(ux.get().find("file") == std::string::npos
				|| win.get().find("file") == std::string::npos)
			{
				utils::Log::warning("Appending path components does not seem to work (ux: %s, Windows: %s)\n", ux.get().c_str(), win.get().c_str());
				return false;
			}

			if(ux.file() != "file" || win.file() != "file")
			{
				utils::Log::warning("Cannot retrieve filename from path (ux: %s, Windows: %s)\n", ux.get().c_str(), win.get().c_str());
				return false;
			}

			ux.set(ux.base());
			win.set(win.base());

			return true;
		}
	}
}

