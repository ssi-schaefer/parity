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

#include "CoffSymbol.h"
#include "CoffFileHeader.h"
#include "CoffSection.h"

namespace parity
{
	namespace binary
	{
		Symbol::Symbol(FileHeader* fh, long idx, const std::string& name)
			: idx_(idx)
			, name_(name)
		{
			::memset(&struct_, 0, sizeof(struct_));

			if(name.length() <= 8)
			{
				::strncpy(struct_.N.ShortName, name.c_str(), 8);
			} else {
				struct_.N.N.Zeros = 0;
				struct_.N.N.Offset = fh->addString(name);
			}
		}

		void Symbol::calcName(const FileHeader* fh)
		{
			if(struct_.N.N.Zeros == 0)
			{
				name_ = fh->getStringFromOffset(struct_.N.N.Offset);

				if(name_.empty())
					name_ = "<invalid>";
			} else {
				size_t len = ::strlen(struct_.N.ShortName);
				name_ = std::string(struct_.N.ShortName, (len < 8 ? len : 8));
			}
		}

		void Symbol::update(utils::MemoryFile& file) 
		{
			struct_.NumberOfAuxSymbols = aux_.size();
			file.append(&struct_, getSizeInFile()); 

			for(AuxSymbol::AuxSymbolVector::iterator it = aux_.begin(); it != aux_.end(); ++it)
			{
				it->update(file);
			}
		}

	}
}

