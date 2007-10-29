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

#ifndef __PCRT_IO_H__
#define __PCRT_IO_H__

#include "internal/pcrt.h"

#pragma push_macro("__STDC__")
#  ifdef __STDC__
#    undef __STDC__
#  endif
#  pragma push_macro("open")
#  pragma push_macro("access")
#  pragma push_macro("chmod")
#  pragma push_macro("creat")
#  pragma push_macro("unlink")	// this is in stdio.h, but still we need it here if io.h is included first!
#    undef open
#    undef access
#    undef chmod
#    undef creat
#    undef unlink
#    define open __crt_invalid_open
#    define access __crt_invalid_access
#    define chmod __crt_invalid_chmod
#    define creat __crt_invalid_creat
#    define unlink __crt_invalid_unlink
#    include RUNTIME_INC(io.h)
#  pragma pop_macro("open")
#  pragma pop_macro("access")
#  pragma pop_macro("chmod")
#  pragma pop_macro("creat")
#  pragma pop_macro("unlink")
#pragma pop_macro("__STDC__")

#include RUNTIME_INC(stdarg.h)

//
// redefine io.h functions which work with files to be able
// of parsing non-native paths.
//
PCRT_BEGIN_C

#pragma push_macro("open")
#pragma push_macro("access")
#pragma push_macro("chmod")
#pragma push_macro("creat")

#undef open
#undef access
#undef chmod
#undef creat

static PCRT_INLINE int open(const char* f, int fl, ...) {
	int ret;
	va_list args;
	va_start(args, fl);

	ret = _open(PCRT_CONV(f), fl, va_arg(args, int));

	va_end(args);
	return ret;
}

static PCRT_INLINE int access(const char* f, int m) { return _access(PCRT_CONV(f), m); }
static PCRT_INLINE int chmod(const char* f, int m) { return _chmod(PCRT_CONV(f), m); }
static PCRT_INLINE int creat(const char* f, int m) { return _creat(PCRT_CONV(f), m); }

#pragma pop_macro("open")
#pragma pop_macro("access")
#pragma pop_macro("chmod")
#pragma pop_macro("creat")

PCRT_END_C

#endif

