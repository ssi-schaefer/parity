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

#ifndef __STATISTICS_H__
#define __STATISTICS_H__

#include <Path.h>

#include <vector>
#include <map>
#include <string>

namespace parity
{
	namespace statistics
	{
		typedef struct {
			std::string name;
			time_t start;
			unsigned short start_milli;
			time_t stop;
			unsigned short stop_milli;
			time_t time;
		} TimingInfoStruct;

		typedef struct {
			time_t all;
			time_t average;
			unsigned int count;
			time_t oldest;
			time_t youngest;
		} StatisticsStruct;

		typedef std::vector<TimingInfoStruct> TimingInfoVector;
		typedef std::map<std::string, StatisticsStruct> StatisticsMap;

		void ProcessFile(const utils::Path& path);
	}
}

#endif

