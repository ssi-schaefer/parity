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

#ifndef __LOG_H__
#define __LOG_H__

#include <string>
#include <iostream>
#include <fstream>

#include "Color.h"
#include "FormatStrings.h"

#ifdef _WIN32
#  include <windows.h>
#else
#  include <unistd.h>
#endif

namespace parity
{
	namespace utils
	{
		class Log
		{
		public:

			typedef enum {
				Verbose,
				Profile,
				Warning,
				Error
			} Level;

			static int verbose(const char* fmt, ...) FORMAT_CHECK(1, 2);
			static int profile(const char* fmt, ...) FORMAT_CHECK(1, 2);
			static int warning(const char* fmt, ...) FORMAT_CHECK(1, 2);
			static int error(const char* fmt, ...)   FORMAT_CHECK(1, 2);

			static void setLevel(Log::Level lvl);
			static void setColor(const Color& col) { color_ = col; }
			
		private:
			static Level level_;
			static Color color_;
			static int lock_;
		};
	}
}

#endif

