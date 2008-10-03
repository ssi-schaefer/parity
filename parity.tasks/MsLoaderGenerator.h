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

#ifndef __LOADERGENERATOR_H__
#define __LOADERGENERATOR_H__

#include <map>
#include <string>

#include <Path.h>

#include <CoffImport.h>
#include <CoffSymbol.h>
#include "BinaryGatherer.h"

namespace parity
{
	namespace tasks
	{
		class MsLoaderGenerator {
		public:
			MsLoaderGenerator(const tasks::BinaryGatherer::ImportHybridityMap& imports);
			void doWork();
		private:
			const tasks::BinaryGatherer::ImportHybridityMap& imports_;

			typedef struct {
				std::string name;
				bool hybrid;
				utils::Path path;

				binary::Import::ImportVector imports;

				unsigned int recordIndex;
				unsigned int nameIndex;
				unsigned int tableIndex;
			} LoaderWorkingItem;

			typedef std::map<std::string, LoaderWorkingItem> LoaderWorkingMap;
		};
	}
}

#endif

