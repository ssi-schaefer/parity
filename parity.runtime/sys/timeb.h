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

#ifndef __PCRT_SYS_TIMEB_H__
#define __PCRT_SYS_TIMEB_H__

#include "../internal/pcrt.h"

#pragma push_macro("__STDC__")
#  ifdef __STDC__
#    undef __STDC__
#  endif
#  include RUNTIME_INC(sys/Timeb.h)
#pragma pop_macro("__STDC__")

//
// FIXXME: Hack to handle "#define ftime _ftime" from folks who
// think to know how to handle windows. this should be fixed elsewhere!
//
#undef _ftime
#undef _timeb

#define _ftime ftime
#define _timeb timeb

#endif

