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
#include <Statistics.h>

#ifndef _WIN32
#  define _strnicmp strncasecmp
#endif

namespace parity
{
	namespace options
	{
		bool setForceLink(const char* OPT_UNUSED(option), const char* OPT_UNUSED(argument), bool& OPT_UNUSED(used))
		{
			utils::Context& ctx = utils::Context::getContext();
			ctx.setForceLink(true);
			return true;
		}

		bool setSharedLink(const char* OPT_UNUSED(option), const char* OPT_UNUSED(argument), bool& OPT_UNUSED(used))
		{
			utils::Context& ctx = utils::Context::getContext();
			ctx.setSharedLink(true);
			return true;
		}

		bool addLibraryPath(const char* option, const char* argument, bool& used)
		{
			utils::Context& ctx = utils::Context::getContext();
			utils::PathVector& vect = ctx.getLibraryPaths();
			utils::Path pth;

			if(::strstr(option, "LIBPATH"))
			{
				char const *arg = ::strchr(option, ':');

				if(*(++arg) == '\0')
					throw utils::Exception("argument to %s must follow immediatly after ':'!", option);

				pth = utils::Path(arg);
			} else {
				char const *arg = ::strchr(option, 'L');
				
				if(*(++arg) == '\0') {
					arg = argument;
					used = true;
				}
				pth = utils::Path(arg);
			}

			utils::Statistics::instance().addInformation("path-library", pth.get());
			vect.push_back(pth);
			return true;

		}

		bool setPreferStatic(const char* option, const char* OPT_UNUSED(argument), bool& OPT_UNUSED(used))
		{
			utils::Context& ctx = utils::Context::getContext();
			if(option[1] == 'B')
			{
				if(::strcmp(option+2, "dynamic") == 0) {
					ctx.setPreferStatic(false);
					return true;
				}
				else if(::strcmp(option+2, "static") == 0) {
					ctx.setPreferStatic(true);
					return true;
				}
			} else
			if (::strcmp(option, "-static") == 0) {
				ctx.setPreferStatic(true);
				return true;
			}
			return false;
		}

		bool addObjectsLibraries(const char* option, const char* OPT_UNUSED(argument), bool& OPT_UNUSED(used))
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
			} else {
				//
				// skip the ,
				//
				++arg;
			}

			utils::Statistics::instance().addInformation("path-runpath", utils::Path(arg).get());
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
			ctx.setSharedEntryPoint(arg);

			return true;
		}

		bool setOutImplib(const char* option, const char* argument, bool& used)
		{
			char const * arg = NULL;
			if (strncmp(option, "-Wl,--out-implib,", 17) == 0) {
				arg = option + 17;
			} else
			if (_strnicmp(option, "/IMPLIB:", 8) == 0) {
				arg = option + 8;
			}
			if (!arg) {
				used = true;
				arg = argument;
			}
			if (strncmp(arg, "-Wl,", 4) == 0) {
				arg += 4;
			}
			utils::Context& ctx = utils::Context::getContext();
			ctx.setOutImplibString(arg);
			return true;
		}
	}
}

