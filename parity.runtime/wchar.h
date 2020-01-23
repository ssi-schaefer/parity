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

#ifndef __PCRT_WCHAR_H__
#define __PCRT_WCHAR_H__

#include "internal/pcrt.h"

#if (_MSC_VER - 0) < 1900
/* before Windows 10 SDK, off_t is defined in wchar.h as well */
# include "internal/pcrt-off_t.h"
#endif /* _MSC_VER < 1900 */

#pragma push_macro("_POSIX_")
#pragma push_macro("__STDC__")
#  if !defined(_POSIX_) && defined(__PARITY_GNU__)
#    define _POSIX_ 1
#  endif
#  undef __STDC__
#  include UCRT_INC(Wchar.h)
#pragma pop_macro("_POSIX_")
#pragma pop_macro("__STDC__")


#endif

