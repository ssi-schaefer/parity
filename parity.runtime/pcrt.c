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

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include "internal/diagnose.h"
#include "internal/output.h"
#include "unistd.h"
#include <excpt.h>

#define RING_SIZE 8

typedef void (*cygwin_init_func_t)();
typedef void (*cygwin_conv_func_t)(const char*, char*);

const char* PcrtPathToNative(const char* ptr) {
	//
	// either return the same string, or allocate another one and convert.
	//
#ifdef _WIN32
	static char* pRing[RING_SIZE] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	static int iRingNum = -1;

	int szRoot = 0;

	if(!ptr)
		return 0;

	if(ptr[0] != '/')
		return ptr;

	iRingNum = ++iRingNum % RING_SIZE;

	if(pRing[iRingNum])
		HeapFree(GetProcessHeap(), 0, pRing[iRingNum]);

	pRing[iRingNum] = NULL;

	//
	// try converting ourselves if possible.
	//
	if((szRoot = GetEnvironmentVariable("INTERIX_ROOT_WIN", 0, 0)) != 0)
	{
		char* ptrRoot = HeapAlloc(GetProcessHeap(), 0, szRoot);

		if(!GetEnvironmentVariable("INTERIX_ROOT_WIN", ptrRoot, szRoot)) {
			HeapFree(GetProcessHeap(), 0, ptrRoot);
		} else {
			char* ptrWalk = 0;

			if(CompareString(LOCALE_USER_DEFAULT, 0, "/dev/fs", 7, ptr, 7) == CSTR_EQUAL)
			{
				ptr += 8;

				//
				// now we have the drive letter under our cursor...
				//
				pRing[iRingNum] = HeapAlloc(GetProcessHeap(), 0, lstrlen(ptr) + 2);
				
				lstrcpyn(pRing[iRingNum], ptr, 2);
				lstrcpyn(&pRing[iRingNum][1], ":", 2);
				lstrcpyn(&pRing[iRingNum][2], &ptr[1], lstrlen(ptr)); // not +1 since we're allread copying one cahr more.
			} else {
				int sub = 0;
				pRing[iRingNum] = HeapAlloc(GetProcessHeap(), 0, szRoot + lstrlen(ptr) + 1);

				if(ptrRoot[lstrlen(ptrRoot) - 1] == '\\' || ptrRoot[lstrlen(ptrRoot) - 1] == '/')
					sub = 1;
				
				lstrcpyn(pRing[iRingNum], ptrRoot, szRoot);
				lstrcpyn(&pRing[iRingNum][lstrlen(ptrRoot) - sub], ptr, lstrlen(ptr) + 1);
			}

			ptrWalk = pRing[iRingNum];

			while(ptrWalk && *ptrWalk != '\0') {
				if(*ptrWalk == '/')
					*ptrWalk = '\\';

				++ptrWalk;
			}

			HeapFree(GetProcessHeap(), 0, ptrRoot);

			return pRing[iRingNum];
		}
    } else if((szRoot = GetEnvironmentVariable("REX_ROOTS", 0, 0)) != 0) {
        char* ptrRoots = HeapAlloc(GetProcessHeap(), 0, szRoot);

        if(GetEnvironmentVariable("REX_ROOTS", ptrRoots, szRoot)) {
            // pair separated by ',' values in pair separated by ';', server path first, then client path
            //  -> <server1>;<client1>,<server2>;<client2>,...
            int lenClient, lenServer, found = 0;
            char lookBehind = 0;
            char *ptrServer, *ptrClient, *ptrFix, *ptrPairEnd;
            char* ptrWalk = ptrRoots;

            do {
                ptrPairEnd = strchr(ptrWalk, ',');
                if(ptrPairEnd != NULL) {
                    *ptrPairEnd = 0;
                }

                ptrServer = ptrWalk;
                ptrClient = strchr(ptrServer, ';');

                if(!ptrClient) {
                    continue;
                }

                *ptrClient = 0;
                ++ptrClient;

                // now we have one client -> server mapping pair available
                lenClient = lstrlen(ptrClient);
                if(CompareString(LOCALE_USER_DEFAULT, 0, ptrClient, lenClient, ptr, lenClient) == CSTR_EQUAL) {
                    found = 1;
                    pRing[iRingNum] = HeapAlloc(GetProcessHeap(), 0, lstrlen(ptr) + lstrlen(ptrServer) + 2);

                    lenServer = lstrlen(ptrServer);
                    lstrcpyn(pRing[iRingNum], ptrServer, lenServer + 1);
                    lstrcpyn(&pRing[iRingNum][lenServer + 1], &ptr[lenClient], lstrlen(&ptr[lenClient]) + 1);
                    pRing[iRingNum][lenServer] = '/';

                    // consolidate string - convert '/' -> '\' and remove all duplicate '/' and '\'
                    ptrFix = pRing[iRingNum];
                    ptrWalk = pRing[iRingNum];
                    while(*ptrWalk != 0) {
                        switch(*ptrWalk) {
                        case '/':
                        case '\\':
                            if(lookBehind != '\\') {
                                *ptrFix++ = '\\';
                            }
                            break;
                        default:
                            *ptrFix++ = *ptrWalk;
                        }

                        lookBehind = *(ptrFix - 1);
                        ptrWalk++;
                    }
                    *ptrFix = 0;

                    HeapFree(GetProcessHeap(), 0, ptrRoots);
                    return pRing[iRingNum];
                }

                if(ptrPairEnd != NULL) {
                    ptrWalk = ptrPairEnd + 1;
                }
            } while(ptrPairEnd != NULL);
        }
        HeapFree(GetProcessHeap(), 0, ptrRoots);
	} else {
		static HMODULE hCygLib = NULL;
		static int inited = 0;
		
		if(!hCygLib)
			hCygLib = LoadLibrary("cygwin1.dll");

		if(hCygLib)
		{
			static cygwin_init_func_t init = NULL; 
			static cygwin_conv_func_t conv = NULL; 

			if(!init)
				init = (cygwin_init_func_t)GetProcAddress(hCygLib, "cygwin_dll_init");
			if(!conv)
				conv = (cygwin_conv_func_t)GetProcAddress(hCygLib, "cygwin_conv_to_full_win32_path");

			if(init && conv)
			{
				//
				// allocate a ring slot, and then convert using cygwin dll.
				//
				pRing[iRingNum] = HeapAlloc(GetProcessHeap(), 0, _MAX_PATH);

				if(!inited++)
					init();

				conv(ptr, pRing[iRingNum]);

				return pRing[iRingNum];
			} else {
				PcrtOutPrint(GetStdHandle(STD_ERROR_HANDLE), "Cannot load all required functions from cygwin1.dll, cannot convert absolute UNIX paths!\n");
			}
		} else {
			PcrtOutPrint(GetStdHandle(STD_ERROR_HANDLE), "Neither REX, Interix Installation, or Cygwin DLL found, cannot convert absolute UNIX paths!\n");
		}
	}

	//
	// still around?
	//
	PcrtOutPrint(GetStdHandle(STD_ERROR_HANDLE), "would need real path conversion for %s. Please report this!\n", ptr);

	return ptr;
#else
	return ptr;
#endif
}

int PcrtInit(int staticCrt)
{
	//
	// The static CRT is not yet initialized when we reach this
	// code. This means that trying to do the following will
	// result in a program failure.
	//
	if(!staticCrt) {
		//
		// Set file buffering. On Windows, the default is a fully
		// buffered stdout and stderr, with a buffer size of 4096.
		// Line buffering is impossible on windows, since that
		// behaves the same as full buffering (argh...). So for now
		// I disable buffering completely, and think about how to
		// implement line buffering in parity.
		//
		setvbuf(stdout, NULL, _IONBF, 0);
		setvbuf(stderr, NULL, _IONBF, 0);

		//
		// Set stream modes to binary to stay compatible with the
		// rest of the world. this makes some autoconf patches
		// obsolete, which handled line ending conversion in some
		// check macros which rely on a single \n
		//
		_setmode(STDIN_FILENO, _O_BINARY);
		_setmode(STDOUT_FILENO, _O_BINARY);
		_setmode(STDERR_FILENO, _O_BINARY);
	}
	//
	// This effectively adds a VecoredExceptionHandler to the
	// process' chain, which enables us to watch all exceptions
	// as they fly by :). this way we can create core files
	// and still continue exception handling as normal. This
	// also means that writing a core file does not neccesarily
	// mean process termination, since the exception handler may
	// take an exception as fatal, where the Default handler does
	// not.
	//
	PcrtSetupExceptionHandling();

	//
	// Success ...
	//
	return 1;
}
