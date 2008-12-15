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

#include "LoaderHelper.h"
#include "LoaderLog.h"

#include <internal/pcrt.h>

char* LoaderFormatErrorMessage(unsigned int error) {
	char* ptrMsg = 0;

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		error,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&ptrMsg,
		0, NULL );

	if(ptrMsg)
		ptrMsg[strlen(ptrMsg) - 2] = 0; // newline.

	return ptrMsg;
}

void LoaderWriteLastWindowsError()
{
	char* ptrMsg = LoaderFormatErrorMessage(GetLastError());

	LogWarning("windows reports the following error: %s\n", ptrMsg);

	LocalFree(ptrMsg);
}

const char* LoaderConvertPathToNative(const char* ptr)
{
	return PcrtPathToNative(ptr);
}

