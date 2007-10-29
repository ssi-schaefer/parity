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

#define RUNTIME_INC(x) <../include/x>

#define PCRT_INLINE __inline

#ifdef __cplusplus
#  define PCRT_BEGIN_C extern "C" {
#  define PCRT_END_C } /* extern "C" */
#else
#  define PCRT_BEGIN_C
#  define PCRT_END_C
#endif

PCRT_BEGIN_C
//
// parity.runtime already has a dependency to parity.loader,
// so we can use it's path conversion too, to avoid reinventing
// the wheel, and having many functions to synchronize.
//
extern const char* LoaderConvertPathToNative(const char*);
#define PCRT_CONV(x) LoaderConvertPathToNative(x)

PCRT_END_C

#endif

