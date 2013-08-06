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

/* require at least windows XP API for GetModuleHandleEx to
 * behave as we expect it for symbol searching... */
#define WINBASE_DECLARE_GET_MODULE_HANDLE_EX

#include <windows.h>
#include <stdio.h>

#include <dbghelp.h>

#include "internal/diagnose.h"
#include "internal/output.h"
#include "libgen.h"

#define EXCEPTION_DEBUG_OUT 0x40010006
#define STACKTRACE_MAX_NONDETAILED_FRAMES 99

extern void* _ReturnAddress();
#pragma intrinsic(_ReturnAddress)

int PcrtWaitForDebugger(int timeout) {
	PcrtOutPrint(GetStdHandle(STD_ERROR_HANDLE), "Process %d waiting for Debugger to be attached (%d seconds): ", GetCurrentProcessId(), timeout);

	while(timeout > 0 && !IsDebuggerPresent()) {
		Sleep(1000);
		if(!(timeout % 5))
			PcrtOutPrint(GetStdHandle(STD_ERROR_HANDLE), "%d", timeout);
		else
			PcrtOutPrint(GetStdHandle(STD_ERROR_HANDLE), ".");

		FlushFileBuffers(GetStdHandle(STD_ERROR_HANDLE));

		timeout--;
	}

	if(!IsDebuggerPresent()) {
		PcrtOutPrint(GetStdHandle(STD_ERROR_HANDLE), " fail (timeout)\n");
		return -1;
	}

	PcrtOutPrint(GetStdHandle(STD_ERROR_HANDLE), " success\n");

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

SymbolLookupType PcrtUseDebugSymbols() {
	//
	// Determine from Environment wether we should use Debug Informations
	// (slower and maybe unavailable)...
	//
	static int checked = 0;
	static DWORD dwSz;
	
	if(!checked) {
		dwSz = GetEnvironmentVariableA("PCRT_INTERNAL_SYMBOLS", NULL, 0);
		checked = 1;
	}

	if(dwSz != 0)
		return LookupInternal;

	return LookupDebugInfo;
}

stackframe_t* PcrtGetStackTraceFrom(void** _bp, void* _ip)
{
	stackframe_t* trace = NULL;
	stackframe_t* last = NULL;
	int numCallers = 0;

	while(numCallers++ < 100 && _bp && _ip
	 /* stack growing still valid */
	 && (intptr_t)_bp >= (last ? ( ((intptr_t)last->ebp) + (sizeof(void*)*2) ) : (intptr_t)0)
	) {
		stackframe_t* frame = (stackframe_t*)malloc(sizeof(stackframe_t));

		if(!frame) {
			PcrtOutPrint(GetStdHandle(STD_ERROR_HANDLE), "cannot allocate memory for stack trace!");
			return NULL;
		}		

		frame->eip = _ip;
		frame->ebp = _bp;
		frame->next= NULL;
		frame->size = 0;

		if(last) {
			frame->size = (unsigned long)frame->ebp - (unsigned long)last->ebp;

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
		}

		_ip = _bp[1];

		frame->ret = _ip;

		//
		// ATTENTION: contains an allocated (strdup) pointer
		// to the name, so free it again!
		//
		frame->sym = PcrtGetNearestSymbol(frame->eip, PcrtUseDebugSymbols());

		if(!trace) {
			trace = frame;
		} else if(last) {
			last->next = frame;
		} else {
			PcrtOutPrint(GetStdHandle(STD_ERROR_HANDLE), "internal error creating stack trace (missing nodes).");
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

		if(trace->sym.name)
			free(trace->sym.name);

		free(trace);
		trace = next;
	}

	return NULL;
}

static char const* PcrtGetGuardMutexName() {
	static char buffer[MAX_PATH] = { 0 };

	if(buffer[0] == '\0') {
		PcrtOutFormatString(buffer, "Local\\PcrtDebugInfoMutex-%p", GetCurrentProcessId());
	}

	return buffer;
}

static int PcrtIsDebugInitialized() {
	//
	// determine wether we have allready initialized the
	// debugging information...
	//
	HANDLE hMutex = OpenMutex(MUTEX_ALL_ACCESS, 0, PcrtGetGuardMutexName());

	if(hMutex) {
		CloseHandle(hMutex);
		return 1;
	}

	return 0;
}

static void PcrtGuardDebugInitialization() {
	if(PcrtIsDebugInitialized()) {
		PcrtOutPrint(GetStdHandle(STD_ERROR_HANDLE), "cannot guard debug information initialization, since it already is guarded.\n");
		return;
	}

	//
	// the mutex object is slightly misused here: guarding
	// the initialization creates a mutex object, but nobody
	// ever waits on it, only it's existance is checked.
	//
	{
		HANDLE hMutex = CreateMutex(NULL, 0, PcrtGetGuardMutexName());

		if(!hMutex) {
			PcrtOutPrint(GetStdHandle(STD_ERROR_HANDLE), "cannot create guard mutex, application may crash if more than one image try to retrieve symbol information.\n");
		}

		//
		// intentionally don't close the handle, since otherwise
		// the mutex would disappear again, which we don't want.
		//
	}
}

typedef BOOL (WINAPI * SymInitFunc)(HANDLE, PCTSTR, BOOL);
typedef BOOL (WINAPI * SymAddrFunc)(HANDLE, DWORD64, DWORD64*, SYMBOL_INFO*);
typedef BOOL (WINAPI * SymRefreshFunc)(HANDLE);
typedef BOOL (WINAPI * EnumModulesFunc)(HANDLE, PENUMLOADED_MODULES_CALLBACK64, PVOID);

static HANDLE			hDbgLib;
static SymInitFunc		hInit;
static SymAddrFunc		hSym;
static SymRefreshFunc	hRefresh;
static EnumModulesFunc	hEnumModules;

void PcrtInitializeDebugInformation() {
	if(!PcrtIsDebugInitialized())
	{
		PcrtGuardDebugInitialization();

		hDbgLib = LoadLibrary("dbghelp.dll");

		if(!hDbgLib) {
			PcrtOutPrint(GetStdHandle(STD_ERROR_HANDLE), "cannot load dbghelp.dll required for debug symbol handling!\n");
			return;
		}

		hInit	= (SymInitFunc)		GetProcAddress(hDbgLib, "SymInitialize");
		hSym	= (SymAddrFunc)		GetProcAddress(hDbgLib, "SymFromAddr");
		hRefresh= (SymRefreshFunc)	GetProcAddress(hDbgLib, "SymRefreshModuleList");
		hEnumModules= (EnumModulesFunc)	GetProcAddress(hDbgLib, "EnumerateLoadedModules64");

		//
		// don't check for hRefresh, since this requires dbghelp.dll v6.5
		// which won't be there always.
		//
		if(!hInit || !hSym) {
			PcrtOutPrint(GetStdHandle(STD_ERROR_HANDLE), "cannot load dbghelp.dll symbols.\n");
		}

		PcrtOutPrint(GetStdHandle(STD_ERROR_HANDLE), "initializing debug information, please wait ... ");

		if(!hInit(GetCurrentProcess(), NULL, TRUE)) {
			PcrtOutPrint(GetStdHandle(STD_ERROR_HANDLE), "failed!\n");
		} else {
			PcrtOutPrint(GetStdHandle(STD_ERROR_HANDLE), "done.\n");
		}
	} else {
		//
		// refresh symbol list.
		//
		if(hRefresh && !hRefresh(GetCurrentProcess())) {
			PcrtOutPrint(GetStdHandle(STD_ERROR_HANDLE), "cannot refresh debug information for loaded modules.\n");
		}
	}
}

syminfo_t PcrtGetNearestSymbol(void* addr, SymbolLookupType t)
{
	syminfo_t info = { 0, 0 };

	if(t == LookupDebugInfo && PcrtUseDebugSymbols() == LookupDebugInfo) {
		if(!PcrtIsDebugInitialized())
			PcrtInitializeDebugInformation();
	} else if(t == LookupDebugInfo) {
		PcrtOutPrint(GetStdHandle(STD_ERROR_HANDLE), "warning: requested debug symbols, but those are not enabled!\n");
	}
	
	if(t == LookupDebugInfo && hSym) {
		//
		// Symbol initialization should be done only once, except
		// when it is de-initialized. for now we take the penalty
		// and de-initialize, so:
		//
		// using debug symbols displays undecorated names by default
		// whereas parity's mechanism displays decorated ones, which
		// of course is a little bit faster.
		//
		// ATTENTION: Using the debug symbols nearly always gives
		//            good results, even on binaries without debug
		//            information, since parity exports almost all
		//            symbols, and this information suffices.
		//            For not exported symbols, debug symbols behave
		//            exactly the same as paritys internal mechanism
		//            and the nearest exported symbol is returned.
		//            Since parity generates symbol information for
		//            some symbols it doesn't export, parity's internal
		//            mechanism can be more accurate on release builds.
		//
		// ATTENTION: Using the debug symbols can lead to a silent crash
		//            of the application on stack overflows. this is
		//            because the dbghelp.dll seems to require more stack
		//            space than is available on the guard page. parity's
		//            tracing and lookup functionality is slim enough to
		//            fit in approximately 5 stack frames with each only
		//            very few locals.
		//
		
		DWORD64			iDisplacement;
		unsigned char	aBuffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME];
		SYMBOL_INFO*	hSymInfo = (SYMBOL_INFO*)&aBuffer;

		memset(&aBuffer, 0, sizeof(aBuffer));
		hSymInfo->MaxNameLen = MAX_SYM_NAME;
		hSymInfo->SizeOfStruct = sizeof(SYMBOL_INFO);

		if(!hSym(GetCurrentProcess(), (DWORD64)addr, &iDisplacement, hSymInfo)) {
			return info;
		}

		info.addr = (void*)hSymInfo->Address;
		info.name = _strdup(hSymInfo->Name);

		return info;
	} else {
		syminfo_t* symtab;
		modinfo_t module = PcrtGetContainingModule(addr);

		if(!module.module) {
			PcrtOutPrint(GetStdHandle(STD_ERROR_HANDLE), "cannot obtain module information for symbol lookup (%p)!\n", addr);
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
			if(((unsigned long)symtab->addr <= (unsigned long)addr)
				&& ((unsigned long)symtab->addr > (unsigned long)info.addr))
			{
				info = *symtab;
			}

			++symtab;
		}

		//
		// take control over the name buffer, since this is
		// const otherwise (buffer is free'd in the Destroy
		// function...
		//
		if(info.name)
			info.name = _strdup(info.name);
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
	static PGET_MODULE_HANDLE_EXA fGetModuleHandleExA = 0;

	if (!fGetModuleHandleExA) {
		fGetModuleHandleExA = (PGET_MODULE_HANDLE_EXA)GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetModuleHandleExA");
	}

	if(fGetModuleHandleExA
	&& !fGetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)addr, &info.module)) {
		PcrtOutPrint(GetStdHandle(STD_ERROR_HANDLE), "cannot find handle for module containing %p\n", addr);
		return info;
	}

	if(!fGetModuleHandleExA || !GetModuleFileNameA(info.module, info.name, MAX_PATH)) {
		strcpy(info.name, "<unknown>");
	}

	return info;
}

void PcrtWriteExceptionInformation(HANDLE hCore, struct _EXCEPTION_POINTERS* ex, int detailed)
{
	long num = 0;
	stackframe_t* trace;

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

	case 0xE06D7363:
		if(ex->ExceptionRecord->NumberParameters >= 3)
			PcrtOutPrint(hCore, "  Native C++ Exception. C++ Frame Magic: %p, Object pointer: %p, Type: %x\n", ex->ExceptionRecord->ExceptionInformation[0], ex->ExceptionRecord->ExceptionInformation[1], ex->ExceptionRecord->ExceptionInformation[2]);
		else
			PcrtOutPrint(hCore, "  Possibly Native C++ Exception (damaged exception information)\n"); break;
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
	case EXCEPTION_BREAKPOINT:					PcrtOutPrint(hCore, "  Debug Breakpoint\n"); break;
	case EXCEPTION_SINGLE_STEP:					PcrtOutPrint(hCore, "  Single Step Breakpoint\n"); break;
	case 0xE0434F4D:							PcrtOutPrint(hCore, "  Managed C++ (CLR) Exception.\n"); break;
	default:									PcrtOutPrint(hCore, "  Unrecognized Structured Exception.\n"); break;
	}

	PcrtOutPrint(hCore, "\n");
	PcrtOutPrint(hCore, "General Exception Information:\n");

	PcrtOutPrint(hCore, "  Exception Code    : %p\n", ex->ExceptionRecord->ExceptionCode);
	PcrtOutPrint(hCore, "  Exception Address : %p\n", ex->ExceptionRecord->ExceptionAddress);
	PcrtOutPrint(hCore, "  Exception Flags   : %p (%s)\n", ex->ExceptionRecord->ExceptionFlags, (ex->ExceptionRecord->ExceptionFlags & EXCEPTION_NONCONTINUABLE ? "non-continueable" : "continueable"));
	PcrtOutPrint(hCore, "  Nested Exception  : %p (%d at %p)\n", ex->ExceptionRecord->ExceptionRecord, (ex->ExceptionRecord->ExceptionRecord ? ex->ExceptionRecord->ExceptionRecord->ExceptionCode : 0), (ex->ExceptionRecord->ExceptionRecord ? ex->ExceptionRecord->ExceptionRecord->ExceptionAddress : 0));
	PcrtOutPrint(hCore, "  Number of Params  : %d\n", ex->ExceptionRecord->NumberParameters);
	
	if(ex->ExceptionRecord->NumberParameters > 0) {
		unsigned int i = 0;
		PcrtOutPrint(hCore, "  Parameters:\n");

		for(i = 0; i < ex->ExceptionRecord->NumberParameters; i++)
		{
			PcrtOutPrint(hCore, "    [%d] %p\n", i, ex->ExceptionRecord->ExceptionInformation[i]);
		}
	}

	if(detailed) {
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
	}

	PcrtOutPrint(hCore, "\n");

	//
	// cannot use the PcrtPrintStackTrace, since we should not rely
	// on the CRT working after an exception. the kernel functions
	// should still work i guess ;)
	//
	trace = PcrtGetStackTraceFrom((void*)ex->ContextRecord->Ebp, (void*)ex->ContextRecord->Eip);

	PcrtOutPrint(hCore, "Stack Layout at the time the exception occured:\n\n");

	while(trace) {
		//
		// PcrtOutPrint does not know too extensive formatting stuff, so
		// we have to do some of the work per pedes here.
		//
		modinfo_t mod = PcrtGetContainingModule(trace->eip);
		char modname_aligned[ST_FW_MODULE + 2];
		char * modname_unaligned = basename(mod.name);
		long len = lstrlen(modname_unaligned);

		if(len > ST_FW_MODULE)
			len = ST_FW_MODULE;

		memset(modname_aligned, ' ', sizeof(modname_aligned));
		lstrcpyn(&modname_aligned[ST_FW_MODULE - len], modname_unaligned, ST_FW_MODULE + 1);

		++num;

		PcrtOutPrint(hCore, " [%s%d]", ( num < 100000 ? ( num < 10000 ? ( num < 1000 ? ( num < 100 ? ( num < 10 ? "     " : "    " ) : "   ") : "  ") : " ") : ""), num);
		PcrtOutPrint(hCore, " %s!%p %s(%d bytes)+0x%x\n", modname_aligned, trace->eip, (trace->sym.name ? trace->sym.name : "???"), trace->size, (trace->sym.addr ? ((unsigned long)trace->eip - (unsigned long)trace->sym.addr) : 0));

		if(!trace->next)
			break;

		if(num >= STACKTRACE_MAX_NONDETAILED_FRAMES && !detailed) {
			PcrtOutPrint(hCore, " ... additional frames omitted in non-detailed mode ...\n");
			break;
		}

		trace = trace->next;
	}

	trace = PcrtDestroyStackTrace(trace);
}

static BOOL CALLBACK PcrtWriteModuleInformationCb(PCTSTR name, DWORD64 base, ULONG size, PVOID calldata)
{
	HANDLE hCore = (HANDLE)calldata;
	DWORD64 last = base + size;
	if (sizeof(PVOID) <= 4) {
		/* 32bit */
		PcrtOutPrint(hCore, "  %p - %p %s\n", (DWORD)base, (DWORD)last, name);
	} else {
		/* 64bit */
		PcrtOutPrint(hCore, "  %p%p - %p%p %s\n",
			(DWORD)(base >> 32), (DWORD)base,
			(DWORD)(last >> 32), (DWORD)last,
			name);
	}
	return TRUE;
}

static void PcrtWriteModulesInformation(HANDLE hCore)
{
	PcrtOutPrint(hCore, "Loaded Modules:\n");

	if (!hEnumModules) {
		PcrtOutPrint(hCore, "  Enumerating loaded modules not available.\n");
		return;
	}

	hEnumModules(GetCurrentProcess(), PcrtWriteModuleInformationCb, (PVOID)hCore);
}

LONG CALLBACK PcrtHandleException(struct _EXCEPTION_POINTERS* ex)
{
	static int nested_count = 0;
	HANDLE hCore;

	if(GetEnvironmentVariableA("PCRT_ENABLE_CRASHBOXES", NULL, 0) == 0) {
		//
		// Disable various error boxes, which we no longer need,
		// as we create core files after seting up exception
		// handling.
		//
		SetErrorMode(SetErrorMode(0) | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);
	}

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
		// write coredump and abort
		break;

	case EXCEPTION_BREAKPOINT:
	case EXCEPTION_SINGLE_STEP:
		//
		// re-enable error message boxes, to enable debugger attachement.
		//
		if(!IsDebuggerPresent()) {
			SetErrorMode(SetErrorMode(0) & ~SEM_NOGPFAULTERRORBOX);
		}
		return EXCEPTION_CONTINUE_SEARCH;

	default:
		/* those exceptions are not handled (includes C++ exceptions) */
		return EXCEPTION_CONTINUE_SEARCH;
	}

	if(nested_count++ == 0) {
		hCore = CreateFile("core", GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		if(hCore == INVALID_HANDLE_VALUE) {
			hCore = GetStdHandle(STD_ERROR_HANDLE);
			PcrtOutPrint(hCore, "ERROR: internal nested exception:\n");
		}

		PcrtWriteExceptionInformation(hCore, ex, 1);
		PcrtOutPrint(hCore, "\n");
		PcrtWriteModulesInformation(hCore);
		PcrtOutPrint(hCore, "\nEnd of core file.\n");

		PcrtOutPrint(GetStdHandle(STD_ERROR_HANDLE), "Exception %p at %p (core dumped)\n", ex->ExceptionRecord->ExceptionCode, ex->ExceptionRecord->ExceptionAddress);

		CloseHandle(hCore);
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

static HANDLE hTraceFile = INVALID_HANDLE_VALUE;
static LONG CALLBACK PcrtHandleExceptionTrace(struct _EXCEPTION_POINTERS* ex) {
	SYSTEMTIME time;

	//
	// Only do something if we're enabled
	//
	if(hTraceFile == INVALID_HANDLE_VALUE || ex->ExceptionRecord->ExceptionCode == EXCEPTION_DEBUG_OUT)
		return EXCEPTION_CONTINUE_SEARCH;

	GetSystemTime(&time);

	PcrtOutPrint(hTraceFile, "Exception seen at %d:%d:%d.%d: %p at %p. Details follow:\n\n", time.wHour, time.wMinute, time.wSecond, time.wMilliseconds, ex->ExceptionRecord->ExceptionCode, ex->ExceptionRecord->ExceptionAddress);
	PcrtWriteExceptionInformation(hTraceFile, ex, 0);
	PcrtOutPrint(hTraceFile, "\n");

	return EXCEPTION_CONTINUE_SEARCH;
}

void PcrtSetupExceptionHandling()
{
	typedef 
	PVOID
	(WINAPI* AddVectoredExceptionHandler_t)(
		IN ULONG FirstHandler,
		IN PVECTORED_EXCEPTION_HANDLER VectoredHandler
		);
	AddVectoredExceptionHandler_t fAddVectoredExceptionHandler;
	unsigned long sz;
	//
	// WARNING: only very basic initialization can be done here,
	// since mainCRTStartup has not run yet, and thus std streams
	// etc. may not be initialized yet.
	//
	
	fAddVectoredExceptionHandler = (AddVectoredExceptionHandler_t)
		GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "AddVectoredExceptionHandler");

	if (!fAddVectoredExceptionHandler) {
		return;
	}

	//
	// Install the Exception Tracer if requested by environment.
	//
	if((sz = GetEnvironmentVariableA("PCRT_TRACE_EXCEPTIONS", NULL, 0)) != 0) {
		if(sz > 1) {
			char* buffer = (char*)HeapAlloc(GetProcessHeap(), 0, sz);

			if(!buffer) {
				PcrtOutPrint(GetStdHandle(STD_ERROR_HANDLE), "cannot allocate memory to read exception trace location, exception tracing not enabled.\n");
				return;
			}

			if(GetEnvironmentVariableA("PCRT_TRACE_EXCEPTIONS", buffer, sz) == 0) {
				PcrtOutPrint(GetStdHandle(STD_ERROR_HANDLE), "cannot read exception trace location from environment, exception tracing not enabled.\n");
				HeapFree(GetProcessHeap(), 0, buffer);
				return;
			}

			if(lstrcmpiA(buffer, "on") == 0 || lstrcmpiA(buffer, "yes") == 0 || (lstrlen(buffer) == 1 && (buffer[0] >= '0' && buffer[0] <= '9')))
				hTraceFile = GetStdHandle(STD_ERROR_HANDLE);
			else
				hTraceFile = CreateFile(buffer, GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

			HeapFree(GetProcessHeap(), 0, buffer);

			//
			// i know, that there is a small handle leak here in theory, but the file
			// should stay open all the time anyway, so why bother - the system closes
			// the handle on exit anyway.
			//
		} else {
			//
			// Set, but empty, use STDERR.
			//
			hTraceFile = GetStdHandle(STD_ERROR_HANDLE);
		}

		if(hTraceFile == INVALID_HANDLE_VALUE) {
			PcrtOutPrint(GetStdHandle(STD_ERROR_HANDLE), "unknown error opening stream for exception tracing, not enabled.\n");
			return;
		}

		{
			SYSTEMTIME time;
			GetSystemTime(&time);

			PcrtOutPrint(hTraceFile, "Exception Tracing started at %d:%d:%d.%d.\n\n", time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);
		}

		if(!fAddVectoredExceptionHandler(1, PcrtHandleExceptionTrace)) {
			PcrtOutPrint(GetStdHandle(STD_ERROR_HANDLE), "failed to install exception trace handler, exception tracing not enabled.\n");
		}
	}
}

