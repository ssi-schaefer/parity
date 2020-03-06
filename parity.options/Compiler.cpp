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
#include <Statistics.h>

namespace parity
{
	namespace options
	{
		bool setSyntaxOnly(const char* OPT_UNUSED(option), const char* OPT_UNUSED(argument), bool& OPT_UNUSED(used))
		{
			utils::Context& ctx = utils::Context::getContext();
			ctx.setSyntaxOnly(true);
			return true;
		}

		bool setCompileOnly(const char* OPT_UNUSED(option), const char* OPT_UNUSED(argument), bool& OPT_UNUSED(used))
		{
			utils::Context& ctx = utils::Context::getContext();
			ctx.setCompileOnly(true);
			return true;
		}

		bool setExceptionHandling(const char* option, const char* OPT_UNUSED(argument), bool& OPT_UNUSED(used))
		{
			utils::Context& ctx = utils::Context::getContext();
			ctx.setExceptionHandling(!::strstr(option, "no"));

			return true;
		}

		bool setRuntimeTypes(const char* option, const char* OPT_UNUSED(argument), bool& OPT_UNUSED(used))
		{
			utils::Context& ctx = utils::Context::getContext();
			ctx.setExceptionHandling((!::strstr(option, "no") && !(option[::strlen(option) - 1] == '-')));
			return true;
		}

		bool setPositionIndep(const char* OPT_UNUSED(option), const char* OPT_UNUSED(argument), bool& OPT_UNUSED(used))
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

		bool setCxxStandard(const char* option, const char* argument, bool& used)
		{
			utils::Context& ctx = utils::Context::getContext();
			/* no need to map the MS options */
			if (strcmp(option, "/Zc:__cplusplus-") == 0) {
				ctx.setCplusPlusMacro(false);
				return true;
			}
			if (strcmp(option, "/Zc:__cplusplus") == 0) {
				ctx.setCplusPlusMacro(true);
				return true;
			}
			if (strncmp(option, "/std:c++", 8) == 0) {
				ctx.setCxxStandardOption(option);
				return true;
			}
			/* map the GNU options */
			static struct {
				char const *gnuOption;
				char const *msOption;
				bool cxxMacro;
			} const mapping[] = {
				{ "-ansi",      "",               false },
				{ "-std=c++98", "",               false },
				{ "-std=c++03", "",               true },
				{ "-std=c++11", "",               true },
				{ "-std=c++14", "/std:c++14",     true },
				{ "-std=c++17", "/std:c++17",     true },
				{ "-std=c++2a", "/std:c++latest", true },
			};
			for(int i = 0; i < sizeof(mapping)/sizeof(mapping[0]); ++i) {
				if (strcmp(mapping[i].gnuOption, option) == 0) {
					ctx.setCxxStandardOption(mapping[i].msOption);
					ctx.setCplusPlusMacro(mapping[i].cxxMacro);
					return true;
				}
			}
			return false;
		}

		bool addSource(const char* option, const char* argument, bool& used)
		{
			utils::Context& ctx = utils::Context::getContext();
			const char * arg = option;
			utils::Path pth;

			if((option[0] == '-' || option[0] == '/') && option[1] == 'T')
			{
				arg = &option[3];
				if(!*arg) {
					arg = argument;
					used = true;
				}

				pth = utils::Path(arg);

				if(option[2] == 'p')
					ctx.getSources()[pth] = utils::LanguageCpp;
				else if(option[2] == 'c')
					ctx.getSources()[pth] = utils::LanguageC;
				else
					throw utils::Exception("wrong language specifier in %s!", option);
			} else {
				if (strncmp(option, "-i", 2) == 0) {
					arg = &option[2];
					if(!*arg) {
						arg = argument;
						used = true;
					}
				} else
				if (strcmp(option, "--include") == 0) {
					arg = argument;
					used = true;
				}
				pth = utils::Path(arg);
				ctx.setSourcesString(arg);
			}

			utils::Statistics::instance().addInformation("file-source", pth.get());

			return true;
		}
		
		bool addSourceFromStdin(const char* option, const char* argument, bool& used)
		{
			if(option[0] != '-' || option[1] != '\0')
				return false;

			utils::Context& ctx = utils::Context::getContext();
			utils::Path tmp = utils::Path::getTemporary(".parity.stdin.XXXXXX.c");
			std::ofstream tmp_src(tmp.get().c_str());

			while(!std::cin.eof()) {
				char buffer[4096]; // uh oh... will this suffice *always*?

				std::cin.getline(buffer, sizeof(buffer));

				if(std::cin.fail())
					break;

				tmp_src << buffer << std::endl;
			}

			tmp_src.close();

			// as the file does exist now:
			// recreate an utils::Path instance to expand the path
			utils::Path pth(tmp.get());

			if(ctx.getForcedLanguage() != utils::LanguageInvalid)
				ctx.getSources()[pth] = ctx.getForcedLanguage();
			else
				ctx.getSources()[pth] = utils::LanguageC;

			ctx.getTemporaryFiles().push_back(pth);

			utils::Statistics::instance().addInformation("file-source", pth.get());

			return true;
		}

		bool setShortWchar(const char* option, const char* OPT_UNUSED(argument), bool& OPT_UNUSED(used))
		{
			utils::Context& ctx = utils::Context::getContext();
			ctx.setShortWchar(!::strstr(option, "no"));
			return true;
		}

		bool setRuntime(const char* option, const char* OPT_UNUSED(argument), bool& OPT_UNUSED(used))
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

