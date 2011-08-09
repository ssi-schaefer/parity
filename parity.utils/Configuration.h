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

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <map>
#include <string>

#include "MappedFile.h"
#include "Context.h"

#define GENERAL_CONFIG_KEY "__general_config__"

namespace parity
{
	namespace utils
	{
		typedef std::pair<const char*, size_t> ConfigSection;
		typedef std::map<std::string, ConfigSection> ConfigSectionMap;

		class Config {
		public:
			static void parseFile(Context& ctx, const MappedFile& file, int argc, char * const * argv);
			static bool parseString(Context& ctx, const char* line, size_t len);
			static ConfigSectionMap getSections(const MappedFile& file);

		private:
			Config();
			static bool parseLine(Context& ctx, const char* data, size_t len);
		};
	}
}

#endif

