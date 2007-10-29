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

#include "Exception.h"

#include <stdarg.h>
#include <cstring>
#include <cstdio>

namespace parity
{
	namespace utils
	{

		Exception::Exception(const char * fmt, ...)
		{
			va_list args;
			va_start(args, fmt);

			size_t length = getLength(fmt, args);
			buffer_ = new char[length + 1];
			::vsnprintf(buffer_, length + 1, fmt, args);

			va_end(args);
		}

		Exception::~Exception()
		{
			if(buffer_)
			{
				delete buffer_;
				buffer_ = NULL;
			}
		}

		size_t Exception::getLength(const char * fmt, va_list args) const
		{
			#ifdef _WIN32
				return ::_vscprintf(fmt, args);
			#else /* !_WIN32 */
				/* How can i calculate this on a Posix System? */
				return 4096;
			#endif
		}

	}
}

