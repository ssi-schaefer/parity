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

#include "Threading.h"
#include "Exception.h"
#include "Log.h"
#include "Timing.h"
#include "Statistics.h"

#include <errno.h>

#ifdef _WIN32
#  include <process.h>
#  include <streambuf>
#else
#  include <unistd.h>
#  include <pthread.h>
#  include <sys/wait.h>
#endif

namespace parity
{
	namespace utils
	{
		void Threading::waitForOneThread(HandleMap::iterator it)
		{
			#ifdef _WIN32
				DWORD exitStatus;
				Log::verbose("waiting for thread %d...\n", it->first);
				WaitForSingleObject(it->second, INFINITE);
				GetExitCodeThread(it->second, &exitStatus);
				CloseHandle(it->second);

				if(exitStatus != 0)
				{
					Log::error("Thread %d exited abnormally with code: %d!\n", it->first, exitStatus);
					exit(1);
				}
			#elif POSIX_THREADING_MODEL == POSIX_THREADS
				void* val;
				Log::verbose("joining posix thread %d...\n", it->first);

				if(pthread_join(it->first, &val) != 0) {
					Log::error("cannot join posix thread %d!\n", it->first);
					exit(1);
				}

				if((unsigned int)val != 0)
				{
					Log::error("Posix thread %d exited abnormally with code: %d!\n", it->first, val);
					exit(1);
				}
			#elif POSIX_THREADING_MODEL == POSIX_FORK
				int ret = 0;
				Log::verbose("waiting for forked child %d.\n", it->first);

				if(waitpid(it->first, &ret, WUNTRACED) != it->first)
				{
					Log::error("cannot wait for forked child %d: %s.\n", it->first, ::strerror(errno));
					exit(1);
				}

				if(WIFEXITED(ret) && WEXITSTATUS(ret) != 0)
				{
					Log::error("forked child %d failed with status %d.\n", it->first, WEXITSTATUS(ret));
					exit(1);
				} else if(!WIFEXITED(ret)) {
					Log::error("forked child %d did not terminate normally.\n", it->first);
					exit(1);
				}
			#endif
		}

		void Threading::synchronize()
		{
			for(HandleMap::iterator it = handles_.begin(); it != handles_.end(); ++it)
			{
				waitForOneThread(it);
			}

			handles_.clear();
		}

		void Threading::synchronize(long id)
		{
			HandleMap::iterator pos = handles_.find(id);
			if(pos != handles_.end())
			{
				waitForOneThread(pos);
			}
		}

		long Threading::run(ThreadingFunction method, void* data, bool allowDataSeparation)
		{
			long threadID;

			#ifdef _WIN32
				HANDLE handle = (HANDLE)_beginthreadex(0, 0, method, data, 0, (unsigned*)&threadID);

				if(!handle)
					throw Exception("cannot create thread: %s", ::strerror(errno));

				Log::verbose("created new thread with ID %d.\n", threadID);

				handles_[threadID] = (void*)handle;
			#elif POSIX_THREADING_MODEL == POSIX_THREADS
				if(pthread_create((pthread_t*)&threadID, 0, (void*(*)(void*))method, data) != 0)
					throw Exception("cannot create thread: %s", ::strerror(errno));

				Log::verbose("created posix thread with ID %d.\n", threadID);

				handles_[threadID] = 0;
			#elif POSIX_THREADING_MODEL == POSIX_FORK
				if(allowDataSeparation)
				{
					threadID = fork();

					switch(threadID) {
					case -1:
						throw Exception("cannot fork child: %s", ::strerror(errno));
					case 0:
						Timing::instance().forked();
						Statistics::instance().forked();
						exit(method(data));
					default:
						Log::verbose("created forked child with id %d.\n", threadID);
						handles_[threadID] = 0;
						break;
					}
				} else {
					Log::verbose("preventing data separation by running single threaded job!\n");
					method(data);
					return 1;
				}
			#elif POSIX_THREADING_MODEL == POSIX_NONE
				Log::verbose("running single threaded job!\n");
				method(data);
				return 1;
			#else
				throw Exception("don't know how to thread!");
			#endif
			return threadID;
		}
	}
}

