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

#include <windows.h>
#include <stdio.h>

#include <dbghelp.h>

#include "internal/diagnose.h"
#include "internal/output.h"
#include "libgen.h"

extern void* _ReturnAddress();
#pragma intrinsic(_ReturnAddress)

int PcrtWaitForDebugger(int timeout) {
	fprintf(stderr, "Process %d waiting for Debugger to be attached (%d seconds): ", GetCurrentProcessId(), timeout);

	while(timeout > 0 && !IsDebuggerPresent()) {
		Sleep(1000);
		if(!(timeout % 5))
			fprintf(stderr, "%d", timeout);
		else
			fprintf(stderr, ".");

		fflush(stderr);

		timeout--;
	}

	if(!IsDebuggerPresent()) {
		fprintf(stderr, " fail (timeout)\n");
		return -1;
	}

	fprintf(stderr, " success\n");

	return 0;
}

int PcrtBreakIfDebugged() {
	if(!IsDebuggerPresent())
		return -1;

	DebugBreak();
	return 0;
}

int PcrtAttachDebugger() {
	//
	// Simply trigger a braekpoint, which will cause the program
	// to "crash" and a debugger to be attached if present.
	//
	if(IsDebuggerPresent())
		return 0;

	DebugBreak();
	
	if(!IsDebuggerPresent())
		return -1;

	return 0;
}

static void PcrtpPrintFormattedError(DWORD dw) 
{ 
    LPVOID lpMsgBuf;

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    fprintf(stderr, "ERROR %d: %s", dw, lpMsgBuf);

    LocalFree(lpMsgBuf);
}

void PcrtPrintStackTrace(FILE* stream, stackframe_t* stack)
{
	stackframe_t* walk = stack;
	unsigned int num = 0;

	if(!stack) {
		fprintf(stream, "cannot obtain stack trace!\n");
		return;
	}

#define ST_FW_ADDRESS 8
#define ST_FW_MODULE  20
#define ST_FW_NAME    45

	fprintf(stream, "   #  %-*s %*s!%s\n", ST_FW_ADDRESS, "Address", ST_FW_MODULE, "Module", "Function (frame size)");
	for(num = 0; num < (ST_FW_ADDRESS + ST_FW_MODULE + ST_FW_NAME); ++num)
		fprintf(stream, "-");
	fprintf(stream, "\n");

	//
	// skip the first frame, since this is the print function itself.
	//
	walk = walk->next;

	num = 0;
	while(1) {
		modinfo_t mod = PcrtGetContainingModule(walk->eip);

		if(walk->sym.addr) {
			
			fprintf(stream, " [%2d] %-*p %*s!%s(%u bytes)+0x%x\n", num++, ST_FW_ADDRESS, walk->eip, ST_FW_MODULE, basename(mod.name), walk->sym.name, walk->size, ((unsigned long)walk->eip - (unsigned long)walk->sym.addr));
		} else {
			fprintf(stream, " [%2d] %-*p %*s!%s(%u bytes)\n", num++, ST_FW_ADDRESS, walk->eip, ST_FW_MODULE, basename(mod.name), "???", walk->size);
		}

		if(!walk->next)
			break;

		walk = walk->next;
	}

	for(num = 0; num < (ST_FW_ADDRESS + ST_FW_MODULE + ST_FW_NAME); ++num)
		fprintf(stream, "-");
	fprintf(stream, "\n");
	stack = PcrtDestroyStackTrace(stack);
}

stackframe_t* PcrtGetStackTraceFrom(void** _bp, void* _ip)
{
	stackframe_t* trace = NULL;
	stackframe_t* last = NULL;

	while(_bp && _ip) {
		stackframe_t* frame = (stackframe_t*)malloc(sizeof(stackframe_t));

		if(!frame) {
			fprintf(stderr, "cannot allocate memory for stack trace!");
			return NULL;
		}		

		frame->eip = _ip;
		frame->ebp = _bp;
		frame->next= NULL;
		frame->size = 0;

		if(last) {
			frame->size = (unsigned long)frame->ebp - (unsigned long)last->ebp;
		}

		//
		// subtract from the size the room that the own ebp
		// and the call return address take up in the frame.
		//
		// left in the size if the accumulated size of:
		//  * local variables
		//  * parameters for the next frame (!)
		//    (the parameters for the call it is right now executing!)
		//
		frame->size -= (sizeof(void*)*2);

		_ip = _bp[1];

		frame->ret = _ip;
		frame->sym = PcrtGetNearestSymbol(frame->eip);

		if(!trace) {
			trace = frame;
		} else if(last) {
			last->next = frame;
		} else {
			fprintf(stderr, "internal error creating stack trace (missing nodes).");
			return NULL;
		}

		last = frame;
		_bp = (void**)_bp[0];
	}

	return trace;
}

stackframe_t* PcrtGetStackTrace()
{
	//
	// Our starting point is the frame above ourselves.
	//
	void* _ip = _ReturnAddress();
	void** _bp;
	

	__asm {
		mov _bp, ebp
	}

	_bp = (void**)_bp[0];

	return PcrtGetStackTraceFrom(_bp, _ip);
}

stackframe_t* PcrtDestroyStackTrace(stackframe_t* trace)
{
	stackframe_t* next;

	while(trace) {
		next = trace->next;
		free(trace);
		trace = next;
	}

	return NULL;
}

syminfo_t PcrtGetNearestSymbol(void* addr)
{
	syminfo_t info = { 0, 0 };
	syminfo_t* symtab;
	modinfo_t module = PcrtGetContainingModule(addr);

	if(!module.module) {
		fprintf(stderr, "cannot obtain module information for symbol lookup (%p)!\n", addr);
		return info;
	}
	
	//
	// keep this name in sync with the one in MsSymbolTableGenerator.
	//
	symtab = (syminfo_t*)GetProcAddress(module.module, "ParityGeneratedSymbolTable");

	if(!symtab) {
		//fprintf(stderr, "cannot find symbol table in %s (%p)\n", module.name, module.module);
		return info;
	}

	//
	// would it be better to copy and sort the table? i guess
	// not, since we have to iterate over the whole thing in
	// any case, and in this case only once. however if we would
	// cache the results, this may bring some performance benefits
	// if getting many, many stack traces (or sym infos).
	//
	while(symtab->addr && symtab->name) {
		if(((unsigned long)symtab->addr < (unsigned long)addr)
			&& ((unsigned long)symtab->addr > (unsigned long)info.addr))
		{
			info = *symtab;
		}

		++symtab;
	}

	return info;
}

modinfo_t PcrtGetContainingModule(void* addr)
{
	//
	// Here we get the module containing the given address.
	// This is done without incrementing the reference count
	// for that module, so we have to be carefull to not pass
	// the handle to the outside world or FreeLibrary.
	//
	modinfo_t info;
	if(!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)addr, &info.module)) {
		fprintf(stderr, "cannot find handle for module containing %p\n", addr);
		return info;
	}

	if(!GetModuleFileNameA(info.module, info.name, MAX_PATH)) {
		strcpy(info.name, "<unknown>");
	}

	return info;
}

static LONG CALLBACK PcrtHandleException(struct _EXCEPTION_POINTERS* ex) {
	HANDLE hCore;
	long num = 0;
	stackframe_t* trace;

	switch(ex->ExceptionRecord->ExceptionCode)
	{
	case EXCEPTION_ACCESS_VIOLATION:
	case EXCEPTION_IN_PAGE_ERROR:
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
	case EXCEPTION_ILLEGAL_INSTRUCTION:
	case EXCEPTION_STACK_OVERFLOW:
	case EXCEPTION_PRIV_INSTRUCTION:
	case EXCEPTION_NONCONTINUABLE_EXCEPTION:
	case EXCEPTION_DATATYPE_MISALIGNMENT:
	case EXCEPTION_FLT_DENORMAL_OPERAND:
	case EXCEPTION_FLT_DIVIDE_BY_ZERO:
	case EXCEPTION_FLT_INEXACT_RESULT:
	case EXCEPTION_FLT_INVALID_OPERATION:
	case EXCEPTION_FLT_OVERFLOW:
	case EXCEPTION_FLT_STACK_CHECK:
	case EXCEPTION_FLT_UNDERFLOW:
	case EXCEPTION_INT_DIVIDE_BY_ZERO:
	case EXCEPTION_INT_OVERFLOW:
	case EXCEPTION_INVALID_DISPOSITION:
		break;

	case EXCEPTION_BREAKPOINT:
	case EXCEPTION_SINGLE_STEP:
	default:
		/* those exceptions are not handled (includes C++ exceptions) */
		return EXCEPTION_CONTINUE_SEARCH;
	}

	PcrtOutPrint(GetStdHandle(STD_ERROR_HANDLE), "Exception %p at %p (core dumped)\n", ex->ExceptionRecord->ExceptionCode, ex->ExceptionRecord->ExceptionAddress);

	hCore = CreateFile("core", GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if(hCore == INVALID_HANDLE_VALUE) {
		PcrtOutPrint(GetStdHandle(STD_ERROR_HANDLE), "Error opening core file!\n");
		return EXCEPTION_CONTINUE_SEARCH;
	}

	//
	// cannot use the PcrtPrintStackTrace, since we should not rely
	// on the CRT working after an exception. the kernel functions
	// should still work i guess ;)
	//
	trace = PcrtGetStackTraceFrom((void*)ex->ContextRecord->Ebp, (void*)ex->ContextRecord->Eip);

	PcrtOutPrint(hCore, "Stack Layout at the time the exception occured:\n\n");

	PcrtOutPrint(hCore, "NOTE: The symbol names are guessed by finding the nearest\n");
	PcrtOutPrint(hCore, "      known symbol. this may be wrong, since private symbols\n");
	PcrtOutPrint(hCore, "      are not recognized.\n\n");

	PcrtOutPrint(hCore, "NOTE: Since the Exception handler sees all exceptions that\n");
	PcrtOutPrint(hCore, "      fly by, it may create a core file even though the\n");
	PcrtOutPrint(hCore, "      process can and will continue. so make sure the\n");
	PcrtOutPrint(hCore, "      process really terminated after writing the core file.\n\n");

	while(trace) {
		PcrtOutPrint(hCore, " [%d] %p %s(%d bytes)+0x%x\n", num++, trace->eip, (trace->sym.name ? trace->sym.name : "???"), trace->size, (trace->sym.addr ? ((unsigned long)trace->eip - (unsigned long)trace->sym.addr) : 0));

		if(!trace->next)
			break;

		trace = trace->next;
	}

	trace = PcrtDestroyStackTrace(trace);

	PcrtOutPrint(hCore, "\n");
	PcrtOutPrint(hCore, "CPU Context at the time the exception occured:\n");

	PcrtOutPrint(hCore, "  Flags : %p\n", ex->ContextRecord->ContextFlags);
	
	if(ex->ContextRecord->ContextFlags & CONTEXT_DEBUG_REGISTERS) {
		PcrtOutPrint(hCore, "  DR0   : %p,", ex->ContextRecord->Dr0);
		PcrtOutPrint(hCore, "  DR1   : %p,", ex->ContextRecord->Dr1);
		PcrtOutPrint(hCore, "  DR2   : %p\n", ex->ContextRecord->Dr2);
		PcrtOutPrint(hCore, "  DR3   : %p,", ex->ContextRecord->Dr3);
		PcrtOutPrint(hCore, "  DR6   : %p,", ex->ContextRecord->Dr6);
		PcrtOutPrint(hCore, "  DR7   : %p\n", ex->ContextRecord->Dr7);
	}
	
	if(ex->ContextRecord->ContextFlags & CONTEXT_SEGMENTS) {
		PcrtOutPrint(hCore, "  SegGS : %p,", ex->ContextRecord->SegGs);
		PcrtOutPrint(hCore, "  SegFS : %p,", ex->ContextRecord->SegFs);
		PcrtOutPrint(hCore, "  SegES : %p\n", ex->ContextRecord->SegEs);
		PcrtOutPrint(hCore, "  SegDS : %p\n", ex->ContextRecord->SegDs);
	}

	if(ex->ContextRecord->ContextFlags & CONTEXT_INTEGER) {
		PcrtOutPrint(hCore, "  EDI   : %p,", ex->ContextRecord->Edi);
		PcrtOutPrint(hCore, "  ESI   : %p,", ex->ContextRecord->Esi);
		PcrtOutPrint(hCore, "  EBX   : %p\n", ex->ContextRecord->Ebx);
		PcrtOutPrint(hCore, "  EDX   : %p,", ex->ContextRecord->Edx);
		PcrtOutPrint(hCore, "  ECX   : %p,", ex->ContextRecord->Ecx);
		PcrtOutPrint(hCore, "  EAX   : %p\n", ex->ContextRecord->Eax);
	}

	if(ex->ContextRecord->ContextFlags & CONTEXT_CONTROL) {
		PcrtOutPrint(hCore, "  EBP   : %p,", ex->ContextRecord->Ebp);
		PcrtOutPrint(hCore, "  EIP   : %p,", ex->ContextRecord->Eip);
		PcrtOutPrint(hCore, "  SegCS : %p\n", ex->ContextRecord->SegCs);
		PcrtOutPrint(hCore, "  EFlags: %p,", ex->ContextRecord->EFlags);
		PcrtOutPrint(hCore, "  ESP   : %p,", ex->ContextRecord->Esp);
		PcrtOutPrint(hCore, "  SegSS : %p\n", ex->ContextRecord->SegSs);
	}

	PcrtOutPrint(hCore, "\n");
	PcrtOutPrint(hCore, "General Exception Information:\n");

	PcrtOutPrint(hCore, "  Exception Code    : %p\n", ex->ExceptionRecord->ExceptionCode);
	PcrtOutPrint(hCore, "  Exception Address : %p\n", ex->ExceptionRecord->ExceptionAddress);
	PcrtOutPrint(hCore, "  Exception Flags   : %p (%s)\n", ex->ExceptionRecord->ExceptionFlags, (ex->ExceptionRecord->ExceptionFlags & EXCEPTION_NONCONTINUABLE ? "non-continueable" : "continueable"));
	PcrtOutPrint(hCore, "  Nested Exception  : %p (%d)\n", ex->ExceptionRecord->ExceptionRecord, (ex->ExceptionRecord->ExceptionRecord ? ex->ExceptionRecord->ExceptionRecord->ExceptionCode : 0));
	PcrtOutPrint(hCore, "  Number of Params  : %d\n", ex->ExceptionRecord->NumberParameters);
	
	if(ex->ExceptionRecord->NumberParameters > 0) {
		unsigned int i = 0;
		PcrtOutPrint(hCore, "  Parameters:\n");

		for(i = 0; i < ex->ExceptionRecord->NumberParameters; i++)
		{
			PcrtOutPrint(hCore, "    [%d] %p\n", i, ex->ExceptionRecord->ExceptionInformation[i]);
		}
	}

	PcrtOutPrint(hCore, "\n");
	PcrtOutPrint(hCore, "Specific Exception Information:\n");

	switch(ex->ExceptionRecord->ExceptionCode)
	{
	case EXCEPTION_ACCESS_VIOLATION:
		if((int)ex->ExceptionRecord->ExceptionInformation[0] == 8)
			PcrtOutPrint(hCore, "  Access Violation (DEP) at %p\n", ex->ExceptionRecord->ExceptionAddress);
		else
			PcrtOutPrint(hCore, "  Access Violation at %p trying to %s memory at %p\n", ex->ExceptionRecord->ExceptionAddress, ((int)ex->ExceptionRecord->ExceptionInformation[0] == 1 ? "write" : "read"), ex->ExceptionRecord->ExceptionInformation[1]);
		break;
	case EXCEPTION_IN_PAGE_ERROR:
		if((int)ex->ExceptionRecord->ExceptionInformation[0] == 8)
			PcrtOutPrint(hCore, "  Page Mapping Error (DEP) at %p\n", ex->ExceptionRecord->ExceptionAddress);
		else
			PcrtOutPrint(hCore, "  Page Mapping Error %x at %p trying to %s memory at %p\n", ex->ExceptionRecord->ExceptionInformation[2], ex->ExceptionRecord->ExceptionAddress, ((int)ex->ExceptionRecord->ExceptionInformation[0] == 1 ? "write" : "read"), ex->ExceptionRecord->ExceptionInformation[1]);
		break;

	/* other exception codes... */
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:		PcrtOutPrint(hCore, "  Array bounds exceeded\n"); break;
	case EXCEPTION_DATATYPE_MISALIGNMENT:		PcrtOutPrint(hCore, "  Datatype misaligned\n"); break;
	case EXCEPTION_FLT_DENORMAL_OPERAND:		PcrtOutPrint(hCore, "  Floating Point: Denormal Operator\n"); break;
	case EXCEPTION_FLT_DIVIDE_BY_ZERO:			PcrtOutPrint(hCore, "  Floating Point: Devide by Zero\n"); break;
	case EXCEPTION_FLT_INEXACT_RESULT:			PcrtOutPrint(hCore, "  Floating Point: Inexact Result (result cannot be represented as Decimal)\n"); break;
	case EXCEPTION_FLT_INVALID_OPERATION:		PcrtOutPrint(hCore, "  Floating Point: Invalid Operation\n"); break;
	case EXCEPTION_FLT_OVERFLOW:				PcrtOutPrint(hCore, "  Floating Point: Overflow. The Exponent is greater than the magnitude allowed by the corresponding type\n"); break;
	case EXCEPTION_FLT_STACK_CHECK:				PcrtOutPrint(hCore, "  Floating Point: Stack over- or underflow\n"); break;
	case EXCEPTION_FLT_UNDERFLOW:				PcrtOutPrint(hCore, "  Floating Point: Undeflow. The Exponent is lower than the magnitude allowed by the corresponding type\n"); break;
	case EXCEPTION_ILLEGAL_INSTRUCTION:			PcrtOutPrint(hCore, "  Invalid Instruction\n"); break;
	case EXCEPTION_INT_DIVIDE_BY_ZERO:			PcrtOutPrint(hCore, "  Integer Division by Zero\n"); break;
	case EXCEPTION_INT_OVERFLOW:				PcrtOutPrint(hCore, "  Integer Overflow\n"); break;
	case EXCEPTION_INVALID_DISPOSITION:			PcrtOutPrint(hCore, "  Invalid Exception Disposition\n"); break;
	case EXCEPTION_NONCONTINUABLE_EXCEPTION:	PcrtOutPrint(hCore, "  Noncontinuable Exception tried to continue\n"); break;
	case EXCEPTION_PRIV_INSTRUCTION:			PcrtOutPrint(hCore, "  Tried to execute a Priviledged Instruction, which is not allowed in the current machine state\n"); break;
	case EXCEPTION_STACK_OVERFLOW:				PcrtOutPrint(hCore, "  Stack Overflow\n"); break;
	}

	CloseHandle(hCore);

	//
	// this should result in process termination
	//
	return EXCEPTION_CONTINUE_SEARCH;
}

void PcrtSetupExceptionHandling()
{
	//
	// WARNING: only very basic initialization can be done here,
	// since mainCRTStartup has not run yet, and thus std streams
	// etc. may not be initialized yet.
	//
	if(!AddVectoredExceptionHandler(1, PcrtHandleException)) {
		//
		// i know, std streams...
		//
		fprintf(stderr, "failed to install exception handler!\n");
	}
}
