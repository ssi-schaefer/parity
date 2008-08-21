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

#define GENERAL_CONFIG_KEY "__general_config__"

namespace parity
{
	namespace utils
	{
		void Config::parseFile(Context& ctx, const MappedFile& file, int argc, char * const * argv)
		{
			utils::Log::verbose("loading %s\n", file.getPath().get().c_str());

			ConfigSectionMap m = getSections(file);

			if(m.empty()) {
				utils::Log::verbose("no sections found, parsing whole file\n");
				if(!parseString(ctx, (const char*)file.getBase(), (char*)file.getTop() - (char*)file.getBase()))
					throw Exception("cannot parse configuration file %s", file.getPath().get().c_str());
			} else {
				int iLoadedSections = 0;
				utils::Log::verbose("found %d sections, give -<section name> to activate on command line\n", m.size());

				ConfigSectionMap::iterator pos = m.find(GENERAL_CONFIG_KEY);

				if(pos != m.end() && pos->second.second > 0) {
					utils::Log::verbose("loading global information from %s\n", file.getPath().get().c_str());
					if(!parseString(ctx, pos->second.first, pos->second.second))
						throw Exception("cannot parse configuration file %s", file.getPath().get().c_str());
				}

				for(ConfigSectionMap::iterator it = m.begin(); it != m.end(); ++it) {
					for(int i = 1; i < argc; ++i) {
						std::string arg = "-";
						arg.append(it->first);

						if(arg.compare(argv[i]) == 0) {
							utils::Log::verbose("loading section %s from %s\n", it->first.c_str(), file.getPath().get().c_str());
							if(!parseString(ctx, it->second.first, it->second.second))
								throw Exception("cannot parse configuration file %s", file.getPath().get().c_str());

							//
							// now ignore this parameter later on
							//
							argv[i][0] = '\0';
							iLoadedSections++;
						}
					}
				}

				//
				// load default section, but only if no other one has been loaded
				// by now.
				//

				if(iLoadedSections)
					return;

				pos = m.find("default");

				if(pos != m.end() && pos->second.second > 0) {
					utils::Log::verbose("loading default information from %s\n", file.getPath().get().c_str());
					if(!parseString(ctx, pos->second.first, pos->second.second))
						throw Exception("cannot parse configuration file %s", file.getPath().get().c_str());
				}
			}
		}

		ConfigSectionMap Config::getSections(const MappedFile& file) {
			const char* top = (const char*)file.getTop();
			const char* line = (const char*)file.getBase();
			const char* end = line;
			ConfigSectionMap m;
			int firstSec = 1;

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
					while(*line == ' ' || *line == '\t') ++line;

					if(strncmp(line, "section", strlen("section")) == 0) {
						if(firstSec) {
							firstSec = 0;

							ConfigSection general;

							general.first = (const char*)file.getBase();
							general.second = line - general.first;

							m[GENERAL_CONFIG_KEY] = general;
						}

						line += strlen("section");
						
						while((*line == ' ' || *line == '\t') && line < top ) ++line;
						const char* e = line;
						while(*e != ' ' && *e != '\t' && *e != '{' && e < top) ++e;

						std::string id(line, e - line);

						ConfigSection s;

						while(*e != '\r' && *e != '\n' && *e != '\0' && e < top) ++e;
						s.first = e;
						
						while(*e != '}' && e < top) ++e;
						s.second = e - s.first; // intentionally without +1, since we don't want the '{'

						m[id] = s;
					}
				}

				if(end + 1 >= top)
					break;

				line = end + 1;
			}

			return m;
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

