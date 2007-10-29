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

#include "Compiler.h"
#include <Log.h>

namespace parity
{
	namespace options
	{
		bool setSyntaxOnly(const char* option, const char* argument, bool& used)
		{
			utils::Context& ctx = utils::Context::getContext();
			ctx.setSyntaxOnly(true);
			return true;
		}

		bool setCompileOnly(const char* option, const char* argument, bool& used)
		{
			utils::Context& ctx = utils::Context::getContext();
			ctx.setCompileOnly(true);
			return true;
		}

		bool setExceptionHandling(const char* option, const char* argument, bool& used)
		{
			utils::Context& ctx = utils::Context::getContext();
			ctx.setExceptionHandling(!::strstr(option, "no"));

			return true;
		}

		bool setRuntimeTypes(const char* option, const char* argument, bool& used)
		{
			utils::Context& ctx = utils::Context::getContext();
			ctx.setExceptionHandling((!::strstr(option, "no") && !(option[::strlen(option) - 1] == '-')));
			return true;
		}

		bool setPositionIndep(const char* option, const char* argument, bool& used)
		{
			utils::Context& ctx = utils::Context::getContext();
			ctx.setPositionIndep(true);
			return true;
		}

		bool setForcedLanguage(const char* option, const char* argument, bool& used)
		{
			utils::Context& ctx = utils::Context::getContext();
			const char* posx = ::strchr(option, 'x');

			if(posx) {
				if(*(++posx) == '\0')
				{
					posx = argument;
					used = true;
				}
			} else {
				if(::strstr(option, "TP"))
					posx = "c++";
				else if(::strstr(option, "TC"))
					posx = "c";
				else
					throw utils::Exception("wrong language specifier in option %s", option);
			}

			ctx.setForcedLanguageString(posx);
			return true;
		}

		bool addSource(const char* option, const char* argument, bool& used)
		{
			utils::Context& ctx = utils::Context::getContext();
			const char * arg = option;

			if((option[0] == '-' || option[0] == '/') && option[1] == 'T')
			{
				arg = &option[3];
				if(!*arg) {
					arg = argument;
					used = true;
				}

				if(option[2] == 'p')
					ctx.getSources()[utils::Path(arg)] = utils::LanguageCpp;
				else if(option[2] == 'c')
					ctx.getSources()[utils::Path(arg)] = utils::LanguageC;
				else
					throw utils::Exception("wrong language specifier in %s!", option);
			} else {
				ctx.setSourcesString(arg);
			}
			return true;
		}

		bool setShortWchar(const char* option, const char* argument, bool& used)
		{
			utils::Context& ctx = utils::Context::getContext();
			ctx.setShortWchar(!::strstr(option, "no"));
			return true;
		}

		bool setRuntime(const char* option, const char* argument, bool& used)
		{
			utils::Context& ctx = utils::Context::getContext();

			if(::strchr(option, 'D'))
				ctx.setRuntime(utils::RuntimeDynamic);
			else if(::strchr(option, 'T'))
				ctx.setRuntime(utils::RuntimeStatic);
			else
				throw utils::Exception("invalid runtime switch: %s", option);

			return true;
		}

	}
}

