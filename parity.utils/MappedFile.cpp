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

#include <sys/stat.h>

#ifdef _WIN32
#  include <windows.h>
#else
#  include <fcntl.h>
#  include <sys/mman.h>
#endif

#include "MappedFile.h"
#include "Exception.h"
#include "Pointer.h"

namespace parity
{
	namespace utils
	{
		static char const* emptyFile = "";

		MappedFile::MappedFile(const Path& path, OpenMode mode)
			: path_(path), base_(0), top_(0), mode_(mode)
#ifdef _WIN32
			, file_(INVALID_HANDLE_VALUE), mapping_(INVALID_HANDLE_VALUE)
#else
			, info_(), file_(0)
#endif
		{
			#ifdef _WIN32
				DWORD dwAccess;
				DWORD dwShare = FILE_SHARE_READ;
				DWORD dwPageAccess;
				DWORD dwMapAccess;

				switch(mode)
				{
				case ModeRead:
					dwAccess = GENERIC_READ;
					dwPageAccess = PAGE_READONLY;
					dwMapAccess = FILE_MAP_READ;
					break;
				case ModeWrite:
					dwAccess = GENERIC_WRITE;
					dwPageAccess = PAGE_READWRITE;
					dwMapAccess = FILE_MAP_ALL_ACCESS;
					break;
				case ModeBoth:
					dwAccess = GENERIC_READ | GENERIC_WRITE;
					dwPageAccess = PAGE_READWRITE;
					dwMapAccess = FILE_MAP_ALL_ACCESS;
					break;
				}

				struct stat tmp;
				stat(path.get().c_str(), &tmp);

				if(tmp.st_size == 0)
				{
					base_ = (void*)emptyFile;
					top_ = MAKEPTR(void*, base_, 1);
					file_ = 0;
					mapping_ = 0;
					return;
				}

				file_ = CreateFile(path.get().c_str(), dwAccess, dwShare, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

				if(file_ == INVALID_HANDLE_VALUE)
					throw Exception("File handle invalid, cannot open file: %s", path_.get().c_str());

				mapping_ = CreateFileMapping(file_, NULL, dwPageAccess, 0, 0, NULL);
				if (mapping_ == 0)
				{
					CloseHandle(file_);
					throw Exception("Cannot create memory mapping on file: %s: %s", path_.get().c_str(), strerror(errno));
				}

				base_ = (void*)MapViewOfFile(mapping_, dwMapAccess, 0, 0, 0);
				if (base_ == 0)
				{
					CloseHandle(mapping_);
					CloseHandle(file_);
					throw Exception("Cannot create view on mapped file: %s", path_.get().c_str());
				}

				top_ = MAKEPTR(void*, base_, tmp.st_size);
			#else /* !_WIN32 */
				int dwMode = 0;
				int dwProt = 0;

				switch(mode)
				{
				case ModeRead:
					dwMode = O_RDONLY;
					dwProt = PROT_READ;
					break;
				case ModeWrite:
					dwMode = O_WRONLY;
					dwProt = PROT_WRITE;
					break;
				case ModeBoth:
					dwMode = O_RDWR;
					dwProt = PROT_WRITE | PROT_READ;
					break;
				}

				if(stat(path.get().c_str(), &info_) != 0)
					throw Exception("Cannot stat file %s!", path_.get().c_str());

				if(info_.st_size == 0)
				{
					base_ = (void*)emptyFile;
					top_ = MAKEPTR(void*, base_, 1);
					file_ = 0;
					return;
				}

				file_ = open(path.get().c_str(), dwMode);

				if(file_ == -1)
					throw Exception("File Descriptor invalid, cannot open file %s!", path_.get().c_str());

				base_ = mmap(NULL, info_.st_size, dwProt, MAP_SHARED, file_, 0);

				if(base_ == MAP_FAILED)
				{
					::close(file_);
					throw Exception("Cannot create memory mapping of file %s!", path_.get().c_str());
				}

				top_ = MAKEPTR(void*, base_, info_.st_size);
			#endif
		}

		MappedFile::~MappedFile()
		{
			close();
		}

		void MappedFile::close()
		{
			#ifdef _WIN32
				if(base_ && base_ != emptyFile)
					UnmapViewOfFile(base_);
				if(mapping_)
					CloseHandle(mapping_);
				if(file_)
					CloseHandle(file_);

				base_ = 0;
				mapping_ = 0;
				file_ = 0;
			#else /* !_WIN32 */
				if(base_ && base_ != emptyFile)
					munmap(base_, info_.st_size);
				if(file_)
					::close(file_);

				base_ = 0;
				file_ = 0;
			#endif
		}

		bool MappedFile::isInRange(void *ptr) const
		{
			if(ptr >= base_ && ptr <= top_)
				return true;
			else
				return false;
		}

	}
}

