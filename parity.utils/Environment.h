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

#ifndef __ENVIRONMENT_H__
#define __ENVIRONMENT_H__

#include "Path.h"
#include "Context.h"

#include <string>
#include <list>

namespace parity
{
	namespace utils
	{
		class Environment
		{
		public:
			Environment(const std::string& value);

			std::string getValue();
			Path getPath();
			PathVector getPathVector();

			bool extend(const Path& path);
			bool set(const std::string& value);
			bool clear();

			static Environment fromString(const std::string& value);

		private:
			Environment() : value_(), varname_() {}
			std::string value_;
			std::string varname_;
		};
	}
}

#endif

