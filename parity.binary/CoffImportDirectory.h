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

#ifndef __IMPORTDIRECTORY_H__
#define __IMPORTDIRECTORY_H__

#include <cstring>
#include <string>
#include <vector>

#include <Pointer.h>
#include <Exception.h>

#include "CoffRelocation.h"
#include "CoffSymbol.h"
#include "CoffSection.h"
#include "CoffImage.h"

namespace parity
{
	namespace binary
	{
		class ImportDirectory {
		public:
			//
			// Contructors and Destructors
			//
			ImportDirectory(Image& file) : vector_() { calcImports(file); }
			
			//
			// Complex Methods
			//
			struct NativeImportSymbolEntry {
				NativeImportSymbolEntry() : Name(), Ordinal(0), ByOrdinal(false) {}

				std::string Name;
				unsigned int Ordinal;
				bool ByOrdinal;
			};

			typedef std::vector<NativeImportSymbolEntry> NativeImportSymbolVector;

			struct NativeImportEntry {
				NativeImportEntry() : LibraryName(), ImportedSymbols() {}

				std::string LibraryName;
				NativeImportSymbolVector ImportedSymbols;
			};

			typedef std::vector<NativeImportEntry> NativeImportVector;

			NativeImportVector const& getNativeImports() const { return vector_; }
		private:
			void calcImports(Image& img);

			//
			// Attributes
			//
			NativeImportVector vector_;
		};
	}
}

#endif

