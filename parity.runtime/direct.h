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

#ifndef __PCRT_DIRECT_H__
#define __PCRT_DIRECT_H__

#include "internal/pcrt.h"

#pragma push_macro("__STDC__")
#  ifdef __STDC__
#    undef __STDC__
#  endif
#  pragma push_macro("chdir")
#  pragma push_macro("mkdir")
#  pragma push_macro("rmdir")
#    define chdir __crt_invalid_chdir
#    define mkdir __crt_invalid_mkdir
#    define rmdir __crt_invalid_rmdir
#    include RUNTIME_INC(Direct.h)
#  pragma pop_macro("chdir")
#  pragma pop_macro("mkdir")
#  pragma pop_macro("rmdir")
#pragma pop_macro("__STDC__")

PCRT_BEGIN_C

#pragma push_macro("chdir")
#pragma push_macro("mkdir")
#pragma push_macro("rmdir")

#undef chdir
#undef mkdir
#undef rmdir

static PCRT_INLINE int chdir(const char* p) { return _chdir(PCRT_CONV(p)); }
static PCRT_INLINE int mkdir(const char* p, ... /* ignored */) { return _mkdir(PCRT_CONV(p)); }
static PCRT_INLINE int rmdir(const char* p) { return _rmdir(PCRT_CONV(p)); }

#pragma pop_macro("chdir")
#pragma pop_macro("mkdir")
#pragma pop_macro("rmdir")

PCRT_END_C

#endif

