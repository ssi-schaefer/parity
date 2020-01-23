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

#define __PCRT_INTERNAL_STDIO_H_NEED_REAL
#include "stdio.h"

#if defined(_MSC_VER) && ((_MSC_VER - 0) < 1900)
// available since Windows 10 SDK (MSVC 14.0)
int snprintf(char* b, size_t c, const char* fmt, ...)
{
	int ret;
	va_list args;
	va_start(args, fmt);
	ret = _vsnprintf(b, c, fmt, args);
	va_end(args);
	return ret;
}
#endif

FILE* pcrt_fopen(const char *filename, const char *mode)
{
  return fopen(PCRT_CONV(filename), mode);
}

errno_t pcrt_fopen_s(FILE **pFile, const char *filename, const char *mode)
{
  return fopen_s(pFile, PCRT_CONV(filename), mode);
}

FILE* pcrt_freopen(const char *filename, const char *mode, FILE *stream)
{
  return freopen(PCRT_CONV(filename), mode, stream);
}

errno_t pcrt_freopen_s(FILE **pFile, const char *filename, const char *mode, FILE *stream)
{
  return freopen_s(pFile, PCRT_CONV(filename), mode, stream);
}

#if (_MSC_VER - 0) >= 1800
// _fsopen does have 3 arguments since MSVC 2012 only
FILE* pcrt_fsopen(const char *filename, const char *mode, int shflag)
{
  return _fsopen(PCRT_CONV(filename), mode, shflag);
}
#endif // _MSC_VER >= 1800

char * pcrt_tempnam(const char* dir, const char *prefix)
{
  return _tempnam(PCRT_CONV(dir), prefix);
}
