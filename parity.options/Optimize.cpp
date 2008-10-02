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

#include "Optimize.h"

namespace parity
{
	namespace options
	{
		bool setOptimizeLevel(const char* option, const char* argument, bool& used)
		{
			const char* opt = 0;

			if(strlen(option) > 2) // 2 => -O
			{
				opt = option + 2;
			} else {
				opt = argument;

				if(opt && isdigit(opt[0])) {
					used = true;
				} else {
					// Default optimization level is two.
					opt = "2";
				}
			}

			utils::Context& ctx = utils::Context::getContext();
			ctx.setOptimizeLevelString(opt);
			return true;
		}

		bool setOmitFramePointer(const char* OPT_UNUSED(option), const char* OPT_UNUSED(argument), bool& OPT_UNUSED(used))
		{
			utils::Context& ctx = utils::Context::getContext();
			ctx.setOmitFramePointer(true);
			return true;
		}

		bool setInlineFunctions(const char* option, const char* OPT_UNUSED(argument), bool& OPT_UNUSED(used))
		{
			utils::Context& ctx = utils::Context::getContext();
			ctx.setInlineFunctions((::strstr(option, "no") != 0 && ::strstr(option, "Ob0") == 0));
			return true;
		}
	}
}

