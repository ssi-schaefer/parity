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

#ifndef __COLLECTORSTUBS_H__
#define __COLLECTORSTUBS_H__

#include <Threading.h>

namespace parity
{
	namespace tasks
	{

		class TaskStubs {
		public:
			static unsigned int THREADINGAPI runDependencyTracking(void*);
			static unsigned int THREADINGAPI runCompiler(void*);
			static unsigned int THREADINGAPI runLinker(void*);
			static unsigned int THREADINGAPI runMsExportGenerator(void*);
			static unsigned int THREADINGAPI runMsStaticImportGenerator(void*);
			static unsigned int THREADINGAPI runMsLoaderGenerator(void*);
		};

	}
}

#endif

