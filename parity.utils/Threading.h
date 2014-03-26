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
#include <config.h>

#ifdef _WIN32
#  include <windows.h>
#  define THREADINGAPI __stdcall
#else
#  define THREADINGAPI
#endif

#define POSIX_FORK 1
#define POSIX_THREADS 2
#define POSIX_NONE 3

//
// at least  on interix 3.5 pthread implementation is
// extremely unstable and produces various strange results:
//  * CTRL+C not working anymore.
//  * unpredictable "Memory Fault (core dumped)"'s
//  * slow as hell.
//
// another thing to consider with threads is shared memory, and
// the various singletons used in parity!
//
#if defined(linux) && defined(HAVE_PTHREAD)
#  define POSIX_THREADING_MODEL POSIX_NONE
#else
#  define POSIX_THREADING_MODEL POSIX_FORK
#endif

namespace parity
{
	namespace utils
	{
		typedef unsigned int (THREADINGAPI *ThreadingFunction)(void*);

		class Threading
		{
		public:
			Threading() : handles_() {}
			void synchronize();
			/*
			 * commented out since not required right now, see cpp file.
			 *
			void synchronize(long id);
			 */
			long run(ThreadingFunction method, void* data, bool allowDataSeparation);
		private:
			typedef std::map<long, void*> HandleMap;
			HandleMap handles_;	
			void waitForOneThread(HandleMap::iterator it);
		};
	}
}

#endif

