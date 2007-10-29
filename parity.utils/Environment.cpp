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

#include "Environment.h"

#include <stdlib.h>

namespace parity
{
	namespace utils
	{

		Environment::Environment(const std::string &value)
		{
			const char * env = getenv(value.c_str());

			if(env)
				value_ = env;
		}

		std::string Environment::getValue()
		{
			return value_;
		}

		Path Environment::getPath()
		{
			return Path(value_);
		}

		PathVector Environment::getPathVector()
		{
			char seperator = ':';
			PathVector pathList;

			if(value_.empty())
				return pathList;

			if(::strchr(value_.c_str(), ';') || ::strchr(value_.c_str(), '\\'))
				seperator = ';';

			std::string::iterator start = value_.begin();
			std::string::iterator end = start;

			while(true)
			{
				while(end != value_.end() && *end != seperator)
					++end;

				std::string path(start, end);

				if(!path.empty())
					pathList.push_back(Path(path));

				if(end == value_.end())
					break;

				start = ++end;
			}

			return pathList;
		}

		Environment Environment::fromString(const std::string &value)
		{
			Environment env;
			env.value_ = value;

			return env;
		}
	}
}

