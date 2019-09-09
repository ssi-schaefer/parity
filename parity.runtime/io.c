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

#define __PCRT_INTERNAL_IO_H_NEED_REAL 0
#include "io.h"

int pcrt_access(const char* f, int m)
{
  return _access(PCRT_CONV(f), m);
}

errno_t pcrt_access_s(const char* f, int m)
{
  return _access_s(PCRT_CONV(f), m);
}

int pcrt_chmod(const char* f, int m)
{
  return _chmod(PCRT_CONV(f), m);
}

int pcrt_creat(const char* f, int m)
{
  return _creat(PCRT_CONV(f), m);
}

int pcrt_open(const char* f, int fl, ...)
{
    int ret;
    va_list args;
    va_start(args, fl);
    ret = _open(f, fl, va_arg(args, int));
    va_end(args);
    return ret;
}

int pcrt_sopen(const char *filename, int oflag, int shflag, ...)
{
    int ret;
    va_list args;
    va_start(args, shflag);
    ret = _sopen(PCRT_CONV(filename), oflag, shflag, va_arg(args, int));
    va_end(args);
    return ret;
}

errno_t pcrt_sopen_s(int *pfh, const char *filename, int oflag, int shflag, int pmode)
{
  return _sopen_s(pfh, PCRT_CONV(filename), oflag, shflag, pmode);
}
