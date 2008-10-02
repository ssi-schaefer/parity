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

#include "Statistics.h"
#include "Log.h"

#include "Context.h"
#include "Color.h"

#include <time.h>
#include <sstream>

#define STAT_START_STR	"parity.statistics {"
#define STAT_END_STR	"} end"
#define STAT_ITEM_OPEN	"{"
#define STAT_ITEM_CLOSE	"}"
#define STAT_TYPE_OPEN	"["
#define STAT_TYPE_CLOSE	"]"

namespace parity
{
	namespace utils
	{
		Statistics& Statistics::instance()
		{
			static Statistics st;
			return st;
		}

		Statistics::Statistics()
			: file_(), forked_(false)
		{
			start();
		}

		void Statistics::start()
		{
			if(file_.is_open()) {
				utils::Log::verbose("Statistics already started, ignoring second request!");
				return;
			}

			Context& ctx = Context::getContext();
			Path f = ctx.getStatisticsFile();

			if(!f.get().empty()) {
				f.toNative();
				file_.open(f.get().c_str(), std::ios_base::app | std::ios_base::out);
				file_ << STAT_START_STR << std::endl;

				utils::Log::verbose("starting capture of statistics to %s\n", f.get().c_str());
			}
		}

		Statistics::~Statistics()
		{
			if(!forked_ && file_.is_open()) {
				file_ << STAT_END_STR << std::endl;
				file_.close();
			}
		}

		void Statistics::addInformation(std::string const& key, std::string const& text) {
			if(!file_.is_open()) return;

			file_ << "\t" << STAT_ITEM_OPEN << STAT_TYPE_OPEN << "text" << STAT_TYPE_CLOSE << key << STAT_ITEM_CLOSE << STAT_ITEM_OPEN << text << STAT_ITEM_CLOSE << std::endl;
		}

		void Statistics::addInformation(std::string const& key, unsigned long number) {
			if(!file_.is_open()) return;

			file_ << "\t" << STAT_ITEM_OPEN << STAT_TYPE_OPEN << "number" << STAT_TYPE_CLOSE << key << STAT_ITEM_CLOSE << STAT_ITEM_OPEN << number << STAT_ITEM_CLOSE << std::endl;
		}

		void Statistics::addInformation(std::string const& key, struct timeb const& time) {
			if(!file_.is_open()) return;

			file_ << "\t" << STAT_ITEM_OPEN << STAT_TYPE_OPEN << "time" << STAT_TYPE_CLOSE << key << STAT_ITEM_CLOSE << STAT_ITEM_OPEN << serializeTime(time) << STAT_ITEM_CLOSE << std::endl;
		}

		std::string Statistics::serializeTime(struct timeb const& tm)
		{
			std::ostringstream oss;
			oss << "[" << tm.time << "|" << tm.millitm << "]";
			return oss.str();
		}

		struct timeb Statistics::deserializeTime(const std::string &str)
		{
			struct timeb tm;

			std::string::size_type posStart = str.find('[');
			std::string::size_type posEnd = std::string::npos;

			if(posStart == std::string::npos)
				throw utils::Exception("cannot deserialize time from string: cannot find start of time value");

			++posStart;
			posEnd = str.find('|', posStart);

			if(posEnd == std::string::npos)
				throw utils::Exception("cannot deserialize time from string: cannot find end of seconds.");

			tm.time = atol(str.substr(posStart, posEnd - posStart).c_str());

			posStart = posEnd+1;
			posEnd = str.find(']', posStart);

			if(posEnd == std::string::npos)
				throw utils::Exception("cannot deserialize time from string: cannot find end of milliseconds.");

			tm.millitm = atoi(str.substr(posStart, posEnd - posStart).c_str());

			return tm;
		}

		static inline void stripBackslashR(std::string& r)
		{
			std::string::size_type pos;
			while((pos = r.find('\r')) != std::string::npos)
				r.replace(pos, 1, "");
		}

		Statistics::StatisticFile Statistics::readStatistics(std::string const& fn)
		{
			StatisticFile stats;
			std::ifstream istr(fn.c_str());

			while(!istr.fail())
			{
				char buffer[1024];
				istr.getline(buffer, 1024);
				std::string line(buffer);
				stripBackslashR(line);

				if(line == STAT_START_STR)
					stats.push_back(readStatisticsSection(istr));
				else if(line != "")
					utils::Log::warning("line not belonging to any statistics sections: %s\n", line.c_str());
			}

			return stats;
		}

		Statistics::StatisticCollection Statistics::readStatisticsSection(std::ifstream& ifs)
		{
			StatisticCollection stats;

			while(!ifs.fail())
			{
				char buffer[1024];
				ifs.getline(buffer, 1024);
				std::string line(buffer);
				stripBackslashR(line);

				if(line == STAT_END_STR)
					break;

				try {
					if(line != "")
						stats.push_back(readStatisticsItem(line));
				} catch(const utils::Exception& e) {
					utils::Log::verbose("cannot parse statistic item: %s: %s\n", line.c_str(), e.what());
				}
			}

			return stats;
		}

		Statistics::StatisticItem Statistics::readStatisticsItem(std::string const& str)
		{
			StatisticItem item;
			std::string::size_type posStart	= std::string::npos;
			std::string::size_type posEnd	= std::string::npos;

			posStart = str.find(STAT_ITEM_OPEN);
			posEnd	 = str.find(STAT_ITEM_CLOSE, ++posStart);

			if(posStart == std::string::npos || posEnd == std::string::npos)
				throw utils::Exception("cannot find key item boundaries!");

			std::string raw_key(str, posStart, posEnd - posStart);

			std::string::size_type posTStart = raw_key.find(STAT_TYPE_OPEN);
			std::string::size_type posTEnd	 = raw_key.find(STAT_TYPE_CLOSE, ++posTStart);

			if(posTStart == std::string::npos || posTEnd == std::string::npos)
				throw utils::Exception("cannot find type boundaries!");

			item.type = std::string(raw_key, posTStart, posTEnd - posTStart);
			item.key  = std::string(raw_key, posTEnd + 1);

			posStart = str.find(STAT_ITEM_OPEN, posEnd);
			posEnd	 = str.find(STAT_ITEM_CLOSE, ++posStart);

			if(posStart == std::string::npos || posEnd == std::string::npos)
				throw utils::Exception("cannot find value item boundaries!");

			item.value = std::string(str, posStart, posEnd - posStart);

			return item;
		}

	}
}

