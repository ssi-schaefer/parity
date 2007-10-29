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

#include "internal/pcrt.h"
#include "string.h"

char* basename(char* path)
{
	static char pPoint[] = ".";
	static char pSlash[] = "/";
	char* end;

	if(!path || *path == '\0')
		return pPoint;

	end = path + strlen(path) - 1;

	if(end == path && (path[0] == '/' || path[0] == '\\'))
		return pSlash;

	while((*end == '\\' || *end == '/') && end >= path)
		*end-- = '\0';

	if((end + 1) == path)
		return pSlash;

	while(*end != '\\' && *end != '/' && end >= path)
		--end;

	while((end-1) >= path && *(end-1) == '/' || *(end-1) == '\\')
		--end;

	return ++end;
}

char* dirname(char* path)
{
	static char pPoint[] = ".";
	static char pSlash[] = "/";
	char* end;

	if(!strchr(path, '/') && !strchr(path, '\\'))
		return pPoint;

	if(!path || *path == '\0')
		return pPoint;

	end = path + strlen(path) - 1;

	if(end == path && (path[0] == '/' || path[0] == '\\'))
		return pSlash;

	while((*end == '\\' || *end == '/') && end >= path)
		*end-- = '\0';

	if((end + 1) == path)
		return pSlash;

	while(*end != '\\' && *end != '/' && end >= path)
		--end;

	while((end-1) >= path && *(end-1) == '/' || *(end-1) == '\\')
		--end;

	if(end == path)
		++end;

	*end = '\0';

	return path;
}

