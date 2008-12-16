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

#include "CoffImportDirectory.h"
#include "CoffFileHeader.h"

#include <Log.h>

#include <cstdio>

namespace parity
{
	namespace binary
	{
		struct NativeImportDirectory {
			unsigned int ILT;
			unsigned int TimeStamp;
			unsigned int ForwarderChain;
			unsigned int NameRVA;
			unsigned int IAT;
		};

		struct NativeILT {
			unsigned int Name : 31; // only 16 bits ordinal!
			unsigned int IsOrdinal : 1;
		};

		//
		// WARNING: this function operates inside one section only, and
		// produces incomplete results, if data is not conatined within
		// a single section (which should be the case with import data).
		//
		void ImportDirectory::calcImports(Image& img) {
			if(!img.getOptionalHeader().getImportTableRVA())
				return;

			NativeImportDirectory* ptr = reinterpret_cast<NativeImportDirectory*>(img.getHeader().getPointerFromRVA(img.getOptionalHeader().getImportTableRVA()));

			while(ptr && (ptr->ILT != 0 && ptr->NameRVA != 0)) {
				NativeImportEntry entry;

				entry.LibraryName = std::string(reinterpret_cast<char const*>(img.getHeader().getPointerFromRVA(ptr->NameRVA)));

				NativeILT* pi = reinterpret_cast<NativeILT*>(img.getHeader().getPointerFromRVA(ptr->ILT));
				
				while(pi && pi->Name != 0) {
					NativeImportSymbolEntry sym;

					sym.ByOrdinal = (pi->IsOrdinal != 0);
					sym.Ordinal = (pi->IsOrdinal ? (static_cast<unsigned int>(pi->Name & 0xFFFF)) : 0);
					
					if(!pi->IsOrdinal) {
						//
						// TODO: the "hint" field is simply skipped and ignored here. could
						// this be of any interest for us?
						//
						sym.Name = std::string(reinterpret_cast<char const*>(img.getHeader().getPointerFromRVA((pi->Name) + 2 /* this on is to skip the "hint" */)));
					}

					entry.ImportedSymbols.push_back(sym);

					++pi;
				}

				vector_.push_back(entry);

				++ptr;
			}
		}
	}
}

