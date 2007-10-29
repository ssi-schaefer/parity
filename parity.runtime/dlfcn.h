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

#ifndef __PCRT_DLFCN_H__
#define __PCRT_DLFCN_H__

#include "internal/pcrt.h"

//
// the only flag supported by parity generated loaders
// is the RTLD_NOW flags, which forces resolving of
// lazy laoded symbols at first sight.
//

#define RTLD_LAZY	0x00
#define RTLD_NOW	0x01
#define RTLD_GLOBAL	0x00
#define RTLD_LOCAL	0x00

PCRT_BEGIN_C

int dlclose(void*);
char* dlerror();
void* dlopen(const char*, int);
void* dlsym(void*, const char*);

PCRT_END_C

#endif

