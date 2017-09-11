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

#ifndef __PCRT_PARITYCRT_H__
#define __PCRT_PARITYCRT_H__

//
// WARNING: use uppercase letters in include file names
// always to avoid clashes with possible defines (e.g.
// if malloc is defined to rpl_malloc, RUNTIME_INC will
// try to include <rpl_malloc.h> which doesnt exist
//
#define RUNTIME_INC(x) <../include/x>
#if ((_MSC_VER - 0) >= 1900)
// MSVC 14.0 (VS 2013) comes with Windows 10 SDK
# define UCRT_INC(x) <../ucrt/x>
#else
# define UCRT_INC(x) <../include/x>
#endif

#if ((_MSC_VER - 0) >= 1700)
// MSVC 11.0 (VS 2012) comes with Windows 8 SDK
# define SHARED_INC(x) <../shared/x>
#else
# define SHARED_INC(x) <../include/x>
#endif

#define PCRT_INLINE __inline

#ifdef __cplusplus
#  define PCRT_BEGIN_C extern "C" {
#  define PCRT_END_C } /* extern "C" */
#else
#  define PCRT_BEGIN_C
#  define PCRT_END_C
#endif

PCRT_BEGIN_C

extern const char* PcrtPathToNative(const char*);
#define PCRT_CONV(x) PcrtPathToNative(x)

extern int PcrtInit();

PCRT_END_C

#endif

