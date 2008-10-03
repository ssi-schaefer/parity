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

#include "MsSymbolTableGenerator.h"

#include <Log.h>
#include <Statistics.h>

#include <CoffObject.h>

namespace parity
{
	namespace tasks
	{
		MsSymbolTableGenerator::MsSymbolTableGenerator(parity::binary::Symbol::SymbolVector &sym)
			: symbols_(sym)
		{
			utils::Log::verbose("generating symbol table for %d symbols..\n", sym.size());
		}

		void MsSymbolTableGenerator::doWork()
		{
			if(symbols_.empty())
				return;

			//
			// Generate a temporary object file which contains:
			//  *) a table of all symbol names, with relocations to the real symbols
			//  *) a function with a well-known name that returns a pointer to that table.
			//

		}
	}
}

