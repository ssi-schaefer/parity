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

#ifndef __AUXSYMBOL_H__
#define __AUXSYMBOL_H__

#include <cstring>
#include <string>
#include <vector>

#include <GenericFile.h>
#include <MemoryFile.h>
#include <Pointer.h>
#include <Exception.h>

namespace parity
{
	namespace binary
	{
		class AuxSymbol {
		public:
			//
			// Contructors and Destructors
			//
			AuxSymbol(void* ptr) { ::memcpy(data_, ptr, 18); }
			AuxSymbol() { ::memset(data_, 0, 18); }
			
			//
			// Public Types
			//
			typedef std::vector<AuxSymbol> AuxSymbolVector;

			//
			// Complex Methods
			//
			void setData(void* data) { ::memcpy(data_, data, 18); }
			void* getData() { return data_; }

			static unsigned short getSizeInFile() { return 18; }
			void update(utils::MemoryFile& file) { file.append(data_, 18); }
		protected:
			//
			// Attributes
			// data_ is 18 since every symbol table entry is 18 bytes long
			//
			char data_[18];
		};
	}
}

#endif

