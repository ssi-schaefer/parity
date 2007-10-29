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

#ifndef __MAPPEDFILECACHE_H__
#define __MAPPEDFILECACHE_H__

#include <map>

#include "Path.h"
#include "MappedFile.h"

namespace parity
{
	namespace utils
	{
		class MappedFileCache
		{
		public:
			MappedFileCache();
			~MappedFileCache();

			MappedFile& get(const Path& pth, OpenMode mode);
			const MappedFile& get(const Path& pth) const;
			void close(const Path& pth);
			void clear();

			bool isCached(const Path& pth) const { return (handles_.find(pth) != handles_.end()); }

			static MappedFileCache& getCache();

		private:

			typedef std::map<Path, MappedFile*> CacheMap;
			CacheMap handles_;
		};
	}
}

#endif

