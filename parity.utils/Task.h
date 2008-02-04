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

#ifndef __TASK_H__
#define __TASK_H__

#include "Path.h"

#include <vector>
#include <map>
#include <string>
#include <iostream>

namespace parity
{
	namespace utils
	{
		class Task {
		public:
			Task() : out_(&std::cout), err_(&std::cerr) {}

			typedef std::vector<std::string> ArgumentVector;
			typedef std::vector<std::string> OutputVector;
			//               data         match whole line only?
			typedef std::map<std::string, bool> FilterMap;

			void addFilter(const std::string& filt, bool wholeLine) { filters_[filt] = wholeLine; }
			bool execute(const Path& executable, const ArgumentVector& arguments);

			bool prepareEnvironment();

			void createCommandScript(ArgumentVector& vec);

			void setOutStream(std::ostream& out) { out_ = &out; }
			void setErrStream(std::ostream& err) { err_ = &err; }
		private:
			FilterMap filters_;
			std::ostream* out_;
			std::ostream* err_;

			void colorizeLine(std::string& input);
			void filterLine(std::string& input);

			void processLine(std::string& line, bool stdOut);
			void processBlock(const char* input, size_t size, bool stdOut);

			static unsigned int taskCounter_;
		};
	}
}

#endif

