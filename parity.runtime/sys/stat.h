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

#ifndef __PCRT_SYS_STAT_H__
#define __PCRT_SYS_STAT_H__

#include "../internal/pcrt.h"

#pragma push_macro("__STDC__")
#  ifdef __STDC__
#    undef __STDC__
#  endif
#  pragma push_macro("stat")
#  pragma push_macro("fstat")
#    undef stat
#    undef fstat
#    define stat __crt_invalid_stat
#    define fstat __crt_invalid_fstat
     // WARNING: The 'S' in stat.h is uppercase
     // intentionally to avoid inclusion of __crt_invalid_stat.h
#    include RUNTIME_INC(sys/Stat.h)
#  pragma pop_macro("stat")
#  pragma pop_macro("fstat")
#pragma pop_macro("__STDC__")

#define _S_PROT     00000777
#define S_IRWXU     00000700
#define S_IRUSR     00000400
#define S_IWUSR     00000200
#define S_IXUSR     00000100

#define S_IRWXG     00000700
#define S_IRGRP     00000400
#define S_IWGRP     00000200
#define S_IXGRP     00000100

#define S_IRWXO     00000700
#define S_IROTH     00000400
#define S_IWOTH     00000200
#define S_IXOTH     00000100

#define S_ISDIR(m) ( ((m) & S_IFMT) == S_IFDIR )
#define S_ISREG(m) ( ((m) & S_IFMT) == S_IFREG )
#define S_ISCHR(m) ( ((m) & S_IFMT) == S_IFCHR )

PCRT_BEGIN_C

#pragma push_macro("stat")
#pragma push_macro("fstat")
#undef stat
#undef fstat

//
// Taken from Microsoft's header!
//
struct stat {
	_dev_t     st_dev;
	_ino_t     st_ino;
	unsigned short st_mode;
	short      st_nlink;
	short      st_uid;
	short      st_gid;
	_dev_t     st_rdev;
	_off_t     st_size;
	time_t st_atime;
	time_t st_mtime;
	time_t st_ctime;
};

static PCRT_INLINE int stat(const char * p, struct stat* b) { return _stat(PCRT_CONV(p), (struct _stat*)b); }
static PCRT_INLINE int fstat(int f, struct stat* b) { return _fstat(f, (struct _stat*)b); }

#pragma pop_macro("stat")
#pragma pop_macro("fstat")

PCRT_END_C

#endif

