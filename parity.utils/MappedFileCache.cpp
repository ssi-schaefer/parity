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

#include "MappedFileCache.h"
#include "Exception.h"
#include "Log.h"

namespace parity
{
	namespace utils
	{
		static MappedFileCache gCache;

		MappedFileCache::MappedFileCache()
			: handles_()
		{
		}

		MappedFileCache::~MappedFileCache()
		{
			clear();
		}

		void MappedFileCache::clear()
		{
			for(CacheMap::iterator it = handles_.begin(); it != handles_.end(); ++it)
			{
				if(it->second)
				{
					delete it->second;
				}
			}

			handles_.clear();
		}

		MappedFileCache& MappedFileCache::getCache()
		{
			return gCache;
		}

		const MappedFile& MappedFileCache::get(const Path &pth) const
		{
			CacheMap::const_iterator pos = handles_.find(pth);

			if(pos != handles_.end())
				return *pos->second;

			throw Exception("file handle not cached for %s!", pth.get().c_str());
		}

		MappedFile& MappedFileCache::get(const Path &pth, OpenMode mode)
		{
			MappedFile*& ptr = handles_[pth];

			if(!ptr)
				ptr = new MappedFile(pth, mode);

			if((mode == ModeRead && !ptr->isReadable())
				|| (mode == ModeWrite && !ptr->isWriteable())
				|| (mode == ModeBoth && !(ptr->isReadable() && ptr->isWriteable())))
			{
				Log::warning("reopening %s, mode not matched for cached handle!", pth.get().c_str());
				delete ptr;
				ptr = new MappedFile(pth, mode);
			}

			return *ptr;
		}

		void MappedFileCache::close(const Path &pth)
		{
			CacheMap::iterator pos = handles_.find(pth);

			if(pos != handles_.end())
			{
				if(pos->second)
					delete pos->second;

				handles_.erase(pos);
			}
		}
	}
}

