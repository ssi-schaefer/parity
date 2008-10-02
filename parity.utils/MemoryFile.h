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

#ifndef __MEMORYFILE_H__
#define __MEMORYFILE_H__

#include "GenericFile.h"
#include "MappedFile.h"
#include "Path.h"

namespace parity
{
	namespace utils
	{
		class MemoryFile : public GenericFile {
		public:
			MemoryFile();
			MemoryFile(size_t sz);
			MemoryFile(void * buffer, size_t sz);
			MemoryFile(const MappedFile& map);
			virtual ~MemoryFile();

			void close();
			bool save(const Path& dest);
			bool resize(size_t sz);
			size_t getSize() { return size_; }

			bool append(const void* data, size_t len);
			void clear();

			bool isInRange(void* ptr) const;

			void* getBase() const { return base_; }
			void* getTop() const { return (char*)base_ + size_; }

			bool isReadable() const { return true; }
			bool isWriteable() const { return true; }
		private:
			MemoryFile(const MemoryFile&);
			MemoryFile& operator=(const MemoryFile&);

			bool allocated_;
			size_t allocated_size_;
			void* base_;
			size_t size_;
		};
	}
}

#endif

