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

LoaderStatus ParityLoaderInit()
{
	int isBindNow = 0;
	LibraryItem* libraries = ParityLoaderGeneratedTable.libraries;

	LogInit();

	LogDebug("=== ParityLoader intializing for %s. ===\n", ParityLoaderGeneratedImageName);

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

		//
		// load library handle.
		//
		handle = LoaderLibraryGetHandle(libraries->name, 1);

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

