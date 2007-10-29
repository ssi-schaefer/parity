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

#include "Configuration.h"

#include "Context.h"
#include "Exception.h"
#include "Log.h"

#include <iomanip>

namespace parity
{
	namespace utils
	{
		void Config::parseFile(Context& ctx, const MappedFile& file)
		{
			utils::Log::verbose("loading %s\n", file.getPath().get().c_str());

			if(!parseString(ctx, (const char*)file.getBase(), (char*)file.getTop() - (char*)file.getBase()))
				throw Exception("cannot parse configuration file %s", file.getPath().get().c_str());
		}

		bool Config::parseString(Context& ctx, const char *data, size_t len)
		{
			const char * top = data + len;
			const char * line = data;
			const char * end = line;

			while(true)
			{
				end = line;

				while(*end != '\r' && *end != '\n')
				{
					if(end >= top)
						break;

					++end;
				}

				if(end > line)
				{
					if(!parseLine(ctx, line, end - line))
						return false;
				}

				if(end + 1 >= top)
					break;

				line = end + 1;
			}

			return true;
		}

		bool Config::parseLine(Context& ctx, const char *line, size_t len)
		{
			const char * start = line;
			const char * end = 0;
			const char * equals = strchr(line, '=');

			while(*start == ' ' || *start == '\t')
				++start;

			if(*start == '#')
				return true;

			if(!equals || equals >= (line + len))
				return true;

			end = equals;

			while(*end == '=' || *end == ' ' || *end == '\t')
				--end;

			std::string option = std::string(start, (end - start) + 1);

			start = equals;

			while(*start == '=' || *start == ' ' || *start == '\t')
				++start;

			end = strchr(start, '#');

			if(end >= (line + len) || !end)
				end = (line + len - 1);

			while(*end == ' ' || *end == '\t' || *end == '#' || *end == '\r' || *end == '\n')
				--end;

			if(end < start)
				return true;

			std::string value = std::string(start, (end - start) + 1);
			const ContextMap* ptr = ctx.getMapping(option);

			if(!ptr)
			{
				Log::warning("unknown configuration option: \"%s\".\n", option.c_str());
				return false;
			} else {
				(ctx.*(ptr->setter))(value);
				Log::verbose("config: %s: %s\n", option.c_str(), value.c_str());
			}

			return true;
		}
	}
}

