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

#include "Misc.h"

#include <Statistics.h>

#include <ctype.h>

namespace parity
{
	namespace options
	{
		bool setUseSSE(const char* option, const char* argument, bool& used)
		{
			utils::Context& ctx = utils::Context::getContext();
			ctx.setUseSSE(true);
			return true;
		}

		bool setUseSSE2(const char* option, const char* argument, bool& used)
		{
			utils::Context& ctx = utils::Context::getContext();
			ctx.setUseSSE2(true);
			return true;
		}

		bool setForScope(const char* option, const char* argument, bool& used)
		{
			utils::Context& ctx = utils::Context::getContext();
			ctx.setForScope(!(::strstr(option, "no") && (::strstr(option, "forScope") && option[strlen(option) - 1] == '-')));
			return true;
		}

		bool setMsExtensions(const char* option, const char* argument, bool& used)
		{
			utils::Context& ctx = utils::Context::getContext();
			ctx.setMsExtensions(!(::strstr(option, "ansi") || ::strstr(option, "no") || ::strstr(option, "Za")));
			return true;
		}

		bool setWarningLevel(const char* option, const char* argument, bool& used)
		{
			utils::Context& ctx = utils::Context::getContext();

			if(::strchr(option, 'w')) {
				ctx.setWarningLevel(0);
				return true;
			}

			const char * posW = ::strchr(option, 'W');

			if(posW)
			{
				++posW;

				if(strlen(posW) < 1)
				{
					posW = argument;
					used = true;
				}

				if(strlen(posW) < 1)
					throw utils::Exception("missing argument to %s", option);

				if(::isdigit(*posW))
				{
					ctx.setWarningLevelString(posW);
				} else if(::strncmp(posW, "all", 3) == 0) {
					ctx.setWarningLevel(3);
				} else {
					// dont throw, but continue searching for another option
					// accepting this (think of -Werror)
					return false;
				}
			} else {
				throw utils::Exception("failed to parse %s as warning-level setting option!", option);
			}

			return true;
		}

		bool setWarningAsError(const char* option, const char* argument, bool& used)
		{
			utils::Context& ctx = utils::Context::getContext();
			ctx.setWarningAsError(true);
			return true;
		}

		bool setAnsiMode(const char* option, const char* argument, bool& used)
		{
			utils::Context& ctx = utils::Context::getContext();
			ctx.setAnsiMode(true);
			return true;
		}

		bool setOutputFile(const char* option, const char* argument, bool& used)
		{
			utils::Context& ctx = utils::Context::getContext();
			const char* arg = 0;

			if((arg = ::strchr(option, 'F')) || (arg = ::strstr(option, "OUT:")) || (arg = ::strstr(option, "out:")))
			{
				//
				// Handle Microsoft output options
				//
				if(arg[1] == 'e' || arg[1] == 'o')
				{
					arg += 2;
				} else if(strlen(arg) > 3 && arg[3] == ':') {
					arg += 4;
				} else {
					// not handled here
					return false;
				}
			} else {
				arg = ::strchr(option, 'o');
				++arg;
			}
			
			if(!arg || !*arg){
				used = true;
				arg = argument;
			}

			utils::Path pth(arg);
	
			utils::Statistics::instance().addInformation("file-output", pth.get());

			ctx.setOutputFile(pth);
			return true;
		}

		bool setDebugable(const char* option, const char* argument, bool& used)
		{
			utils::Context& ctx = utils::Context::getContext();
			ctx.setDebugable(true);
			return true;
		}

	}
}

