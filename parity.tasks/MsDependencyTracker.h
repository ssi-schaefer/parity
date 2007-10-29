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

#ifndef __MSDEPENDENCYTRACKER_H__
#define __MSDEPENDENCYTRACKER_H__

#include <map>
#include <vector>

#include <Context.h>
#include <Path.h>
#include <MappedFile.h>

namespace parity
{
	namespace tasks
	{
		class MsDependencyTracker {
		public:
			MsDependencyTracker();
			void doWork();

		private:
			typedef std::map<utils::Path, bool> DependencyTrackMap;
			DependencyTrackMap dependencies_;
			DependencyTrackMap lock_;

			utils::Path lookupPath(const std::string& file);
			void getIncludes(const utils::Path& file, utils::PathVector& target);
			void trackFile(const utils::Path& file);
		};
	}
}

#endif

