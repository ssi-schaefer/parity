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
		{
			Context& ctx = Context::getContext();
			Path f = ctx.getStatisticsFile();

			if(!f.get().empty()) {
				f.toNative();
				file_.open(f.get().c_str(), std::ios_base::app | std::ios_base::out);

				file_ << "run @ " << ctime(0) << "{" << std::endl;
			}
		}

		Statistics::~Statistics()
		{
			if(file_.is_open()) {
				file_ << "}" << std::endl;
				file_.close();
			}
		}

		void Statistics::addInformation(std::string const& key, std::string const& text) {
			if(!file_.is_open()) return;

			file_ << "\t{" << "[text]" << key << "}{" << text << "}" << std::endl;
		}

		void Statistics::addInformation(std::string const& key, unsigned long number) {
			if(!file_.is_open()) return;

			file_ << "\t{" << "[number]" << key << "}{" << number << "}" << std::endl;
		}

		void Statistics::addInformation(std::string const& key, struct timeb const& time) {
			if(!file_.is_open()) return;

			file_ << "\t{" << "[time]" << key << "}{" << serializeTime(time) << "}" << std::endl;
		}

		std::string Statistics::serializeTime(struct timeb const& tm)
		{
			std::ostringstream oss;
			oss << "[" << tm.time << "|" << tm.millitm << "]" << std::endl;
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
	}
}

