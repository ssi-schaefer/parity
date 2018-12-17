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
#include <CoffDirectiveSection.h>

namespace parity
{
	namespace tasks
	{
		MsSymbolTableGenerator::MsSymbolTableGenerator(parity::binary::Symbol::SymbolVector &sym)
			: symbols_(sym)
		{
			utils::Log::verbose("generating symbol table for %ld symbols..\n", sym.size());
		}

		static const unsigned char dataEmptyPtr[] = { 0x00, 0x00, 0x00, 0x00 };

		void MsSymbolTableGenerator::doWork()
		{
			utils::Context& ctx = utils::Context::getContext();

			if(symbols_.empty())
				return;

			//
			// Generate a temporary object file which contains:
			//  *) a table of all symbol names, with relocations to the real symbols
			//  *) a function with a well-known name that returns a pointer to that table.
			//
			binary::Object obj;
			binary::FileHeader& hdr = obj.getHeader();

			hdr.setMachine(binary::FileHeader::MachineI386);

			binary::Section& sectText = hdr.addSection(".text");
			binary::Section& sectRData = hdr.addSection(".rdata");
			binary::Section& rawSection = hdr.addSection(".drectve");

			sectText.setCharacteristics( binary::Section::CharAlign16Bytes | binary::Section::CharMemoryExecute | binary::Section::CharMemoryRead | binary::Section::CharContentCode);
			sectRData.setCharacteristics(binary::Section::CharAlign4Bytes  | binary::Section::CharMemoryRead    | binary::Section::CharContentInitData);
			rawSection.setCharacteristics(binary::Section::CharAlign1Bytes | binary::Section::CharLinkInfo | binary::Section::CharLinkRemove);

			binary::DirectiveSection drectveSection(rawSection);

			std::map<std::string, int> symIndexMap;

			for(binary::Symbol::SymbolVector::const_iterator it = symbols_.begin(); it != symbols_.end(); ++it)
			{
				//
				// generate name symbols in an extra pass, since they end up in
				// the same section as the table.
				//
				binary::Symbol& symName = hdr.addSymbol("$NAME_" + it->getName());
				sectRData.markSymbol(symName);
				symName.setStorageClass(binary::Symbol::ClassStatic);
				sectRData.addData(it->getName().c_str(), it->getName().length() + 1);

				symIndexMap["$NAME_" + it->getName()] = symName.getIndex();

				//
				// keep up section alignment (most probably 4 bytes).
				//
				sectRData.padSection();

				binary::Symbol& extSym = hdr.addSymbol(it->getName());
				extSym.setStorageClass(binary::Symbol::ClassExternal);
				extSym.setSectionNumber(0);
				extSym.setType(binary::Symbol::ComplexPointer);

				symIndexMap[it->getName()] = extSym.getIndex();
			}

			//binary::Symbol& symtab = hdr.addSymbol("$START_ParitySymbolTable");
			binary::Symbol& symtab = hdr.addSymbol("_ParityGeneratedSymbolTable");
			sectRData.markSymbol(symtab);
			//symtab.setStorageClass(binary::Symbol::ClassStatic);
			symtab.setStorageClass(binary::Symbol::ClassExternal);

			for(binary::Symbol::SymbolVector::const_iterator it = symbols_.begin(); it != symbols_.end(); ++it)
			{
				//
				// format for table is a struct like this: (keep in sync with parity.runtime/diagnose.h)
				// struct symtab_entry {
				//   void* addr;
				//   char const* name;
				// }
				//
				sectRData.markRelocation(hdr.getAllSymbols()[symIndexMap[it->getName()]], binary::Relocation::i386Direct32);
				sectRData.addData(dataEmptyPtr, sizeof(dataEmptyPtr));
				sectRData.markRelocation(hdr.getAllSymbols()[symIndexMap["$NAME_" + it->getName()]], binary::Relocation::i386Direct32);
				sectRData.addData(dataEmptyPtr, sizeof(dataEmptyPtr));
			}

			//
			// NULL-terminate the table.
			//
			sectRData.addData(dataEmptyPtr, sizeof(dataEmptyPtr));
			sectRData.addData(dataEmptyPtr, sizeof(dataEmptyPtr));

			binary::Symbol& symtab_end = hdr.addSymbol("$END_ParitySymbolTable");
			sectRData.markSymbol(symtab_end);
			symtab_end.setStorageClass(binary::Symbol::ClassStatic);

			drectveSection.addDirective("/EXPORT:_ParityGeneratedSymbolTable,DATA");

			//
			// now save the object file to disk.
			//
			utils::MemoryFile mem;
			obj.update(mem);

			utils::Path pth = utils::Path::getTemporary(".parity.symtab.XXXXXX.o");

			mem.save(pth);
			ctx.getObjectsLibraries().push_back(pth);
			ctx.getTemporaryFiles().push_back(pth);
		}
	}
}
