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

#include <stdlib.h>

#include "dlfcn.h"
#include "internal/pcrt.h"

//
// code comes from parity.loader
// to avoid circulars, we put an extern here.
// WARNING: this means, that using dlfcn creates
// a link-time dependency to the parity.loader lib!!
//
extern void* LoaderLibraryGetHandle(const char*, int);

#include <windows.h>

#define ERRORTAB \
	ERR_ITEM("No Error", NoError) \
	ERR_ITEM("Cannot find given library", LibError) \
	ERR_ITEM("Cannot find given symbol", SymError) \
	ERR_ITEM("Memory allocation error", MemError)

#define ERR_ITEM(x, y) x,
static const char* tabErrors[] = {
	ERRORTAB
};

#undef ERR_ITEM
#define ERR_ITEM(x, y) y,
enum {
	ERRORTAB
};

static const char* curError;

int dlclose(void* handle)
{
	if (LoaderLibraryReleaseHandle(handle)) {
		curError = tabErrors[NoError];
		return 0;
	}
	curError = tabErrors[LibError];
	return GetLastError();
}

char* dlerror()
{
	const char* ptr = curError;
	curError = tabErrors[NoError];
	return (char*)ptr;
}

void* dlopen(const char* name, int flags)
{
	void* handle = 0;
	const char* conv = PcrtPathToNative(name);
	int unsetEnv = 0;
	curError = tabErrors[NoError];

	if(flags & RTLD_NOW && !GetEnvironmentVariable("LD_BIND_NOW", 0, 0)) {
		SetEnvironmentVariable("LD_BIND_NOW", "on");
		unsetEnv = 1;
	}

	handle = LoaderLibraryGetHandle(conv, 0);

	if(!handle) {
		//
		// if we cannot find a handle with all our lookup mechanisms,
		// check wether an appended .dll helps, since the other name
		// may have been an import library (.so in our case).
		//
		char* conv_dll = (char*)malloc(strlen(conv) + 5); /* .dll + \0 */

		if(conv_dll) {
			strcpy(conv_dll, conv);
			strcat(conv_dll, ".dll");

			//
			//
			//
			handle = LoaderLibraryGetHandle(conv_dll, 0);

			free(conv_dll);
		} else {
			curError = tabErrors[MemError];
			return 0;
		}
	}

	if(unsetEnv)
		SetEnvironmentVariable("LD_BIND_NOW", 0);

	if(!handle)
		curError = tabErrors[LibError];

	return handle;
}

void* dlsym(void* handle, const char* symbol)
{
	void* sym = 0;
	curError = tabErrors[0];
	sym = (void*)GetProcAddress((HMODULE)handle, symbol);

	if(!sym)
		curError = tabErrors[SymError];

	return sym;
}

