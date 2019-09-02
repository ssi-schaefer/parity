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


#ifdef __PCRT_INTERNAL_IO_H_NEED_PREAMBLE
#undef __PCRT_INTERNAL_IO_H_NEED_PREAMBLE

#define __PCRT_INTERNAL_COREIO_H_NEED_PREAMBLE
#include "pcrt-coreio.h"

#undef  access
#undef _access
#undef _access_s
#undef  chmod
#undef _chmod
#undef  creat
#undef _creat
#undef  open
#undef _open
#undef _sopen
#undef _sopen_s

#endif // __PCRT_INTERNAL_IO_H_NEED_PREAMBLE


#ifdef __PCRT_INTERNAL_IO_H_NEED_POSTAMBLE
#undef __PCRT_INTERNAL_IO_H_NEED_POSTAMBLE

#define __PCRT_INTERNAL_COREIO_H_NEED_POSTAMBLE
#include "pcrt-coreio.h"

#ifndef __PCRT_INTERNAL_IO_H_IMPL
#define __PCRT_INTERNAL_IO_H_IMPL
//
// redefine io.h functions which work with files to be able
// of parsing non-native paths.
//
PCRT_BEGIN_C

static PCRT_INLINE int pcrt_access(const char* f, int m)
{
  return _access(PCRT_CONV(f), m);
}

static PCRT_INLINE errno_t pcrt_access_s(const char* f, int m)
{
  return _access_s(PCRT_CONV(f), m);
}

static PCRT_INLINE int pcrt_chmod(const char* f, int m)
{
  return _chmod(PCRT_CONV(f), m);
}

static PCRT_INLINE int pcrt_creat(const char* f, int m)
{
  return _creat(PCRT_CONV(f), m);
}

static PCRT_INLINE int pcrt_open(const char* f, int fl, ...)
{
    int ret;
    va_list args;
    va_start(args, fl);
    ret = _open(PCRT_CONV(f), fl, va_arg(args, int));
    va_end(args);
    return ret;
}

static PCRT_INLINE int pcrt_sopen(const char *filename, int oflag, int shflag, ...)
{
    int ret;
    va_list args;
    va_start(args, shflag);
    ret = _sopen(PCRT_CONV(filename), oflag, shflag, va_arg(args, int));
    va_end(args);
    return ret;
}

static PCRT_INLINE errno_t pcrt_sopen_s(int *pfh, const char *filename, int oflag, int shflag, int pmode)
{
  return _sopen_s(pfh, PCRT_CONV(filename), oflag, shflag, pmode);
}

PCRT_END_C

#endif // __PCRT_INTERNAL_IO_H_IMPL


#define  access   pcrt_access
#define _access   pcrt_access
#define _access_s pcrt_access_s
#define  chmod    pcrt_chmod
#define _chmod    pcrt_chmod
#define  creat    pcrt_creat
#define _creat    pcrt_creat
#define  open     pcrt_open
#define _open     pcrt_open
#define _sopen    pcrt_sopen
#define _sopen_s  pcrt_sopen_s

#endif // __PCRT_INTERNAL_IO_H_NEED_POSTAMBLE
