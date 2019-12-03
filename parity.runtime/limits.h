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

#ifndef __PCRT_LIMITS_H__
#define __PCRT_LIMITS_H__

#include "internal/pcrt.h"

#pragma push_macro("_POSIX_")
#  if !defined(_POSIX_) && defined(__PARITY_GNU__)
#    define _POSIX_ 1
#  endif
#  include RUNTIME_INC(Limits.h)

#if defined(_POSIX_) && (_MSC_VER - 0 >= 1800)
   /* gone since MSVC 2013 */
#  define _POSIX_PATH_MAX     255
#  ifndef PATH_MAX
#    define PATH_MAX          512
#  endif
#endif

#pragma pop_macro("_POSIX_")

#endif

