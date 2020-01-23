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

#ifndef __PCRT_TIME_H__
#define __PCRT_TIME_H__

#include "internal/pcrt.h"

#pragma push_macro("_POSIX_")
#  if !defined(_POSIX_) && defined(__PARITY_GNU__)
#    define _POSIX_ 1
#  endif
#  include UCRT_INC(Time.h)
#pragma pop_macro("_POSIX_")

PCRT_BEGIN_C

extern size_t _pcrt_strftime_safe(char* buf, size_t size, const char* fmt, const struct tm* time);
#define strftime _pcrt_strftime_safe

#if !defined(_CRT_NO_TIME_T) && defined(TIME_UTC)

#define CLOCK_REALTIME TIME_UTC

typedef int clockid_t;

static PCRT_INLINE int clock_gettime(clockid_t clock_id, struct timespec *tp)
{
	return timespec_get(tp, clock_id);
}

#endif // TIME_UTC

PCRT_END_C

#endif

