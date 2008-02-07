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

#include "LoaderInit.h"
#include "LoaderLibrary.h"
#include "LoaderLog.h"
#include "LoaderLazy.h"

#include <stdio.h>
#include <windows.h>

int iEnableRTL = 0;

static char * ParityLoaderGetMangledRTLName(const char* name)
{
	static char buffer[256];
	int i = 0;

	lstrcpy(buffer, "LD_RTL_ENABLE_");

	for(i = lstrlen(buffer); *name != '\0'; ++name)
	{
		char c = *name;
		if((c >= 0x30 && c <= 0x39 || c >= 0x41 && c <= 0x5A 
				|| c >= 0x61 && c <= 0x7A || c == '_'))
			buffer[i++] = c;
		else
			buffer[i++] = '_';
	}

	buffer[i] = '\0';
	return buffer;
}

LoaderStatus ParityLoaderInit()
{
	int isBindNow = 0;
	LibraryItem* libraries = ParityLoaderGeneratedTable.libraries;

	LogInit();

	LogDebug("=== ParityLoader intializing for %s. ===\n", ParityLoaderGeneratedImageName);

	switch(ParityLoaderGeneratedRuntimeLinking)
	{
	case RuntimeLinkageDisabled:
		iEnableRTL = 0;
		break;
	case RuntimeLinkageEnabled:
		iEnableRTL = 1;
		break;
	case RuntimeLinkageInherit:
		{
			char * pcEnvVar = ParityLoaderGetMangledRTLName(ParityLoaderGeneratedImageName);
			LogDebug("inheriting, trying %s\n", pcEnvVar);

			if(!GetEnvironmentVariable(pcEnvVar, 0, 0)) {
				LogDebug("inheriting, trying LD_RTL\n");
				if(GetEnvironmentVariable("LD_RTL", 0, 0)) {
					iEnableRTL = 1;
				}
			} else {
				iEnableRTL = 1;
			}
		}

		break;
	}

	LogDebug("runtime linking is %s\n", iEnableRTL ? "enabled" : "disabled");

	ParityLoaderPreloadSymbols();

	//
	// check wether bind now is set.
	//
	isBindNow = GetEnvironmentVariable("LD_BIND_NOW", 0, 0);

	while(libraries->name != 0 && libraries->imports != 0)
	{
		int			cntCode = 0;
		int			cntData = 0;
		int			cntLazy = 0;
		void*		handle = 0;
		ImportItem*	import = libraries->imports;
		char*		pcRtlVar = iEnableRTL ? ParityLoaderGetMangledRTLName(libraries->name) : 0;

		//
		// set runtime linkage flags accordingly...
		//
		if(pcRtlVar)
		{
			LogDebug("activating runtime linkage for %s (%s)\n", libraries->name, pcRtlVar);

			if(!SetEnvironmentVariable(pcRtlVar, "on"))
			{
				LogWarning("cannot set environment for runtime linkage (target %s)\n!", ParityLoaderGeneratedImageName);
			}

			//
			// Runtime link allready loaded stuff from children
			//
			ParityLoaderRuntimeLink(libraries);
		}

		//
		// load library handle.
		//
		handle = LoaderLibraryGetHandle(libraries->name, 1);

		//
		// unset runtime linkage flags.
		//
		if(pcRtlVar)
			SetEnvironmentVariable(pcRtlVar, 0);

		//
		// load all imports for that library.
		//
		while(import->name)
		{
			//
			// load specific import.
			//
			if((import->import == 0 || import->import == (void*)0xbaadf00d) || isBindNow)
			{
				//
				// load from library, otherwise its lazy loaded.
				//

				if(import->import == (void*)0xbaadf00d)
					++cntData;
				else
					++cntCode;

				if(import->ordinal != 0)
					import->import = (void*)GetProcAddress((HMODULE)handle, (char*)import->ordinal);
				else
					import->import = (void*)GetProcAddress((HMODULE)handle, import->name);

				if(!import->import) {
					LogWarning("cannot load symbol %s from %s!\n", import->name, import->library);
					ExitProcess(1);
				}

				//LogDebug(" * %s (0x%x)\n", import->name, import->import);
			} else {
				//LogDebug(" * %s (lazy 0x%x)\n", import->name, import->import);
				++cntLazy;
			}
			++import;
		}

		LogDebug("loaded %d symbols from %s (%d code, %d data, %d lazy)\n", cntCode+cntData, libraries->name, cntCode, cntData, cntLazy);

		++libraries;
	}

	LogDebug("=== ParityLoader finishing for %s. ===\n", ParityLoaderGeneratedImageName);

	return StatusSuccess;
}

