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
   // WARNING: The 'S' in stat.h is uppercase
   // intentionally to avoid inclusion of __crt_invalid_stat.h
#  include UCRT_INC(sys/Stat.h)
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

//
// Cannot typedef 'struct stat' to 'struct pcrt_stat', so
// we have to declare a fresh 'struct pcrt_stat' instead:
// Keep synchronized with MSVC header!
//
#ifndef _WIN64
struct pcrt_stat32 {
        _dev_t     st_dev;
        _ino_t     st_ino;
        unsigned short st_mode;
        short      st_nlink;
        short      st_uid;
        short      st_gid;
        _dev_t     st_rdev;
        long       st_size;
#ifdef _USE_32BIT_TIME_T
        long       st_atime;
        long       st_mtime;
        long       st_ctime;
#else // !_USE_32BIT_TIME_T
        long long  st_atime;
        long long  st_mtime;
        long long  st_ctime;
#endif // !_USE_32BIT_TIME_T
};

static PCRT_INLINE int pcrt_stat32(const char * p, struct pcrt_stat32* b)
{
#ifdef _USE_32BIT_TIME_T
	typedef char __static_assert_t[(sizeof(struct pcrt_stat32) == sizeof(struct _stat32) != 0)];
	return _stat32(PCRT_CONV(p), (struct _stat32*)b);
#else // !_USE_32BIT_TIME_T
	typedef char __static_assert_t[(sizeof(struct pcrt_stat32) == sizeof(struct _stat64i32) != 0)];
	return _stat64i32(PCRT_CONV(p), (struct _stat64i32*)b);
#endif // !_USE_32BIT_TIME_T
}
static PCRT_INLINE int pcrt_fstat32(int f, struct pcrt_stat32* b)
{
#ifdef _USE_32BIT_TIME_T
	typedef char __static_assert_t[(sizeof(struct pcrt_stat32) == sizeof(struct _stat32) != 0)];
	return _fstat32(f, (struct _stat32*)b);
#else // !_USE_32BIT_TIME_T
	typedef char __static_assert_t[(sizeof(struct pcrt_stat32) == sizeof(struct _stat64i32) != 0)];
	return _fstat64i32(f, (struct _stat64i32*)b);
#endif // !_USE_32BIT_TIME_T
}
#endif // !_WIN64

struct pcrt_stat64 {
        _dev_t     st_dev;
        _ino_t     st_ino;
        unsigned short st_mode;
        short      st_nlink;
        short      st_uid;
        short      st_gid;
        _dev_t     st_rdev;
        long long  st_size;
#if !defined(_WIN64) && defined(_USE_32BIT_TIME_T)
        long       st_atime;
        long       st_mtime;
        long       st_ctime;
#else // !_USE_32BIT_TIME_T
        long long  st_atime;
        long long  st_mtime;
        long long  st_ctime;
#endif // !_USE_32BIT_TIME_T
};

static PCRT_INLINE int pcrt_stat64(const char * p, struct pcrt_stat64* b)
{
#if !defined(_WIN64) && defined(_USE_32BIT_TIME_T)
	typedef char __static_assert_t[(sizeof(struct pcrt_stat64) == sizeof(struct _stat32i64) != 0)];
	return _stat32i64(PCRT_CONV(p), (struct _stat32i64*)b);
#else // !_USE_32BIT_TIME_T
	typedef char __static_assert_t[(sizeof(struct pcrt_stat64) == sizeof(struct _stat64) != 0)];
	return _stat64(PCRT_CONV(p), (struct _stat64*)b);
#endif // !_USE_32BIT_TIME_T
}
static PCRT_INLINE int pcrt_fstat64(int f, struct pcrt_stat64* b)
{
#if !defined(_WIN64) && defined(_USE_32BIT_TIME_T)
	typedef char __static_assert_t[(sizeof(struct pcrt_stat64) == sizeof(struct _stat32i64) != 0)];
	return _fstat32i64(f, (struct _stat32i64*)b);
#else // !_USE_32BIT_TIME_T
	typedef char __static_assert_t[(sizeof(struct pcrt_stat64) == sizeof(struct _stat64) != 0)];
	return _fstat64(f, (struct _stat64*)b);
#endif // !_USE_32BIT_TIME_T
}

#undef   stat
#undef  _stat
#undef   stat64
#undef  _stat64
#undef  _stati64
#undef  fstat
#undef _fstat
#undef  fstat64
#undef _fstat64
#undef _fstati64

#ifdef _WIN64
# define   stat    pcrt_stat64
# define  _stat    pcrt_stat64
# define   stat64  pcrt_stat64
# define  _stat64  pcrt_stat64
# define  _stati64 pcrt_stat64
# define  fstat    pcrt_fstat64
# define _fstat    pcrt_fstat64
# define  fstat64  pcrt_fstat64
# define _fstat64  pcrt_fstat64
# define _fstati64 pcrt_fstat64
#else
# define   stat    pcrt_stat32
# define  _stat    pcrt_stat32
# define   stat64  pcrt_stat64
# define  _stat64  pcrt_stat64
# define  _stati64 pcrt_stat64
# define  fstat    pcrt_fstat32
# define _fstat    pcrt_fstat32
# define  fstat64  pcrt_fstat64
# define _fstat64  pcrt_fstat64
# define _fstati64 pcrt_fstat64
#endif

PCRT_END_C

#endif

