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

#ifndef __MISC_H__
#define __MISC_H__

#include "Argument.h"

#include <Context.h>
#include <Exception.h>

namespace parity
{
	namespace options
	{
		bool setUseSSE(const char* option, const char* argument, bool& used);
		bool setUseSSE2(const char* option, const char* argument, bool& used);
		bool setForScope(const char* option, const char* argument, bool& used);
		bool setMsExtensions(const char* option, const char* argument, bool& used);
		bool setWarningLevel(const char* option, const char* argument, bool& used);
		bool setWarningAsError(const char* option, const char* argument, bool& used);
		bool setAnsiMode(const char* option, const char* argument, bool& used);
		bool setOutputFile(const char* option, const char* argument, bool& used);
		bool setDebugable(const char* option, const char* argument, bool& used);
	}
}

#endif

