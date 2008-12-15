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

#include "LoaderLibrary.h"
#include "LoaderInit.h"
#include "LoaderLog.h"
#include "LoaderHelper.h"

#include <windows.h>

#define LOADER_MAX_ERRORS 64
#define LOADER_PATHFIELD_LEN 50

typedef struct {
	const char* name;
	void* handle;
} LoaderLibraryCacheNode;

typedef struct {
	const char* path;
} LoaderPathCacheNode;

typedef struct {
	const char* path;
	unsigned long error;
} LoaderErrorList;

static LoaderLibraryCacheNode* gHandleCache = 0;
static LoaderPathCacheNode* gPathCache = 0;

static unsigned int gPathCacheCount = 0;
static unsigned int gHandleCacheCount = 0;

static void* LibLoad(const char* name, unsigned int strict)
{
	void* handle = 0;
	unsigned int i;
	LoaderErrorList lst[LOADER_MAX_ERRORS];
	unsigned int longest = 0;

	if(!name)
		return GetModuleHandle(0);

	for(i = 0; i < gPathCacheCount; ++i)
	{
		char * ptrTry = HeapAlloc(GetProcessHeap(), 0, lstrlen(name) + lstrlen(gPathCache[i].path) + 2);

		lstrcpyn(ptrTry, gPathCache[i].path, lstrlen(gPathCache[i].path) + 1);
		ptrTry[lstrlen(gPathCache[i].path)] = '\\';
		lstrcpyn(&ptrTry[lstrlen(gPathCache[i].path)+1], name, lstrlen(name) + 1);

		handle = LoadLibrary(ptrTry);

		if(!handle && i <= LOADER_MAX_ERRORS) {
			register int len;

			lst[i].error = GetLastError();
			lst[i].path  = gPathCache[i].path;

			len = strlen(lst[i].path);
			if(len > longest)
				longest = len;
		}

		HeapFree(GetProcessHeap(), 0, ptrTry);

		if(handle) {
			LogDebug("loaded %s through path cache.\n", name);
			return handle;
		}
	}

	//
	// try default lookup
	//
	handle = LoadLibrary(name);

	if(!handle && i < LOADER_MAX_ERRORS) {
		lst[i].error = GetLastError();
		lst[i].path = "...";
	}

	if(handle)
	{
		LogDebug("loaded %s through windows default lookup or absolute name\n", name);
		return handle;
	}

	if(strict) {
		unsigned int y = 0;
		char spaces[MAX_PATH];

		FillMemory(spaces, sizeof(spaces), ' ');

		LogWarning("failed to load %s from following paths (%d in cache):\n", name, gPathCacheCount);

		for(y = 0; y <= i && y <= LOADER_MAX_ERRORS; ++y) {
			char* tmp = LoaderFormatErrorMessage(lst[y].error);
			register int index = longest - strlen(lst[y].path);

			spaces[index] = 0;

			LogWarning(" * [%s%d] %s%s (%s)\n", 
				( lst[y].error < 10000 ? ( lst[y].error < 1000 ? ( lst[y].error < 100 ? ( lst[y].error < 10 ? "    " : "   " ) : "  ") : " ") : ""),
				lst[y].error, lst[y].path, spaces, tmp);

			spaces[index] = ' ';

			LocalFree(tmp);
		}
	}

	return 0;
}

static void* LibGetCached(const char* name)
{
	unsigned int i;

	for(i = 0; i < gHandleCacheCount; ++i)
	{
		if(CompareString(LOCALE_USER_DEFAULT, 0, name, -1, gHandleCache[i].name, -1) == CSTR_EQUAL)
		{
			//LogDebug("found cached handle for %s (0x%x)\n", name, gHandleCache[i].handle);
			return gHandleCache[i].handle;
		}
	}

	return 0;
}

static void LibAddToCache(const char* name, void* handle)
{
	if(gHandleCache == 0)
		gHandleCache = HeapAlloc(GetProcessHeap(), 0, sizeof(LoaderLibraryCacheNode));
	else
		gHandleCache = HeapReAlloc(GetProcessHeap(), 0, gHandleCache, sizeof(LoaderLibraryCacheNode) * (gHandleCacheCount + 1));

	if(!gHandleCache)
	{
		LogWarning("cannot allocate %d bytes for the handle cache, this will slow down things a little!\n", sizeof(LoaderLibraryCacheNode) * (gHandleCacheCount + 1));
		LoaderWriteLastWindowsError();
		gHandleCacheCount = 0;
	} else {
		gHandleCache[gHandleCacheCount].name = name;
		gHandleCache[gHandleCacheCount].handle = handle;

		++gHandleCacheCount;

		LogDebug("cached handle for %s (0x%x)\n", name, handle);
	}
}

static const char* LibStrChr(const char* ptr, char c)
{
	while(*ptr != '\0' && *ptr != c) ++ptr;

	if(*ptr == c)
		return ptr;

	return 0;
}

static void LibCreatePathCache()
{
	unsigned int szLdLib = 0;
	const char* ptrRPath = ParityLoaderGeneratedRunPath;

	//
	// first lookup from LD_LIBRARY_PATH!
	// this has the effect, that LD_LIBRARY_PATH
	// is searched before the runpaths. (for libtool:
	// shlibpath_overrides_runpath = yes!). This saves
	// us from relinking all the time.
	//
	szLdLib = GetEnvironmentVariable("LD_LIBRARY_PATH", 0, 0);

	if(szLdLib > 1)
	{
		char* ptrLdLib = HeapAlloc(GetProcessHeap(), 0, szLdLib);
		char* start = 0;
		char* end = 0;

		if(!GetEnvironmentVariable("LD_LIBRARY_PATH", ptrLdLib, szLdLib))
		{
			LogWarning("cannot get environment variable LD_LIBRARY_PATH!\n");
		} else {
			char pSep = ':';
			start = ptrLdLib;
			end = ptrLdLib;

			if(LibStrChr(ptrLdLib, ';') || LibStrChr(ptrLdLib, '\\'))
				pSep = ';';

			LogDebug("adding LD_LIBRARY_PATH to path cache (sep: %c)\n", pSep);

			while(end && *end != '\0')
			{
				int isSep = 0;

				while(*end != pSep && *end != '\0')
					++end;

				if(*end == pSep) {
					*end = '\0';
					isSep = 1;
					++end;
				}

				if(gPathCache == 0)
					gPathCache = HeapAlloc(GetProcessHeap(), 0, sizeof(LoaderPathCacheNode));
				else
					gPathCache = HeapReAlloc(GetProcessHeap(), 0, gPathCache, sizeof(LoaderPathCacheNode) * (gPathCacheCount + 1));

				if(!gPathCache)
				{
					LogWarning("cannot allocate %d bytes for the path cache, this will break library loading!\n", sizeof(LoaderPathCacheNode) * (gPathCacheCount + 1));
					LoaderWriteLastWindowsError();
					gPathCacheCount = 0;
				} else {
					//
					// buffer2 is never freed, this is intentionally!
					//
					const char * buffer1 = LoaderConvertPathToNative(start);
					char * buffer2 = HeapAlloc(GetProcessHeap(), 0, sizeof(char) * (lstrlen(buffer1) + 1));
					lstrcpy(buffer2, buffer1);
					gPathCache[gPathCacheCount].path = buffer2;

					LogDebug(" * %s\n", gPathCache[gPathCacheCount].path);

					++gPathCacheCount;
				}

				if(!isSep)
					break;

				start = end;
			}
		}
	}

	//
	// second lookup runpaths, pointers can be stored directly.
	//
	LogDebug("adding runpaths to path cache\n");

	while(ptrRPath && *ptrRPath != '\0')
	{
		if(gPathCache == 0)
			gPathCache = HeapAlloc(GetProcessHeap(), 0, sizeof(LoaderPathCacheNode));
		else
			gPathCache = HeapReAlloc(GetProcessHeap(), 0, gPathCache, sizeof(LoaderPathCacheNode) * (gPathCacheCount + 1));

		if(!gPathCache)
		{
			LogWarning("cannot allocate %d bytes for the path cache, this will break library loading!\n", sizeof(LoaderPathCacheNode) * (gPathCacheCount + 1));
			LoaderWriteLastWindowsError();
			gPathCacheCount = 0;
		} else {
			gPathCache[gPathCacheCount].path = ptrRPath;
			++gPathCacheCount;

			LogDebug(" * %s\n", ptrRPath);
		}

		while(*ptrRPath != '\0')
			++ptrRPath;

		++ptrRPath;
	}
}

void* LoaderLibraryGetHandle(const char* name, int strict)
{
	void* handle = 0;
	unsigned int errormode = 0;

	if(gPathCache == 0)
		LibCreatePathCache();

	handle = LibGetCached(name);

	if(handle)
		return handle;

	errormode = SetErrorMode(SEM_FAILCRITICALERRORS);
	SetErrorMode(errormode | SEM_FAILCRITICALERRORS);

	handle = LibLoad(name, strict);

	SetErrorMode(errormode);

	if(handle) {
		LibAddToCache(name, handle);
		return handle;
	} else if(strict) {
		/*
		 * Don't issue a warning here, since LibLoad does this already.
		 */
		ExitProcess(1);
	}

	return 0;
}

