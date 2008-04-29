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

#ifndef __VERSION_H__
#define __VERSION_H__

//
// This belongs into the parity project, but it is needed
// here, and it's impossible to create a dependency on the
// final executable here.
//

#ifdef HAVE_CONFIG_H
  #include <config.h>
#else
  #define PACKAGE "parity"
  #define PACKAGE_NAME "parity"
  #ifdef _WIN32
	#define PACKAGE_STRING "parity 1-win32"
    #define PACKAGE_VERSION " 1-win32"
  #else
    #define PACKAGE_STRING "parity 1-unknown"
    #define PACKAGE_VERSION " 1-unknown"
  #endif
#endif

#endif

