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


#ifdef __PCRT_INTERNAL_IO_H_NEED_PREWRAP

# define __PCRT_INTERNAL_COREIO_H_NEED_PREWRAP
# include "pcrt-coreio.h"

# ifndef __PCRT_INTERNAL_IO_H_NEED_REAL

#  pragma push_macro( "access")
#  pragma push_macro("_access")
#  pragma push_macro("_access_s")
#  pragma push_macro( "chmod")
#  pragma push_macro("_chmod")
#  pragma push_macro( "creat")
#  pragma push_macro("_creat")
#  pragma push_macro( "open")
#  pragma push_macro("_open")
#  pragma push_macro("_sopen")
#  pragma push_macro("_sopen_s")

#  undef  access
#  undef _access
#  undef _access_s
#  undef  chmod
#  undef _chmod
#  undef  creat
#  undef _creat
#  undef  open
#  undef _open
#  undef _sopen
#  undef _sopen_s

#  define  access   pcrt_no_access
#  define _access   pcrt_no__access
#  define _access_s pcrt_no__access_s
#  define  chmod    pcrt_no_chmod
#  define _chmod    pcrt_no__chmod
#  define  creat    pcrt_no_creat
#  define _creat    pcrt_no__creat
#  define  open     pcrt_no_open
#  define _open     pcrt_no__open
#  define _sopen    pcrt_no__sopen
#  define _sopen_s  pcrt_no__sopen_s

# endif // !__PCRT_INTERNAL_IO_H_NEED_REAL
# undef     __PCRT_INTERNAL_IO_H_NEED_PREWRAP
#endif //   __PCRT_INTERNAL_IO_H_NEED_PREWRAP


#ifdef __PCRT_INTERNAL_IO_H_NEED_POSTWRAP

# define __PCRT_INTERNAL_COREIO_H_NEED_POSTWRAP
# include "pcrt-coreio.h"

PCRT_BEGIN_C

extern int pcrt_access(const char* f, int m);
extern errno_t pcrt_access_s(const char* f, int m);
extern int pcrt_chmod(const char* f, int m);
extern int pcrt_creat(const char* f, int m);
extern int pcrt_open(const char* f, int fl, ...);
extern int pcrt_sopen(const char *filename, int oflag, int shflag, ...);
extern errno_t pcrt_sopen_s(int *pfh, const char *filename, int oflag, int shflag, int pmode);

PCRT_END_C

# ifndef __PCRT_INTERNAL_IO_H_NEED_REAL

#  undef  access
#  undef _access
#  undef _access_s
#  undef  chmod
#  undef _chmod
#  undef  creat
#  undef _creat
#  undef  open
#  undef _open
#  undef _sopen
#  undef _sopen_s

#  ifndef  __PCRT_INTERNAL_IO_H
#   define __PCRT_INTERNAL_IO_H
//
// redefine io.h functions which work with files to be able
// of parsing non-native paths.
//
PCRT_BEGIN_C

static PCRT_INLINE int access(const char* f, int m)
{
  return pcrt_access(f, m);
}

static PCRT_INLINE int _access(const char* f, int m)
{
  return pcrt_access(f, m);
}

static PCRT_INLINE errno_t _access_s(const char* f, int m)
{
  return pcrt_access_s(f, m);
}

static PCRT_INLINE int chmod(const char* f, int m)
{
  return pcrt_chmod(f, m);
}

static PCRT_INLINE int _chmod(const char* f, int m)
{
  return pcrt_chmod(f, m);
}

static PCRT_INLINE int creat(const char* f, int m)
{
  return pcrt_creat(f, m);
}

static PCRT_INLINE int _creat(const char* f, int m)
{
  return pcrt_creat(f, m);
}

static PCRT_INLINE int open(const char* f, int fl, ...)
{
    int ret;
    va_list args;
    va_start(args, fl);
    ret = pcrt_open(f, fl, va_arg(args, int));
    va_end(args);
    return ret;
}

static PCRT_INLINE int _open(const char* f, int fl, ...)
{
    int ret;
    va_list args;
    va_start(args, fl);
    ret = pcrt_open(f, fl, va_arg(args, int));
    va_end(args);
    return ret;
}

static PCRT_INLINE int _sopen(const char *f, int oflag, int shflag, ...)
{
    int ret;
    va_list args;
    va_start(args, shflag);
    ret = pcrt_sopen(f, oflag, shflag, va_arg(args, int));
    va_end(args);
    return ret;
}

static PCRT_INLINE errno_t _sopen_s(int *pfh, const char *f, int oflag, int shflag, int pmode)
{
  return pcrt_sopen_s(pfh, f, oflag, shflag, pmode);
}

PCRT_END_C

#  endif // __PCRT_INTERNAL_IO_H

#  pragma pop_macro( "access")
#  pragma pop_macro("_access")
#  pragma pop_macro("_access_s")
#  pragma pop_macro( "chmod")
#  pragma pop_macro("_chmod")
#  pragma pop_macro( "creat")
#  pragma pop_macro("_creat")
#  pragma pop_macro( "open")
#  pragma pop_macro("_open")
#  pragma pop_macro("_sopen")
#  pragma pop_macro("_sopen_s")

# endif // !__PCRT_INTERNAL_IO_H_NEED_REAL
# undef     __PCRT_INTERNAL_IO_H_NEED_POSTWRAP
#endif //   __PCRT_INTERNAL_IO_H_NEED_POSTWRAP
