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

#ifndef __PCRT_DIAGNOSE_H__
#define __PCRT_DIAGNOSE_H__

#include "pcrt.h"

typedef struct _stackframe_t {
	void* eip;
	void* ebp;
	void* ret;
	char const * sym;

	struct _stackframe_t* next;
} stackframe_t;

PCRT_BEGIN_C

extern int PcrtWaitForDebugger(int timeout);
extern int PcrtBreakIfDebugged();
extern int PcrtAttachDebugger();

extern void PcrtPrintStackTrace();
extern stackframe_t* PcrtGetStackTrace();

PCRT_END_C

#endif
