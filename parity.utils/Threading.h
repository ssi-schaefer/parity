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

#ifndef __THREADING_H__
#define __THREADING_H__

#include <map>

#ifdef _WIN32
#  include <windows.h>
#  define THREADINGAPI __stdcall
#else
#  define THREADINGAPI
#endif

namespace parity
{
	namespace utils
	{
		typedef unsigned int (THREADINGAPI *ThreadingFunction)(void*);

		class Threading
		{
		public:
			void synchronize();
			void synchronize(long id);
			long run(ThreadingFunction method, void* data, bool allowDataSeparation);
		private:
			typedef std::map<long, void*> HandleMap;
			HandleMap handles_;	
			void waitForOneThread(HandleMap::iterator it);
		};
	}
}

#endif

