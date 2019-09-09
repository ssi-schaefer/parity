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

/* need off_t for fseeko/ftello */
#include "internal/pcrt-off_t.h"

#define __PCRT_INTERNAL_STDIO_H_NEED_PREWRAP
#include "internal/pcrt-stdio.h"

#pragma push_macro("_POSIX_")
#pragma push_macro("__STDC__")
#  if !defined(_POSIX_) && defined(__PARITY_GNU__)
#    define _POSIX_ 1
#  endif
#  undef __STDC__
#  include UCRT_INC(Stdio.h)
#pragma pop_macro("_POSIX_")
#pragma pop_macro("__STDC__")

#include RUNTIME_INC(Stdarg.h)

#define __PCRT_INTERNAL_STDIO_H_NEED_POSTWRAP 1
#include "internal/pcrt-stdio.h"


//
// additional functions not available from MSVC
//
PCRT_BEGIN_C

#if defined(_MSC_VER) && ((_MSC_VER - 0) < 1900)
// available since Windows 10 SDK (MSVC 14.0)
extern int snprintf(char* b, size_t c, const char* fmt, ...);
#endif

static PCRT_INLINE FILE* popen(const char* c, const char* m)
{
  return _popen(c, m);
}

static PCRT_INLINE int pclose(FILE* f)
{
  return _pclose(f);
}

static PCRT_INLINE int fseeko(FILE* f, off_t o, int w)
{
  if (sizeof(off_t) == 8)
	return _fseeki64(f, o, w);
  return fseek(f, o, w);
}

static PCRT_INLINE off_t ftello(FILE* f)
{
  if (sizeof(off_t) == 8)
	return _ftelli64(f);
  return ftell(f);
}

PCRT_END_C

//
// TODO: Wide character versions?
//

#endif // __PCRT_STDIO_H__
