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

#include "MemoryFile.h"
#include "Exception.h"

#include <fcntl.h>
#include <errno.h>

#ifdef _WIN32
#  include <io.h>
#endif

#define BUFFER_GROWSTEP			4096
#define BUFFER_ALLOCSIZE(s)		((((s)+BUFFER_GROWSTEP-1)/BUFFER_GROWSTEP)*BUFFER_GROWSTEP)

namespace parity
{
	namespace utils
	{
		MemoryFile::MemoryFile()
			: allocated_(true)
			, allocated_size_(0)
			, base_(0)
			, size_(0)
		{
		}

		MemoryFile::MemoryFile(size_t sz)
			: allocated_(true)
			, allocated_size_(BUFFER_ALLOCSIZE(sz + 1))
			, base_(0)
			, size_(sz)
		{
			base_ = malloc(allocated_size_);

			if(!base_)
				throw Exception("Cannot allocate buffer for MemoryFile!");
		}

		MemoryFile::MemoryFile(void * buffer, size_t sz)
			: allocated_(false)
			, allocated_size_(0)
			, base_(buffer)
			, size_(sz)
		{
			if(!base_)
				throw Exception("Invalid buffer for MemoryFile given!");
		}

		MemoryFile::MemoryFile(const MappedFile& map)
			: allocated_(true)
			, allocated_size_(0)
			, base_(0)
			, size_((reinterpret_cast<char*>(map.getTop()) - reinterpret_cast<char*>(map.getBase())) + 1)
		{
			allocated_size_ = BUFFER_ALLOCSIZE(size_);
			base_ = malloc(allocated_size_);

			if(!base_)
				throw Exception("Cannot allocate buffer for MemoryFile!");

			::memcpy(base_, map.getBase(), size_);
		}

		MemoryFile::~MemoryFile()
		{
			close();
		}

		void MemoryFile::close()
		{
			if(allocated_ && base_)
				free(base_);

			base_ = NULL;
			size_ = 0;
		}

		bool MemoryFile::save(const Path& dest)
		{
			#ifdef _WIN32
			#  define O_CREAT _O_CREAT
			#  define O_TRUNC _O_TRUNC
			#  define O_WRONLY _O_WRONLY
			#  define open _open
			#  define write _write
			#  define close _close
			#endif

			int fd = ::open(dest.get().c_str(), O_CREAT | O_TRUNC | O_WRONLY
				#ifdef _WIN32
					| _O_BINARY
				#endif
				);

			if(fd == -1)
				throw Exception("Cannot open %s: %s", dest.get().c_str(), strerror(errno));

			if(::write(fd, base_, static_cast<unsigned int>(size_)) == -1)
			{
				::close(fd);
				throw Exception("Cannot write %s: %s", dest.get().c_str(), strerror(errno));
			}

			::close(fd);

			dest.mode(0777);

			return true;

			#ifdef _WIN32
			#  undef open
			#  undef close
			#  undef write
			#endif
		}

		bool MemoryFile::resize(size_t sz)
		{
			if(!allocated_)
			{
				//
				// switch to allocated buffer
				//
				allocated_ = true;
				allocated_size_ = BUFFER_ALLOCSIZE(size_);
				void* buf = malloc(allocated_size_);
				::memcpy(buf, base_, size_);
				base_ = buf;
			}

			if(sz == size_)
				return true;

			size_t new_size = BUFFER_ALLOCSIZE(sz);

			if(new_size > allocated_size_) {
				allocated_size_ = new_size;
				base_ = realloc(base_, allocated_size_);
			}
			size_ = sz;

			if(!base_)
				throw Exception("Cannot resize buffer for MemoryFile!");

			return true;
		}

		bool MemoryFile::isInRange(void *ptr) const
		{
			if(ptr >= base_ && ptr <= (reinterpret_cast<char*>(base_) + size_))
				return true;
			else
				return false;
		}

		bool MemoryFile::append(const void* data, size_t len)
		{
			if(!resize(size_ + len))
				return false;

			::memcpy(reinterpret_cast<char*>(base_) + (size_ - len), data, len);

			return true;
		}

		void MemoryFile::clear()
		{
			close();
			allocated_ = true;
		}

	}
}

