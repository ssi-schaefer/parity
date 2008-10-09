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

#include "Internal.h"
#include "Version.h"
#include <Configuration.h>

#include <iostream>

namespace parity
{
	namespace options
	{
		bool setDebugLevel(const char* option, const char* argument, bool& used)
		{
			if(!argument)
				throw utils::Exception("%s requires an argument!", option);

			used = true;
			
			utils::Context& ctx = utils::Context::getContext();
			ctx.setDebugLevel(argument);
			return true;
		}

		bool setLinkerPassthrough(const char* option, const char* argument, bool& used)
		{
			if(!argument)
				throw utils::Exception("%s requires an argument!", option);

			used = true;

			utils::Context& ctx = utils::Context::getContext();

			std::string val = ctx.getLinkerPassThrough();
			val.append(" ");
			val.append(argument);

			ctx.setLinkerPassThrough(val);
			return true;
		}

		bool setCompilerPassthrough(const char* option, const char* argument, bool& used)
		{
			if(!argument)
				throw utils::Exception("%s requires an argument!", option);

			used = true;

			utils::Context& ctx = utils::Context::getContext();

			std::string val = ctx.getCompilerPassThrough();
			val.append(" ");
			val.append(argument);

			ctx.setCompilerPassThrough(val);
			return true;
		}

		bool setUnhandledSourceFilePassthrough(const char* option, const char* OPT_UNUSED(argument), bool& used)
		{
			return setCompilerPassthrough("-X", option, used);
		}

		bool setUnhandledObjectFilePassthrough(const char* option, const char* OPT_UNUSED(argument), bool& used)
		{
			return setLinkerPassthrough("-Y", option, used);
		}

		bool showParityVersion(const char* OPT_UNUSED(option), const char* OPT_UNUSED(argument), bool& OPT_UNUSED(used))
		{
			std::cout << PACKAGE_NAME << " " << PACKAGE_VERSION << " " << "(" << __DATE__ << ") ";
#ifdef _WIN32
			std::cout << "native windows build" << std::endl;
#elif defined(__INTERIX)
			std::cout << "interix g++ build" << std::endl;
#elif defined(__CYGWIN__)
			std::cout << "cygwin g++ build" << std::endl;
#endif
			std::cout << "Copyright (c) 2007, 2008 Markus Duft <markus.duft@salomon.at>" << std::endl;

			std::cout << PACKAGE_NAME << " comes with ABSOLUTELY NO WARRANTY; This is free software, and you are" << std::endl;
			std::cout << "welcome to redistribute it under certain conditions; see COPYING.LESSER for details." << std::endl;

			/* output some configuration values... */
			#ifdef HAVE_CONFIG_H
				std::cout << std::endl;
				std::cout << "Sysconf Directory: " << PARITY_SYSCONFDIR << std::endl;
				std::cout << "Include Directory: " << PARITY_INCLUDEDIR << std::endl;
				std::cout << "Library Directory: " << PARITY_LIBDIR << std::endl;
			#endif

			exit(0);

			#ifndef _WIN32
			/* never reached (this is there for some gcc versions)! */
			return false;
			#endif
		}

		bool setCtxDump(const char* OPT_UNUSED(option), const char* OPT_UNUSED(argument), bool& OPT_UNUSED(used))
		{
			utils::Context& ctx = utils::Context::getContext();
			ctx.setCtxDump(true);
			return true;
		}

		bool setColorMode(const char* option, const char* argument, bool& used)
		{
			utils::Context& ctx = utils::Context::getContext();
			
			if(!argument || ::strlen(argument) < 1)
				throw utils::Exception("%s requires an argument!", option);

			if(::strcmp(argument, "bright"))
				ctx.setColorMode(utils::Color::Bright);
			else if(::strcmp(argument, "dark"))
				ctx.setColorMode(utils::Color::Dark);
			else
				throw utils::Exception("%s is not a valid argument to %s", argument, option);

			used = true;

			return true;
		}

		bool addConfigString(const char* option, const char* argument, bool& used)
		{
			if(!argument || ::strlen(argument) < 1)
				throw utils::Exception("%s requires an argument!", option);

			used = true;

			if(!utils::Config::parseString(utils::Context::getContext(), argument, strlen(argument)))
				throw utils::Exception("cannot parse configuration string \"%s\"\n", argument);

			return true;
		}

	}
}

