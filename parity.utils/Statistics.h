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

#ifndef __STATS_H__
#define __STATS_H__

#include <map>
#include <vector>
#include <string>
#include <fstream>

#include <sys/timeb.h>

namespace parity
{
	namespace utils
	{
		class Statistics {
		public:
			static Statistics& instance();

			void addInformation(std::string const& key, std::string const& text);
			void addInformation(std::string const& key, unsigned long number);
			void addInformation(std::string const& key, struct timeb const& time);

			static std::string serializeTime(struct timeb const& tm);
			static struct timeb deserializeTime(std::string const& str);

			void start();
			void forked() { forked_ = true; }

			//
			// for reading.
			//
			typedef struct {
				std::string key;
				std::string value;
				std::string type;
			} StatisticItem;

			typedef std::vector<StatisticItem> StatisticCollection;
			typedef std::vector<StatisticCollection> StatisticFile;

			static StatisticFile readStatistics(std::string const& fn);
		private:
			Statistics();
			~Statistics();

			static StatisticCollection readStatisticsSection(std::ifstream& ifs);
			static StatisticItem readStatisticsItem(std::string const& str);

			std::ofstream file_;
			bool forked_;
		};
	}
}

#endif

