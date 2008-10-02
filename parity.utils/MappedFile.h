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

#ifndef __MAPPEDFILE_H__
#define __MAPPEDFILE_H__

#ifdef _WIN32
#  include <windows.h>
#endif

#include "Path.h"
#include "GenericFile.h"

namespace parity
{
	namespace utils
	{

		class MappedFile : public GenericFile {
		public:
			MappedFile(const Path& path, OpenMode mode);
			virtual ~MappedFile();

			bool isInRange(void* ptr) const;
			void close();

			void* getBase() const { return base_; }
			void* getTop() const { return top_; }

			bool isReadable() const { return (mode_ == ModeRead || mode_ == ModeBoth); }
			bool isWriteable() const { return (mode_ == ModeWrite || mode_ == ModeBoth); }

			const Path& getPath() const { return path_; }

		private:
			MappedFile(const MappedFile&);
			MappedFile& operator=(const MappedFile&);

			const Path path_;

			void* base_;
			void* top_;

			OpenMode mode_;

			#ifdef _WIN32
				HANDLE file_;
				HANDLE mapping_;
			#else /* !_WIN32 */
				struct stat info_;
				int file_;
			#endif
		};

	}
}

#endif

