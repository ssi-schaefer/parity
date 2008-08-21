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

#include "dirent.h"
#include "errno.h"

#include "internal/pcrt.h"

#include <windows.h>

#define SUFFIX	"*"
#define	SLASH	"\\"

DIR* opendir(const char* szPath)
{
	DIR *nd;
	unsigned int rc;
	char szFullPath[MAX_PATH];
	const char* szConv = 0;

	errno = 0;

	if(!szPath) {
		errno = EFAULT;
		return (DIR *) 0;
	}

	if(szPath[0] == '\0') {
		errno = ENOTDIR;
		return (DIR *) 0;
	}

	szConv = PcrtPathToNative(szPath);

	rc = GetFileAttributes (szConv);
	if(rc == (unsigned int)-1) {
		errno = ENOENT;
		return (DIR *) 0;
	}
	if(!(rc & FILE_ATTRIBUTE_DIRECTORY)) {
		errno = ENOTDIR;
		return (DIR*) 0;
	}

	_fullpath(szFullPath, szConv, MAX_PATH);

	nd = (DIR*)malloc(sizeof(DIR) + (strlen(szFullPath) + strlen(SLASH) + strlen(SUFFIX) + 1) * sizeof(char));

	if (!nd) {
		errno = ENOMEM;
		return (DIR*) 0;
	}

	strcpy(nd->dd_name, szFullPath);

	/* Add on a slash if the path does not end with one. */
	if(nd->dd_name[0] != '\0' &&
		nd->dd_name[strlen(nd->dd_name) - 1] != '/' &&
		nd->dd_name[strlen(nd->dd_name) - 1] != '\\')
	{
		strcat(nd->dd_name, SLASH);
	}

	strcat(nd->dd_name, SUFFIX);

	nd->dd_handle = -1;
	nd->dd_index = 0;
	nd->dd_dir.d_ino = 0;
	nd->dd_dir.d_reclen = 0;
	nd->dd_dir.d_namlen = 0;
	memset (nd->dd_dir.d_name, 0, FILENAME_MAX);

	return nd;
}

struct dirent* readdir(DIR* dirp)
{
	errno = 0;

	if(!dirp) {
		errno = EFAULT;
		return (struct dirent *) 0;
	}

	if(dirp->dd_index < 0) {
		return (struct dirent *) 0;
	} else if(dirp->dd_index == 0) {
		dirp->dd_handle = _findfirst(dirp->dd_name, &(dirp->dd_data));

		if (dirp->dd_handle == -1)
			dirp->dd_index = -1;
		else
			dirp->dd_index = 1;
	} else {
		if(_findnext(dirp->dd_handle, &(dirp->dd_data))) {
			DWORD winerr = GetLastError();
			if (winerr == ERROR_NO_MORE_FILES)
				errno = 0;	

			_findclose(dirp->dd_handle);
			dirp->dd_handle = -1;
			dirp->dd_index = -1;
		} else {
			dirp->dd_index++;
		}
	}

	if(dirp->dd_index > 0)
	{
		dirp->dd_dir.d_namlen = strlen(dirp->dd_data.name);
		strcpy(dirp->dd_dir.d_name, dirp->dd_data.name);
		return &dirp->dd_dir;
	}

	return (struct dirent*) 0;
}

int closedir(DIR* dirp)
{
	int rc;

	errno = 0;
	rc = 0;

	if(!dirp) {
		errno = EFAULT;
		return -1;
	}

	if(dirp->dd_handle != -1) {
		rc = _findclose(dirp->dd_handle);
	}

	free (dirp);
	return rc;
}

void rewinddir(DIR* dirp)
{
	errno = 0;

	if(!dirp) {
		errno = EFAULT;
		return;
	}

	if(dirp->dd_handle != -1) {
		_findclose (dirp->dd_handle);
	}

	dirp->dd_handle = -1;
	dirp->dd_index = 0;
}

long telldir(DIR* dirp)
{
	errno = 0;

	if(!dirp) {
		errno = EFAULT;
		return -1;
	}

	return dirp->dd_index;
}

void seekdir(DIR* dirp, long lPos)
{
	errno = 0;

	if (!dirp) {
		errno = EFAULT;
		return;
	}

	if (lPos < -1) {
		errno = EINVAL;
		return;
	} else if(lPos == -1) {
		if(dirp->dd_handle != -1) {
			_findclose(dirp->dd_handle);
		}
		dirp->dd_handle = -1;
		dirp->dd_index = -1;
	} else {
		rewinddir(dirp);
		while((dirp->dd_index < lPos) && readdir (dirp));
	}
}

/* unicode stuff to follow... */

#define _WSUFFIX	L"*"
#define	_WSLASH		L"\\"

_WDIR* _wopendir(const wchar_t* szPath)
{
	_WDIR *nd;
	unsigned int rc;
	wchar_t szFullPath[MAX_PATH];

	errno = 0;

	if(!szPath) {
		errno = EFAULT;
		return 0;
	}

	if(szPath[0] == '\0') {
		errno = ENOTDIR;
		return 0;
	}

	// TODO: no conversion yet...
	//szConv = LoaderConvertPathToNative(szPath);

	rc = GetFileAttributesW(szPath);
	if(rc == (unsigned int)-1) {
		errno = ENOENT;
		return 0;
	}
	if(!(rc & FILE_ATTRIBUTE_DIRECTORY)) {
		errno = ENOTDIR;
		return 0;
	}

	_wfullpath(szFullPath, szPath, MAX_PATH);

	nd = (_WDIR*)malloc(sizeof(_WDIR) + (wcslen(szFullPath) + wcslen(_WSLASH) + wcslen(_WSUFFIX) + 1) * sizeof(wchar_t));

	if (!nd) {
		errno = ENOMEM;
		return 0;
	}

	wcscpy(nd->dd_name, szFullPath);

	/* Add on a slash if the path does not end with one. */
	if(nd->dd_name[0] != '\0' &&
		nd->dd_name[wcslen(nd->dd_name) - 1] != L'/' &&
		nd->dd_name[wcslen(nd->dd_name) - 1] != L'\\')
	{
		wcscat(nd->dd_name, _WSLASH);
	}

	wcscat(nd->dd_name, _WSUFFIX);

	nd->dd_handle = -1;
	nd->dd_index = 0;
	nd->dd_dir.d_ino = 0;
	nd->dd_dir.d_reclen = 0;
	nd->dd_dir.d_namlen = 0;
	memset(nd->dd_dir.d_name, 0, FILENAME_MAX * sizeof(wchar_t));

	return nd;
}

struct _wdirent* _wreaddir(_WDIR* dirp)
{
	errno = 0;

	if(!dirp) {
		errno = EFAULT;
		return 0;
	}

	if(dirp->dd_index < 0) {
		return 0;
	} else if(dirp->dd_index == 0) {
		dirp->dd_handle = _wfindfirst(dirp->dd_name, &(dirp->dd_data));

		if (dirp->dd_handle == -1)
			dirp->dd_index = -1;
		else
			dirp->dd_index = 1;
	} else {
		if(_wfindnext(dirp->dd_handle, &(dirp->dd_data))) {
			DWORD winerr = GetLastError();
			if (winerr == ERROR_NO_MORE_FILES)
				errno = 0;	

			_findclose(dirp->dd_handle);
			dirp->dd_handle = -1;
			dirp->dd_index = -1;
		} else {
			dirp->dd_index++;
		}
	}

	if(dirp->dd_index > 0)
	{
		dirp->dd_dir.d_namlen = wcslen(dirp->dd_data.name);
		wcscpy(dirp->dd_dir.d_name, dirp->dd_data.name);
		return &dirp->dd_dir;
	}

	return 0;
}

int _wclosedir(_WDIR* dirp)
{
	int rc;

	errno = 0;
	rc = 0;

	if(!dirp) {
		errno = EFAULT;
		return -1;
	}

	if(dirp->dd_handle != -1) {
		rc = _findclose(dirp->dd_handle);
	}

	free (dirp);
	return rc;
}

void _wrewinddir(_WDIR* dirp)
{
	errno = 0;

	if(!dirp) {
		errno = EFAULT;
		return;
	}

	if(dirp->dd_handle != -1) {
		_findclose (dirp->dd_handle);
	}

	dirp->dd_handle = -1;
	dirp->dd_index = 0;
}

long _wtelldir(_WDIR* dirp)
{
	errno = 0;

	if(!dirp) {
		errno = EFAULT;
		return -1;
	}

	return dirp->dd_index;
}

void _wseekdir(_WDIR* dirp, long lPos)
{
	errno = 0;

	if (!dirp) {
		errno = EFAULT;
		return;
	}

	if (lPos < -1) {
		errno = EINVAL;
		return;
	} else if(lPos == -1) {
		if(dirp->dd_handle != -1) {
			_findclose(dirp->dd_handle);
		}
		dirp->dd_handle = -1;
		dirp->dd_index = -1;
	} else {
		_wrewinddir(dirp);
		while((dirp->dd_index < lPos) && _wreaddir(dirp));
	}
}

