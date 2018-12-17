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

#ifndef __PCRT_STDLIB_H__
#define __PCRT_STDLIB_H__

#include "internal/pcrt.h"

#pragma push_macro("_POSIX_")
#pragma push_macro("__STDC__")
#  ifdef _POSIX_
#    undef _POSIX_
#  endif
#  ifdef __STDC__
#    undef __STDC__
#  endif
#  include UCRT_INC(Stdlib.h)
#pragma pop_macro("_POSIX_")
#pragma pop_macro("__STDC__")

#include "search.h"

PCRT_BEGIN_C

#pragma push_macro("realpath")
#undef realpath

extern int setenv(const char* name, const char* value, int overwrite);
extern int unsetenv(const char* name);

//
// PATH_MAX is 512 on windows and interix...
//
extern char* realpath(const char* p, char* r);

#pragma pop_macro("realpath")

PCRT_END_C

#endif

