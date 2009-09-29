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

#include "LoaderLazy.h"
#include "LoaderLibrary.h"
#include "LoaderLog.h"
#include "LoaderHelper.h"

void ParityLoaderLazyLoadSymbol(ImportItem* item)
{
	void * handle;

	handle = LoaderLibraryGetHandle(item->library, 1);

	LogDebug("lazy loading %s...", item->name);

	//
	// it's not possible that handle is zero, because
	// the loader would bail out at another location
	// if this happens.
	//

	if(item->ordinal != 0)
		item->import = (void*)GetProcAddress((HMODULE)handle, (char*)item->ordinal);
	else
		item->import = (void*)GetProcAddress((HMODULE)handle, item->name);

	if(!item->import)
	{
		LogDebug("fail!\n");
		LogWarning("cannot load symbol %s from %s, is it the wrong library version?\n", item->name, item->library);

		ExitProcess(1);
	}

	LogDebug("ok (0x%x)\n", item->import);
}

typedef struct {
	const char* name;
} PreloadCacheNode;

static PreloadCacheNode* gPreloadLibCache = 0;
static unsigned int gPreloadLibCacheCount = 0;

static void PreloadCreateCache()
{
	unsigned int szPreload = 0;

	szPreload = GetEnvironmentVariable("LD_PRELOAD", 0, 0);

	if(szPreload) {
		char * ptrPreload = HeapAlloc(GetProcessHeap(), 0, szPreload);

		if(GetEnvironmentVariable("LD_PRELOAD", ptrPreload, szPreload) == 0) {
			LogWarning("cannot get environment variable LD_PRELOAD!\n");
		} else {
			char* start = ptrPreload;
			char* end = ptrPreload;

			LogDebug("building preload library cache...\n");

			while(end && *end != '\0')
			{
				int isSep = 0;

				while(*end != ' ' && *end != '\t' && *end != '\0')
					++end;

				if(*end == ' ' || *end == '\t')
				{
					isSep = 1;
					*end = '\0';
					++end;
				}

				if(strlen(start) > 0) {
					if(!gPreloadLibCache)
						gPreloadLibCache = HeapAlloc(GetProcessHeap(), 0, sizeof(PreloadCacheNode));
					else
						gPreloadLibCache = HeapReAlloc(GetProcessHeap(), 0, gPreloadLibCache, sizeof(PreloadCacheNode) * (gPreloadLibCacheCount + 1));

					if(!gPreloadLibCache)
					{
						LogWarning("cannot allocate %d bytes for preload cache, this will break preloading!\n", sizeof(PreloadCacheNode) * (gPreloadLibCacheCount + 1));
						LoaderWriteLastWindowsError();
						gPreloadLibCacheCount = 0;
					} else {
						gPreloadLibCache[gPreloadLibCacheCount].name = start;
						++gPreloadLibCacheCount;
					}
				}

				start = end;

				if(*end == '\0')
					break;
			}
		}
	}
}

void ParityLoaderPreloadSymbols()
{
	unsigned int i;

	if(GetEnvironmentVariable("LD_PRELOAD", 0, 0) == 0)
		return;

	if(!gPreloadLibCache)
		PreloadCreateCache();

	LogDebug("trying to preload symbols from %d preload libraries.\n", gPreloadLibCacheCount);

	for(i = 0; i < gPreloadLibCacheCount; ++i)
	{
		void * handle = LoaderLibraryGetHandle(LoaderConvertPathToNative(gPreloadLibCache[i].name), 0);
		LibraryItem* libs = ParityLoaderGeneratedTable.libraries;

		if(!handle) {
			LogDebug("cannot load preload library %s, skipping\n", gPreloadLibCache[i].name);
			continue;
		}

		while(libs->name && libs->imports)
		{
			ImportItem* item = libs->imports;

			while(item->name)
			{
				void* symbol = 0;
				//
				// try loading item from current preload library.
				//

				if(item->ordinal != 0)
					symbol = (void*)GetProcAddress((HMODULE)handle, (char*)item->ordinal);
				else
					symbol = (void*)GetProcAddress((HMODULE)handle, item->name);

				if(symbol)
				{
					LogDebug(" * %s: %s (0x%x).\n", gPreloadLibCache[i].name, item->name, symbol);
					item->import = symbol;
				}

				++item;
			}

			++libs;
		}
	}
}

