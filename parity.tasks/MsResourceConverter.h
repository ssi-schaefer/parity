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

#ifndef __MSRESOURCECONVERTERTASK_H__
#define __MSRESOURCECONVERTERTASK_H__

#include <Context.h>
#include <Path.h>
#include <Task.h>

#include <fstream>

namespace parity
{
	namespace tasks
	{
		class MsResourceConverter {
		public:
			struct Constructible {
				std::string sourceFile;
				std::string outputFile;
			};

			MsResourceConverter(Constructible & c)
				: sourceFile_(c.sourceFile)
				, outputFile_(c.outputFile)
			{}

			void doWork();

		private:
			void compileGeneric(const utils::Path& file, utils::Path executable, utils::Task::ArgumentVector& vec);

			void prepareGeneric(utils::Task::ArgumentVector& vec);
			void prepareGenericFile(utils::Path file, utils::Task::ArgumentVector& vec);

			std::string sourceFile_;
			std::string outputFile_;
		};
	}
}

#endif

