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

#include "Log.h"
#include "Context.h"
#include "Color.h"

#include <stdio.h>

#ifndef _WIN32
#  include <unistd.h>
#endif

namespace parity
{
	namespace utils
	{

		Log::Level Log::level_ = Log::Warning;
		Color Log::color_ = Color(Color::Bright);
		int Log::lock_;

		void Log::setLevel(Log::Level lvl)
		{
			level_ = lvl;
		}

		#ifdef _WIN32
		#  define usleep(x) Sleep(x / 1000)
		#endif

		int Log::verbose(const char* fmt, ...)
		{
			int ret = 0;

			if(level_ == Verbose) {
				va_list args;
				va_start(args, fmt);

				while(lock_ > 0) usleep(10);

				++lock_;
				//fprintf(stderr, "verbose: ");
				ret = vfprintf(stdout, fmt, args);
				--lock_;

				va_end(args);
			}

			return ret;
		}

		int Log::profile(const char* fmt, ...)
		{
			int ret = 0;

			if(level_ <= Profile) {
				va_list args;
				va_start(args, fmt);

				while(lock_ > 0) usleep(10);

				++lock_;
				//fprintf(stderr, "profile: ");
				ret = vfprintf(stdout, fmt, args);
				--lock_;

				va_end(args);
			}

			return ret;
		}

		int Log::warning(const char* fmt, ...)
		{
			int ret = 0;
			Color col(Context::getContext().getColorMode());

			if(level_ <= Warning) {
				va_list args;
				va_start(args, fmt);

				while(lock_ > 0) usleep(10);

				++lock_;
				fprintf(stderr, col.yellow("warning: ").c_str());
				ret = vfprintf(stderr, fmt, args);
				--lock_;

				va_end(args);
			}

			return ret;
		}

		int Log::error(const char* fmt, ...)
		{
			int ret = 0;
			Color col(Context::getContext().getColorMode());

			va_list args;
			va_start(args, fmt);

			while(lock_ > 0) usleep(10);

			++lock_;
			fprintf(stderr, col.red("error  : ").c_str());
			ret = vfprintf(stderr, fmt, args);
			--lock_;

			va_end(args);

			return ret;
		}

	}
}

