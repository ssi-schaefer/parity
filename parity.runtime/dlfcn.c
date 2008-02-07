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

#include "dlfcn.h"

//
// code comes from parity.loader
//
#include "LoaderInit.h"
#include "LoaderHelper.h"
#include "LoaderLibrary.h"

#include <windows.h>

static const char* tabErrors[] = {
	"No Error",
	"Cannot find given library",
	"Cannot find given symbol"
};

static const char* curError;

int dlclose(void* handle)
{
	//
	// due to limitations in the library cache this
	// cannot be done!
	//
	curError = tabErrors[0];
	return 0;
}

char* dlerror()
{
	const char* ptr = curError;
	curError = tabErrors[0];
	return (char*)ptr;
}

void* dlopen(const char* name, int flags)
{
	void* handle = 0;
	const char* conv = LoaderConvertPathToNative(name);
	int unsetEnv = 0;
	curError = tabErrors[0];

	if(flags & RTLD_NOW && !GetEnvironmentVariable("LD_BIND_NOW", 0, 0)) {
		SetEnvironmentVariable("LD_BIND_NOW", "on");
		unsetEnv = 1;
	}

	//
	// TODO: implement handling of import libraries?
	//
	handle = LoaderLibraryGetHandle(conv, 0);

	if(unsetEnv)
		SetEnvironmentVariable("LD_BIND_NOW", 0);

	if(!handle)
		curError = tabErrors[1];

	return handle;
}

void* dlsym(void* handle, const char* symbol)
{
	void* sym = 0;
	curError = tabErrors[0];
	sym = GetProcAddress((HMODULE)handle, symbol);

	if(!sym)
		curError = tabErrors[2];

	return sym;
}

