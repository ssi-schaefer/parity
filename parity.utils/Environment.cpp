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

			varname_ = value;
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

		bool Environment::extend(const Path& path)
		{
#ifdef _WIN32
			char sep = ';';
#else
			char sep = ':';
#endif

			if(!path.isNative())
				throw Exception("path must be in native format!");

			//
			// strip trailing seperators
			//
			while(value_[value_.length()] == sep)
				value_ = value_.substr(0, value_.length() -1);

			//
			// Now append one seperator.
			//
			value_ += sep;

			//
			// now the value
			//
			value_.append(path.get());

			return set(value_);
		}

		bool Environment::set(const std::string& value)
		{
			value_ = value;
#ifdef _WIN32
			return (SetEnvironmentVariableA(varname_.c_str(), value.c_str()) == TRUE);
#else
			return (setenv(varname_.c_str(), value.c_str(), 1) == 0);
#endif
		}
	}
}

