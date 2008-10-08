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

#ifndef __PCRT_UNISTD_H__
#define __PCRT_UNISTD_H__

#include "internal/pcrt.h"

#include "io.h"
#include "stdio.h"
#include "stdlib.h"
#include "process.h"
#include "sys/types.h"
#include "direct.h"

#include RUNTIME_INC(Stdarg.h)

#define STDIN_FILENO	0
#define STDOUT_FILENO	1
#define STDERR_FILENO	2

//
// mode values for access function. X_OK is the same as F_OK on
// windows, since there is no separate execute permission.
//
#define F_OK 00
#define W_OK 02
#define R_OK 04
#define X_OK 00

PCRT_BEGIN_C

#pragma push_macro("sleep")
#pragma push_macro("usleep")

#undef sleep
#undef usleep

//
// TODO: The usleep implementation below is really bad, since
// it doesn't really sleep the number of usec's given, but in
// the worst case 1000 times longer (minnimum 1 ms)!
//

__declspec(dllimport) void __stdcall Sleep(unsigned long millis);

static PCRT_INLINE unsigned int sleep(unsigned int secs) { Sleep((unsigned long)secs * 1000); return 0; }
static PCRT_INLINE int usleep(int usec) { Sleep((unsigned long)usec / 1000); return 0; }

#pragma pop_macro("sleep")
#pragma pop_macro("usleep")

PCRT_END_C

#endif

