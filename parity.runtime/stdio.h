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

#ifndef __PCRT_STDIO_H__
#define __PCRT_STDIO_H__

#include "internal/pcrt.h"

#pragma push_macro("_POSIX_")
#pragma push_macro("__STDC__")
#  if !defined(_POSIX_) && defined(__PARITY_GNU__)
#    define _POSIX_ 1
#  endif
#  ifdef __STDC__
#    undef __STDC__
#  endif
#  pragma push_macro("unlink")
#    define unlink __crt_invalid_unlink
#    include RUNTIME_INC(Stdio.h)
#  pragma pop_macro("unlink")
#pragma pop_macro("_POSIX_")
#pragma pop_macro("__STDC__")

#include RUNTIME_INC(Stdarg.h)

//
// redefine with path conversion attached. We simply hope
// that nobody is stupid enough to use those names as
// something else then a call to this functions.
//
#define fopen(f, m)				fopen(PCRT_CONV(f), m)
#define freopen(f, m, p)		freopen(PCRT_CONV(f), m, p)

PCRT_BEGIN_C

#pragma push_macro("snprintf")
#pragma push_macro("unlink")
#pragma push_macro("popen")
#pragma push_macro("pclose")
#pragma push_macro("tempnam")

#undef snprintf
#undef unlink
#undef popen
#undef pclose
#undef tempnam

extern int snprintf(char* b, size_t c, const char* fmt, ...);

static PCRT_INLINE int unlink(const char* f) { return _unlink(PCRT_CONV(f)); }
static PCRT_INLINE FILE* popen(const char* c, const char* m) { return _popen(c, m); }
static PCRT_INLINE int pclose(FILE* f) { return _pclose(f); }
static PCRT_INLINE char* tempnam(const char* d, const char* p) { return _tempnam(PCRT_CONV(d), p); }

#pragma pop_macro("snprintf")
#pragma pop_macro("unlink")
#pragma pop_macro("popen")
#pragma pop_macro("pclose")
#pragma pop_macro("tempnam")

PCRT_END_C

//
// TODO: Wide character versions?
//

#endif

