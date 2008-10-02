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

#include "Stats.h"
#include "DependencyHandler.h"

#include <Log.h>
#include <Statistics.h>

#include <map>
#include <iomanip>

namespace parity
{
	namespace statistics
	{
		static std::map<std::string, unsigned long> deps_f_;
		static unsigned long longest_ = 0;

		void HandleDependency(std::string const& key, std::string const& value, std::string const&)
		{
			available_stats["dependency"] = PrintDependency;
			std::string file = key.substr(key.find('(') + 1, key.find(')') - (key.find('(') + 1));
			deps_f_[file] = atol(value.c_str());

			if(file.length() > longest_)
				longest_ = file.length();
		}

		void PrintDependency()
		{
			unsigned long all = 0;

			std::cout << std::setw(longest_ + 1 + 10) << std::setfill('-') << "-" << std::setfill(' ') << std::endl;
			std::cout << "Dependency Statistics:" << std::endl;
			std::cout << std::left << std::setw(longest_ + 1) << "file" << std::setw(10) << "dep. count" << std::endl;
			std::cout << std::setw(longest_ + 1 + 10) << std::setfill('-') << "-" << std::setfill(' ') << std::endl;

			if(!short_)
			{
				for(std::map<std::string, unsigned long>::iterator it = deps_f_.begin(); it != deps_f_.end(); ++it)
				{
					std::cout << std::left << std::setw(longest_ + 1) << it->first << std::setw(10) << std::right << it->second << std::endl;
					all += it->second;
				}
			}

			std::cout << std::setw(longest_ + 1 + 10) << std::setfill('-') << "-" << std::setfill(' ') << std::endl;
			std::cout << std::left << std::setw(longest_ + 1) << "sum" << std::setw(10) << std::right << all << std::endl;
			std::cout << std::left << std::setw(longest_ + 1) << "average" << std::setw(10) << std::right << ((float)all/deps_f_.size()) << std::endl;
			std::cout << std::setw(longest_ + 1 + 10) << std::setfill('-') << "-" << std::setfill(' ') << std::endl;
		}
	}
}
