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
#include <windows.h>
#include <stdio.h>

//
// Describes the symbol that is nearest to
// a given address...
//
typedef struct {
	void* addr;
	char* name;
} syminfo_t;

//
// Describes a loaded module. WARNING: dont'
// _ever_ do a FreeLibrary on the HMODULE!
// it is allocated without reference counting,
// so closing it will most likely crash the
// program!
//
typedef struct {
	HMODULE module;
	char name[MAX_PATH];
} modinfo_t;

//
// Describes a stack frame. this is a single linked list,
// that can be traversed from the top-most to the bottom-most
// stack frame.
//
typedef struct _stackframe_t {
	void* eip;
	void* ebp;
	void* ret;
	size_t size;
	syminfo_t sym;

	struct _stackframe_t* next;
} stackframe_t;

typedef enum {
	LookupInternal = 0,
	LookupDebugInfo = 1
} SymbolLookupType;

PCRT_BEGIN_C

extern int PcrtWaitForDebugger(int timeout);
extern int PcrtBreakIfDebugged();
extern int PcrtAttachDebugger();

extern void PcrtPrintStackTrace(FILE* stream, stackframe_t* stack);
extern stackframe_t* PcrtGetStackTrace();
extern stackframe_t* PcrtGetStackTraceFrom(void* _bp, void* _ip);
extern stackframe_t* PcrtDestroyStackTrace(stackframe_t* trace);
SymbolLookupType PcrtUseDebugSymbols();
extern syminfo_t PcrtGetNearestSymbol(void* addr, SymbolLookupType t);
extern modinfo_t PcrtGetContainingModule(void* addr);
extern void PcrtSetupExceptionHandling();
extern void PcrtWriteExceptionInformation(HANDLE hCore, struct _EXCEPTION_POINTERS* ex, int detailed);
extern LONG CALLBACK PcrtHandleException(struct _EXCEPTION_POINTERS* ex);

PCRT_END_C

#endif
