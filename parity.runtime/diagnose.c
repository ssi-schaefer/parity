#include <windows.h>
#include <stdio.h>

#include "internal/diagnose.h"

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
