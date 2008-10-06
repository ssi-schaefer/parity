#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif

#include <windows.h>
#include <stdio.h>

#include "internal/diagnose.h"

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


static DWORD WINAPI PcrtpDiagnoseThread(DWORD Unused) {
	DWORD dwContinuationStatus = DBG_CONTINUE;

	if(!DebugActiveProcess(GetCurrentProcessId())) {
		fprintf(stderr, "Cannot attach debugger to the current process: ");
		PcrtpPrintFormattedError(GetLastError());

		return -1;
	}

	for(;;) {
		//
		// Debugger loop
		//
		DEBUG_EVENT ev;
		if(!WaitForDebugEvent(&ev, INFINITE)) {
			fprintf(stderr, "Cannot wait for debug event: ");
			PcrtpPrintFormattedError(GetLastError());
		}

		fprintf(stderr, "got debug event: %d\n", ev.dwDebugEventCode);
	}
}

void PcrtPrintStackTrace()
{
	stackframe_t* stack = PcrtGetStackTrace();
	stackframe_t* walk = stack;
	unsigned int num = 0;

	if(!stack) {
		fprintf(stderr, "Cannot obtain Stack Trace!\n");
		return;
	}

	fprintf(stderr, "   #  Address  StackAddr.   Name\n");
	fprintf(stderr, " ---------------------------------\n");

	while(1) {
		fprintf(stderr, " [%2d] %p [%p] : %s\n", num++, walk->eip, walk->ebp, walk->sym);

		if(!walk->next)
			break;

		walk = walk->next;
	}

	fprintf(stderr, " ---------------------------------\n");
	stack = PcrtDestroyStackTrace(stack);
}

stackframe_t* PcrtGetStackTrace()
{
	//
	// Our starting point is the frame above ourselves.
	//
	void* _ip = _ReturnAddress();
	void** _bp;
	stackframe_t* trace = NULL;
	stackframe_t* last = NULL;

	__asm {
		mov _bp, ebp
	}

	_bp = (void**)_bp[0];

	while(_bp && _ip) {
		stackframe_t* frame = malloc(sizeof(stackframe_t));

		if(!frame) {
			fprintf(stderr, "cannot allocate memory for stack trace!");
			return NULL;
		}		

		frame->eip = _ip;
		frame->ebp = _bp;
		frame->next= NULL;

		_ip = _bp[1];

		frame->ret = _ip;
		frame->sym = NULL; /* dlinfo(....) ? */

		if(!trace) {
			trace = frame;
		} else if(last) {
			last->next = frame;
		} else {
			fprintf(stderr, "neither top nor last node set.");
			return NULL;
		}

		last = frame;
		_bp = (void**)_bp[0];
	}

	return trace;
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

	//
	// TODO: gather information from symbol tables.
	//

	return info;
}
