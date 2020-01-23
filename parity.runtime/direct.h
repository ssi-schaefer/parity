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

#undef  chdir
#undef _chdir
#undef  mkdir
#undef _mkdir
#undef  rmdir
#undef _rmdir

#pragma push_macro("__STDC__")
#  undef __STDC__
#  include UCRT_INC(Direct.h)
#pragma pop_macro("__STDC__")

PCRT_BEGIN_C

static PCRT_INLINE int pcrt_chdir(const char* p)
{
  return _chdir(PCRT_CONV(p));
}

static PCRT_INLINE int pcrt_mkdir(const char* p, ... /* ignored */)
{
  return _mkdir(PCRT_CONV(p));
}

static PCRT_INLINE int pcrt_rmdir(const char* p)
{
  return _rmdir(PCRT_CONV(p));
}

PCRT_END_C

#define  chdir pcrt_chdir
#define _chdir pcrt_chdir
#define  mkdir pcrt_mkdir
#define _mkdir pcrt_mkdir
#define  rmdir pcrt_rmdir
#define _rmdir pcrt_rmdir

#endif

