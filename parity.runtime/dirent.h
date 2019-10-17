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

#ifndef __PCRT_DIRENT_H__
#define __PCRT_DIRENT_H__

#include "internal/pcrt.h"
#include "unistd.h"

struct dirent
{
	long			d_ino;
	unsigned short	d_reclen;
	unsigned short	d_namlen;
	char			d_name[FILENAME_MAX];
};

struct _wdirent
{
	long			d_ino;
	unsigned short	d_reclen;
	unsigned short	d_namlen;
	wchar_t			d_name[FILENAME_MAX];
};

typedef struct {
	struct _finddata_t	dd_data;
	struct dirent		dd_dir;
	intptr_t			dd_handle;
	int					dd_index;
	char				dd_name[1];
} DIR;

typedef struct {
	struct _wfinddata_t	dd_data;
	struct _wdirent		dd_dir;
	intptr_t			dd_handle;
	int					dd_index;
	wchar_t				dd_name[1];
} _WDIR;

PCRT_BEGIN_C

DIR* opendir(const char*);
struct dirent* readdir(DIR*);
int closedir(DIR*);
void rewinddir(DIR*);
long telldir(DIR*);
void seekdir(DIR*, long);

_WDIR* _wopendir(const wchar_t*);
struct _wdirent* _wreaddir(_WDIR*);
int _wclosedir(_WDIR*);
void _wrewinddir(_WDIR*);
long _wtelldir(_WDIR*);
void _wseekdir(_WDIR*, long);

PCRT_END_C

#endif

