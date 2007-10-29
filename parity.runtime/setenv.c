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

#include "stdlib.h"

#include <errno.h>
#include <windows.h>

int setenv(const char* name, const char* value, int overwrite)
{
	if(!overwrite && GetEnvironmentVariable(name, 0, 0))
		return 0;

	if(strchr(name, '='))
	{
		errno = EINVAL;
		return -1;
	}
	
	if(SetEnvironmentVariable(name, value))
		return 0;

	return -1;
}

int unsetenv(const char* name)
{
	if(SetEnvironmentVariable(name, 0))
		return 0;

	return -1;
}

