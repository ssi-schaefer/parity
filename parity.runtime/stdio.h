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

#define __PCRT_INTERNAL_STDIO_H_NEED_PREAMBLE
#include "internal/pcrt-stdio.h"

#undef fopen
#undef fopen_s
#undef freopen
#undef freopen_s
#undef _fsopen

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

#define __PCRT_INTERNAL_STDIO_H_NEED_POSTAMBLE
#include "internal/pcrt-stdio.h"


PCRT_BEGIN_C

static PCRT_INLINE FILE* pcrt_fopen(const char *filename, const char *mode)
{
  return fopen(PCRT_CONV(filename), mode);
}

static PCRT_INLINE errno_t pcrt_fopen_s(FILE **pFile, const char *filename, const char *mode)
{
  return fopen_s(pFile, PCRT_CONV(filename), mode);
}

static PCRT_INLINE FILE* pcrt_freopen(const char *filename, const char *mode, FILE *stream)
{
  return freopen(PCRT_CONV(filename), mode, stream);
}

static PCRT_INLINE errno_t pcrt_freopen_s(FILE **pFile, const char *filename, const char *mode, FILE *stream)
{
  return freopen_s(pFile, PCRT_CONV(filename), mode, stream);
}

#if (_MSC_VER - 0) >= 1800
// _fsopen does have 3 arguments since MSVC 2012 only
static PCRT_INLINE FILE* pcrt_fsopen(const char *filename, const char *mode, int shflag)
{
  return _fsopen(PCRT_CONV(filename), mode, shflag);
}
#endif // _MSC_VER >= 1800

#pragma push_macro("snprintf")
#pragma push_macro("popen")
#pragma push_macro("pclose")
#pragma push_macro("tempnam")
#pragma push_macro("fseeko")
#pragma push_macro("ftello")

#undef snprintf
#undef popen
#undef pclose
#undef tempnam

extern int snprintf(char* b, size_t c, const char* fmt, ...);

static PCRT_INLINE FILE* popen(const char* c, const char* m) { return _popen(c, m); }
static PCRT_INLINE int pclose(FILE* f) { return _pclose(f); }
static PCRT_INLINE char* tempnam(const char* d, const char* p) { return _tempnam(PCRT_CONV(d), p); }

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

#pragma pop_macro("snprintf")
#pragma pop_macro("popen")
#pragma pop_macro("pclose")
#pragma pop_macro("tempnam")
#pragma pop_macro("fseeko")
#pragma pop_macro("ftello")

PCRT_END_C

#define fopen     pcrt_fopen
#define fopen_s   pcrt_fopen_s
#define freopen   pcrt_freopen
#define freopen_s pcrt_freopen_s
#if (_MSC_VER - 0) >= 1800
# define _fsopen  pcrt_fsopen
#endif // _MSC_VER >= 1800

//
// TODO: Wide character versions?
//

#endif

