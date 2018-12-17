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

#ifndef __PRPRROCESSOR_H__
#define __PREPROCESSOR_H__

#include "Argument.h"

#include <Context.h>
#include <Exception.h>

namespace parity
{
	namespace options
	{
		bool setKeepComments(const char* option, const char* argument, bool& used);
		bool setNoStdIncludes(const char* option, const char* argument, bool& used);
		bool addDefine(const char* option, const char* argument, bool& used);
		bool removeDefine(const char* option, const char* argument, bool& used);
		bool setPreprocess(const char* option, const char* argument, bool& used);
		bool addIncludePath(const char* option, const char* argument, bool& used);
		bool addIncludeFile(const char* option, const char* argument, bool& used);
		bool setDependencyTracking(const char* option, const char* argument, bool& used);
		bool setDependencyPhony(const char* option, const char* argument, bool& used);
		bool setDependencyGenerated(const char* option, const char* argument, bool& used);
		bool setDependencyOutput(const char* option, const char* argument, bool& used);
		bool setDependencyTarget(const char* option, const char* argument, bool& used);
	}
}

#endif

