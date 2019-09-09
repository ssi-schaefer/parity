/****************************************************************\
*                                                                *
* Copyright (C) 2007 by Markus Duft <markus.duft@ssi-schaefer.com>
* Copyright (C) 2019 by Michaek Haubenwallner <michael.haubenwallner@ssi-schaefer.com>
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


#ifdef __PCRT_INTERNAL_STDIO_H_NEED_PREWRAP

# define __PCRT_INTERNAL_COREIO_H_NEED_PREWRAP
# include "pcrt-coreio.h"

# ifndef __PCRT_INTERNAL_STDIO_H_NEED_REAL

#  pragma push_macro( "fopen")
#  pragma push_macro( "fopen_s")
#  pragma push_macro( "freopen")
#  pragma push_macro( "freopen_s")
#  pragma push_macro("_fsopen")
#  pragma push_macro( "tempnam")
#  pragma push_macro("_tempnam")

#  define  fopen     pcrt_no_fopen
#  define  fopen_s   pcrt_no_fopen_s
#  define  freopen   pcrt_no_freopen
#  define  freopen_s pcrt_no_freopen_s
#  define _fsopen    pcrt_no__fsopen
#  define  tempnam   pcrt_no_tempnam
#  define _tempnam   pcrt_no__tempnam

# endif // !__PCRT_INTERNAL_STDIO_H_NEED_REAL
# undef     __PCRT_INTERNAL_STDIO_H_NEED_PREWRAP
#endif //   __PCRT_INTERNAL_STDIO_H_NEED_PREWRAP


#ifdef __PCRT_INTERNAL_STDIO_H_NEED_POSTWRAP

# define __PCRT_INTERNAL_COREIO_H_NEED_POSTWRAP
# include "pcrt-coreio.h"

PCRT_BEGIN_C

extern FILE*   pcrt_fopen(const char *filename, const char *mode);
extern errno_t pcrt_fopen_s(FILE **pFile, const char *filename, const char *mode);
extern FILE*   pcrt_freopen(const char *filename, const char *mode, FILE *stream);
extern errno_t pcrt_freopen_s(FILE **pFile, const char *filename, const char *mode, FILE *stream);
#if (_MSC_VER - 0) >= 1800
// _fsopen does have 3 arguments since MSVC 2012 only
extern FILE* pcrt_fsopen(const char *filename, const char *mode, int shflag);
#endif
extern char *  pcrt_tempnam(const char* dir, const char *prefix);

PCRT_END_C

# ifndef __PCRT_INTERNAL_STDIO_H_NEED_REAL

#  undef  fopen
#  undef  fopen_s
#  undef  freopen
#  undef  freopen_s
#  undef _fsopen
#  undef  tempnam
#  undef _tempnam

#  pragma pop_macro( "fopen")
#  pragma pop_macro( "fopen_s")
#  pragma pop_macro( "freopen")
#  pragma pop_macro( "freopen_s")
#  pragma pop_macro("_fsopen")
#  pragma pop_macro( "tempnam")
#  pragma pop_macro("_tempnam")

#  ifndef  __PCRT_INTERNAL_STDIO_H
#   define __PCRT_INTERNAL_STDIO_H
//
// redefine stdio.h functions which work with files to be able
// of parsing non-native paths.
//
PCRT_BEGIN_C

static PCRT_INLINE FILE* fopen(const char *filename, const char *mode)
{
  return pcrt_fopen(filename, mode);
}

static PCRT_INLINE errno_t fopen_s(FILE **pFile, const char *filename, const char *mode)
{
  return pcrt_fopen_s(pFile, filename, mode);
}

static PCRT_INLINE FILE* freopen(const char *filename, const char *mode, FILE *stream)
{
  return pcrt_freopen(filename, mode, stream);
}

static PCRT_INLINE errno_t freopen_s(FILE **pFile, const char *filename, const char *mode, FILE *stream)
{
  return pcrt_freopen_s(pFile, filename, mode, stream);
}

#if (_MSC_VER - 0) >= 1800
// _fsopen does have 3 arguments since MSVC 2012 only
static PCRT_INLINE FILE* _fsopen(const char *filename, const char *mode, int shflag)
{
  return pcrt_fsopen(filename, mode, shflag);
}
#endif // _MSC_VER >= 1800

static PCRT_INLINE char * tempnam(const char* dir, const char *prefix)
{
  return pcrt_tempnam(dir, prefix);
}

static PCRT_INLINE char * _tempnam(const char* dir, const char *prefix)
{
  return pcrt_tempnam(dir, prefix);
}

PCRT_END_C

#  endif // __PCRT_INTERNAL_STDIO_H

#  endif // !__PCRT_INTERNAL_STDIO_H_NEED_REAL
# undef      __PCRT_INTERNAL_STDIO_H_NEED_POSTWRAP
#endif //    __PCRT_INTERNAL_STDIO_H_NEED_POSTWRAP
