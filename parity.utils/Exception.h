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

#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__

#include <stdarg.h>
#include <stdexcept>

#include "FormatStrings.h"

namespace parity
{
	namespace utils
	{

		class Exception {
		public:
			Exception(const char*, ...) FORMAT_CHECK(2, 3);
			Exception(const Exception& rhs) : buffer_(rhs.buffer_) {}
			~Exception();

			const char* what() const { return buffer_; }
			Exception& operator=(const Exception&);
		private:
			char * buffer_;

			size_t getLength(const char*, va_list) const;
		};

	}
}

#endif

