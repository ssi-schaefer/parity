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

#include "sys/times.h"

#include <windows.h>

#define FACTOR (0x19db1ded53e8000LL)
#define NSPERSEC 10000000LL

static clock_t __stdcall __to_clock_t (FILETIME * src, int flag)
{
  unsigned long long total = ((unsigned long long) src->dwHighDateTime << 32) + ((unsigned)src->dwLowDateTime);

  /* Convert into clock ticks - the total is in 10ths of a usec. */
  if (flag)
    total -= FACTOR;

  total /= (unsigned long long) (NSPERSEC / CLK_TCK);
  return (clock_t)total;
}

clock_t times (struct tms * buf)
{
	FILETIME creation_time, exit_time, kernel_time, user_time;
	HANDLE hMainProc = GetCurrentProcess();

	DWORD ticks = GetTickCount ();

	/* Ticks is in milliseconds, convert to our ticks. Use long long to prevent overflow. */
	clock_t tc = (clock_t) ((long long) ticks * CLK_TCK / 1000);

	GetProcessTimes (hMainProc, &creation_time, &exit_time, &kernel_time, &user_time);

	buf->tms_stime = __to_clock_t (&kernel_time, 0);
	buf->tms_utime = __to_clock_t (&user_time, 0);
	buf->tms_cstime = __to_clock_t (&kernel_time, 1);
	buf->tms_cutime = __to_clock_t (&user_time, 1);

	return tc;
}

