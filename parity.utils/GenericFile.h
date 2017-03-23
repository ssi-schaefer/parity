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

#ifndef __GENERICFILE_H__
#define __GENERICFILE_H__

#ifndef _WIN32
#  include <unistd.h>
#else
#  include <windows.h>
#endif

namespace parity
{
	namespace utils
	{
		enum OpenMode{
			ModeRead,
			ModeWrite,
			ModeBoth
		};

		class GenericFile {
		public:
			virtual ~GenericFile() {}
			virtual void close() = 0;

			virtual void* getBase() const = 0;
			virtual void* getTop() const = 0;

			virtual size_t getSize() { return reinterpret_cast<char *>(getTop()) - reinterpret_cast<char *>(getBase()); }

			virtual bool isInRange(void* ptr) const = 0;

			virtual bool isReadable() const = 0;
			virtual bool isWriteable() const = 0;
		};
	}
}

#endif

