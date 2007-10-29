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

#include "Linker.h"
#include <Log.h>

namespace parity
{
	namespace options
	{
		bool setForceLink(const char* option, const char* argument, bool& used)
		{
			utils::Context& ctx = utils::Context::getContext();
			ctx.setForceLink(true);
			return true;
		}

		bool setSharedLink(const char* option, const char* argument, bool& used)
		{
			utils::Context& ctx = utils::Context::getContext();
			ctx.setSharedLink(true);
			return true;
		}

		bool addLibraryPath(const char* option, const char* argument, bool& used)
		{
			utils::Context& ctx = utils::Context::getContext();
			
			utils::PathVector& vect = ctx.getLibraryPaths();

			if(::strstr(option, "LIBPATH"))
			{
				const char* arg = ::strchr(option, ':');

				if(*(++arg) == '\0')
					throw utils::Exception("argument to %s must follow immediatly after ':'!", option);

				vect.push_back(utils::Path(arg));
			} else {
				const char* arg = ::strchr(option, 'L');
				
				if(*(++arg) == '\0') {
					arg = argument;
					used = true;
				}

				vect.push_back(utils::Path(arg));
			}
			return true;

		}

		bool setPreferStatic(const char* option, const char* argument, bool& used)
		{
			utils::Context& ctx = utils::Context::getContext();
			if(option[1] == 'B')
			{
				if(::strstr(option, "dynamic"))
					ctx.setPreferStatic(false);
				else if(::strstr(option, "static"))
					ctx.setPreferStatic(true);
			} else
				ctx.setPreferStatic(true);
			return true;
		}

		bool addObjectsLibraries(const char* option, const char* argument, bool& used)
		{
			utils::Context& ctx = utils::Context::getContext();

			if(::strlen(option) == 3)
			{
				if(::strcmp(option, "-lm") == 0 || ::strcmp(option, "-lc") == 0)
					return true;
			}

			ctx.setObjectsLibrariesString(option);
			return true;
		}

		bool setSubsystem(const char* option, const char* argument, bool& used)
		{
			utils::Context& ctx = utils::Context::getContext();
			const char* arg = ::strchr(option, ':');

			if(arg)
				++arg;
			else {
				arg = argument;
				used = true;
			}

			ctx.setSubsystemString(arg);

			return true;
		}

		bool addRunPath(const char* option, const char* argument, bool& used)
		{
			utils::Context& ctx = utils::Context::getContext();
			const char* arg = 0;
			const char* opt = option;

			if(::strstr(option, "-Wl,"))
				opt = &option[4];

			arg = ::strchr(opt, ',');

			if(!arg) {
				arg = argument;

				if(!arg)
					throw utils::Exception("%s requires and argument!", option);

				used = true;
			}

			ctx.setRunPathsString(arg);

			return true;
		}
	
		bool setEntryPoint(const char* option, const char* argument, bool& used)
		{
			const char* arg = argument;
			used = true;

			if(::strstr(option, "-Wl,")) {
				if((arg = ::strchr(&option[4], ',')) == 0)
					arg = argument;
				else
					used = false;
			}

			if(!arg)
				throw utils::Exception("%s requires an argument!", option);

			//
			// argument may contain a -Wl,
			//
			if(::strstr(arg, "-Wl,"))
				arg = &arg[4];

			utils::Context& ctx = utils::Context::getContext();
			ctx.setEntryPoint(arg);

			return true;
		}

	}
}

