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
#include "DependencyHandler.h"

#include <Log.h>
#include <Statistics.h>

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

		if(std::string(argv[i]) == "--short")
		{

		}

		parity::utils::Path pth(argv[i]);
		pth.toNative();
		if(!pth.exists())
		{
			parity::utils::Log::error("cannot find statistics file: %s\n", pth.get().c_str());
			exit(1);
		}

		//
		// set-up mappings
		//
		parity::statistics::StatisticHandlerMapping timing_mapping = { "Timing", parity::statistics::HandleTiming };
		parity::statistics::StatisticHandlerMapping depend_mapping = { "dependencies", parity::statistics::HandleDependency };
		parity::statistics::available_mappings.push_back(timing_mapping);
		parity::statistics::available_mappings.push_back(depend_mapping);

		parity::statistics::ProcessFile(pth);

		for(parity::statistics::StatisticPrinterVector::iterator it = parity::statistics::available_stats.begin(); it != parity::statistics::available_stats.end(); ++it)
		{
			it->second();
		}
	}
}

namespace parity
{
	namespace statistics
	{
		StatisticPrinterVector available_stats;
		StatisticMappingVector available_mappings;
		bool short_ = false;

		void ProcessFile(const utils::Path& path)
		{
			utils::Statistics::StatisticFile file = utils::Statistics::readStatistics(path.get());

			for(utils::Statistics::StatisticFile::iterator f = file.begin(); f != file.end(); ++f)
			{
				for(utils::Statistics::StatisticCollection::iterator col = f->begin(); col != f->end(); ++col)
				{
					bool proc = false;
					for(StatisticMappingVector::const_iterator map = available_mappings.begin(); map != available_mappings.end(); ++map)
					{
						if(col->key.compare(0, map->key.length(), map->key) == 0)
						{
							map->handler(col->key, col->value, col->type);
							proc = true;
						}
					}

					if(!proc)
						DefaultHandler(col->key, col->value, col->type);
				}
			}
		}

		void DefaultHandler(std::string const& key, std::string const& value, std::string const& type)
		{
			utils::Log::verbose("unhandled statistical information of type %s: %s: %s\n", type.c_str(), key.c_str(), value.c_str());
		}
	}
}

