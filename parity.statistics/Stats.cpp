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

#include <Log.h>

#include <fstream>
#include <time.h>

//
// CONFIX:EXENAME('parity.statistics')
//
int main(int argc, char** argv)
{
	for(int i = 1; i < argc; ++i)
	{
		if(std::string(argv[i]) == "--verbose")
		{
			parity::utils::Log::setLevel(parity::utils::Log::Verbose);
			continue;
		}

		parity::utils::Path pth(argv[i]);
		pth.toNative();
		if(!pth.exists())
		{
			parity::utils::Log::error("cannot find statistics file: %s\n", pth.get().c_str());
			exit(1);
		}

		parity::statistics::ProcessFile(pth);
	}
}

namespace parity
{
	namespace statistics
	{
		static void DumpStatistics(const StatisticsMap& map)
		{
			for(StatisticsMap::const_iterator it = map.begin(); it != map.end(); ++it)
			{
				std::cout << "Statistics for: " << it->first << std::endl;
				std::cout << " * Total Time spent     : " << it->second.all / 1000 << " sec (" << it->second.all << " ms)" << std::endl;
				std::cout << " * Average Time spent   : " << it->second.average << " ms" << std::endl;
				std::cout << " * Number of entries    : " << it->second.count << std::endl;

				const int TIME_BUFSIZE = 1024;
				char strStart[TIME_BUFSIZE];
				char strStop[TIME_BUFSIZE];

				strftime(strStart, TIME_BUFSIZE, "%d.%m, %H:%M:%S", localtime(&it->second.oldest));
				strftime(strStop, TIME_BUFSIZE, "%d.%m, %H:%M:%S", localtime(&it->second.youngest));

				std::cout << " * Measurement Timespan : " << strStart << " - " << strStop << std::endl << std::endl;
			}
		}

		void ProcessFile(const utils::Path& path)
		{
			utils::Log::verbose("processing %s...\n", path.get().c_str());
			std::ifstream ifs(path.get().c_str());

			if(!ifs.is_open())
			{
				utils::Log::error("cannot open %s\n", path.get().c_str());
				exit(1);
			}

			TimingInfoVector timings;

			while(!ifs.fail())
			{
				TimingInfoStruct item;
				char buf[1024];

				ifs.getline(buf, 1024);
				std::string line = buf;

				std::string::size_type start = 0;
				std::string::size_type end = line.find('|');
				item.name = line.substr(start, end);

				if(item.name.empty())
					break;

				start = end + 1;
				end = line.find(':', start);
				item.start = atoi(line.substr(start, end).c_str());
				start = end + 1;
				end = line.find('|', start);
				item.start_milli = atoi(line.substr(start, end).c_str());

				start = end + 1;
				end = line.find(':', start);
				item.stop = atoi(line.substr(start, end).c_str());
				start = end + 1;
				end = line.find('|', start);
				item.stop_milli = atoi(line.substr(start, end).c_str());

				start = end + 1;
				end = line.find('|', start);
				item.time = atoi(line.substr(start, end).c_str());

				//
				// calculate nicer names...
				//
				if(item.name[0] == '[')
					item.name = item.name.substr(item.name.find(']') + 2);

				timings.push_back(item);
			}

			ifs.close();

			//
			// all timing information parsed into a TimingInfoVector, now accumulate...
			//
			StatisticsMap stats;

			for(TimingInfoVector::iterator it = timings.begin(); it != timings.end(); ++it)
			{
				StatisticsStruct& item = stats[it->name];

				item.count++;
				item.all += it->time;
				item.average = item.all / item.count;

				if(it->start < item.oldest || item.oldest == 0)
					item.oldest = it->start;
				
				if(it->stop > item.youngest || item.youngest == 0)
					item.youngest = it->stop;
			}

			DumpStatistics(stats);
		}
	}
}

