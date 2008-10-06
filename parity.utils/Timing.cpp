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

#include "Timing.h"
#include "Log.h"

#include "Context.h"
#include "Color.h"

#include "Statistics.h"

#include <iomanip>
#include <stdio.h>

#ifdef _WIN32
#  include <windows.h>
#else
#  include <sys/resource.h>
#endif

namespace parity
{
	namespace utils
	{
		Timing& Timing::instance()
		{
			static Timing tm;
			return tm;
		}

		void Timing::start(const std::string &key)
		{
			SortedListEntry entry;
			TimePair pair;
			ftime(&pair.first);

			if(longest_ < key.length())
				longest_ = key.length();

			entry.first = key;
			entry.second = pair;

			times_.push_back(entry);

			if(key != "Timing")
				Statistics::instance().addInformation("TimingS:" + key, entry.second.first);
		}

		void Timing::stop(const std::string &key)
		{
			for(SortedTimingVector::iterator it = times_.begin(); it != times_.end(); ++it)
			{
				if(it->first == key)
				{
					ftime(&it->second.second);

					//
					// both Timing and Configuration loading can never be
					// there, since both have to be started before a statistic
					// output file is set.
					//
					if(key != "Timing" && key != "Configuration Loading")
						Statistics::instance().addInformation("TimingE:" + key, it->second.second);

					break;
				}
			}
		}

		void Timing::dump()
		{
			if(!times_.size()) {
				return;
			}

			size_t print_w = longest_;

			if(width_)
				print_w = width_ - 17;

			Context& ctx = Context::getContext();
			Color col(ctx.getColorMode());
			Log::profile("\n%s %s\n", col.magenta("   Timing information:").c_str(), forked_ ? col.red("(forked)").c_str() : "");
			Log::profile("   ");
			for(unsigned int i = 0; i < (print_w + 2); ++i)
				Log::profile(col.magenta("-").c_str());	
			Log::profile(col.magenta("-----------\n").c_str());

			time_t all = 0;
			time_t all_calc = 0;

			for(SortedTimingVector::iterator it = times_.begin(); it != times_.end(); ++it)
			{
				time_t sec, ms;
				sec = it->second.second.time - it->second.first.time;
				ms  = it->second.second.millitm - it->second.first.millitm;
				if(ms < 0) {
					ms += 1000;
					sec -= 1;
				}

				#ifdef _WIN32
				#  define snprintf _snprintf
				#endif

				time_t tmp = ((sec * 1000) + ms);

				//
				// Only print if timing was stopped allready for this.
				//
				if(tmp < 0)
					continue;

				char buf[20];
				::snprintf(buf, 19, "%d", tmp);

				std::string col_buf;

				if(tmp < 500) {
					col_buf = col.green(buf);
				} else if(tmp < 1000) {
					col_buf = col.yellow(buf);
				} else {
					col_buf = col.red(buf);
				}

				#ifdef snprintf
				#  undef snprintf
				#endif

				if(it->first[0] == '[') {
					if(Context::getContext().getTimingShowTasks())
						Log::profile("    %-*s: %*s ms\n", print_w + 1 + col.green("").length(), col.green(it->first).c_str(), 6 + col.red("").length(), (tmp < 0 ? col.red("unfin.").c_str() : col_buf.c_str()));
				} else if(it->first == "Timing") {
					all = tmp;
				} else {
					Log::profile("%s%-*s: %*s ms\n", col.cyan(" * ").c_str(), print_w + 2, it->first.c_str(), 6 + col.red("").length(), (tmp < 0 ? col.red("unfin.").c_str() : col_buf.c_str()));
					all_calc += tmp;
				}
			}

			Log::profile("   ");
			for(unsigned int i = 0; i < (print_w + 2); ++i)
				Log::profile(col.magenta("-").c_str());	
			Log::profile(col.magenta("-----------\n").c_str());

			if(all > 0)
				Log::profile("   %-*s: %6d ms\n", print_w + 2, "All together (real)", all);

			if(all_calc > 0)
				Log::profile("   %-*s: %6d ms\n", print_w + 2, "All together (summed)", all_calc);

			if(all > 0 || all_calc > 0) {
				Log::profile("   ");
				for(unsigned int i = 0; i < (print_w + 2); ++i)
					Log::profile(col.magenta("-").c_str());	
				Log::profile(col.magenta("-----------\n").c_str());
				Log::profile("\n");
			}
		}
	}
}

