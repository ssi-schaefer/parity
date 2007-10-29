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

#ifndef __PCRT_UTIME_H__
#define __PCRT_UTIME_H__

#include "internal/pcrt.h"

#pragma push_macro("__STDC__")
#  ifdef __STDC__
#    undef __STDC__
#  endif
#  pragma push_macro("utime")
#    undef utime
#    define utime __crt_invalid_utime
     // WARNING: the big 'U' is intentionally, because otherwise
     // we would include sys/__crt_invalid_utime.h ;o)
#    include RUNTIME_INC(sys/Utime.h)
#  pragma pop_macro("utime")
#pragma pop_macro("__STDC__")

//
// utime function needs path name conversion
//
PCRT_BEGIN_C

#pragma push_macro("utime")
#undef utime

static PCRT_INLINE int utime	(const char* f, const struct utimebuf* p)		{ return _utime(PCRT_CONV(f), p); }

#pragma pop_macro("utime")

PCRT_END_C

#endif

