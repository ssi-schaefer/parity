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
#include "TimingHandler.h"

#include <Log.h>
#include <Statistics.h>

#include <map>
#include <iomanip>

namespace parity
{
	namespace statistics
	{
		typedef std::map<std::string, time_t> TimingMap;
		typedef std::map<std::string, struct timeb> OpenTimingMap;
		static TimingMap timings_;
		static TimingMap avg_;
		static TimingMap counts_;
		static OpenTimingMap open_;
		static TimingMap invalids_;
		static std::string::size_type longest_ = 0;

		void HandleTiming(std::string const& key, std::string const& value, std::string const&)
		{
			available_stats["timing"] = PrintTiming;
			std::string::size_type posKey = key.find(':');
			std::string k_new = key.substr(posKey + 1);

			if(k_new[0] == '[') k_new = " -> " + k_new.substr(k_new.find(']') + 2);

			if(key.substr(0, posKey) == "TimingS") {
				open_[k_new] = utils::Statistics::deserializeTime(value);
			} else if(key.substr(0, posKey) == "TimingE") {
				std::string k = k_new;
				OpenTimingMap::iterator start = open_.find(k);

				if(start == open_.end()) {
					utils::Log::verbose("found end of timing before start for key %s\n", k.c_str());
					invalids_[k]++;
					return;
				}

				struct timeb end = utils::Statistics::deserializeTime(value);

				time_t sec, ms;
				sec = end.time - start->second.time;
				ms  = end.millitm - start->second.millitm;
				if(ms < 0) {
					ms += 1000;
					sec -= 1;
				}

				if(counts_[k]++ == 0)
					avg_[k] = ((sec*1000) + ms);
				else
					avg_[k] = (avg_[k] + ((sec * 1000) + ms)) / 2;

				timings_[k] += ((sec * 1000) + ms);

				if(k.length() > longest_)
					longest_ = k.length();

				open_.erase(start);
			} else {
				utils::Log::warning("ignoring malformed timing information key: %s\n", key.c_str());
				return;
			}
		}

		void PrintTiming()
		{
			time_t all = 0;

			std::cout << std::setw(longest_ + 1 + 36) << std::setfill('-') << "-" << std::setfill(' ') << std::endl;
			std::cout << "Timing Statistics: (N/S = never started, N/F = never finished)" << std::endl;
			std::cout << std::left << std::setw(longest_ + 1) << "Task" << std::setw(13) << "add-up" << std::setw(13) << "average" << std::setw(10) << std::right << "count" << std::endl;
			std::cout << std::setw(longest_ + 1 + 36) << std::setfill('-') << "-" << std::setfill(' ') << std::endl;

			for(TimingMap::iterator it = invalids_.begin(); it != invalids_.end(); ++it)
			{
				std::cout << std::left << std::setw(longest_ + 1) << it->first << std::setw(10) << "N/S" << "ms " << std::setw(10) << "N/S" << "ms " << std::setw(10) << std::right << invalids_[it->first] << std::endl;
			}

			for(TimingMap::iterator it = timings_.begin(); it != timings_.end(); ++it)
			{
				std::cout << std::left << std::setw(longest_ + 1) << it->first << std::setw(10) << it->second << "ms " << std::setw(10) << avg_[it->first] << "ms " << std::setw(10) << std::right << counts_[it->first] << std::endl;
				all += it->second;
			}

			for(OpenTimingMap::iterator it = open_.begin(); it != open_.end(); ++it)
			{
				std::cout << std::left << std::setw(longest_ + 1) << it->first << std::setw(10) << "N/F" << "ms " << std::setw(10) << "N/F" << "ms " << std::setw(10) << std::right << "N/A" << std::endl;
			}
			
			std::cout << std::setw(longest_ + 1 + 36) << std::setfill('-') << "-" << std::setfill(' ') << std::endl;
			std::cout << std::left << std::setw(longest_ + 1) << "sum" << std::setw(10) << all << "ms " << std::setw(10) << (all/timings_.size()) << "ms" << std::endl;
			std::cout << std::setw(longest_ + 1 + 36) << std::setfill('-') << "-" << std::setfill(' ') << std::endl;
		}
	}
}
