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

#ifndef __TIMING_H__
#define __TIMING_H__

#include <map>
#include <vector>
#include <string>

#include <sys/timeb.h>

namespace parity
{
	namespace utils
	{
		typedef std::pair<struct timeb, struct timeb> TimePair;
		typedef std::pair<std::string, TimePair> SortedListEntry;
		typedef std::vector<SortedListEntry> SortedTimingVector;

		class Timing {
		public:
			~Timing() { stop("Timing"); dump(); }
			static Timing& instance();
			void start(const std::string& key);
			void stop(const std::string& key);

			void setOutputWidth(size_t w) { width_ = w; }

			void dump();

			void forked() { forked_ = true; times_.clear(); longest_ = 0; }

			const SortedTimingVector& getCurrentState() { return times_; }
		private:
			Timing() : times_(), longest_(0), forked_(false), width_(0) { start("Timing"); }

			SortedTimingVector times_;
			size_t longest_;

			bool forked_;
			size_t width_;
		};
	}
}

#endif

