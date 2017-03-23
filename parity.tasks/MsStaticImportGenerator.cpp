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

#include "MsStaticImportGenerator.h"

#include <Log.h>
#include <Statistics.h>

#include <CoffObject.h>

namespace parity
{
	namespace tasks
	{
		MsStaticImportGenerator::MsStaticImportGenerator(parity::binary::Symbol::SymbolVector &imp)
			: imports_(imp)
		{
			utils::Log::verbose("generating static imports for %ld symbols..\n", imp.size());
		}

		void MsStaticImportGenerator::doWork()
		{
			if(imports_.empty())
				return;

			//
			// Generate a temporary object file which contains:
			//  *) a UNDEF symbol table entry for each real static symbol
			//  *) a real symbol pointing to (relocated to) the real UNDEF symbol.
			//
			binary::Object obj;
			binary::FileHeader& hdr = obj.getHeader();
			binary::Section& dataSection = hdr.addSection(".data");

			//
			// at the moment this is 32bit!
			//
			hdr.setMachine(binary::FileHeader::MachineI386);

			const unsigned char dummyData[] = { 0x00, 0x00, 0x00, 0x00 };

			for(binary::Symbol::SymbolVector::const_iterator it = imports_.begin(); it != imports_.end(); ++it)
			{
				std::string importName = "__imp_" + it->getName();
				binary::Symbol& ext = hdr.addSymbol(it->getName());
				binary::Symbol& imp = hdr.addSymbol(importName);
				
				dataSection.markSymbol(imp);
				dataSection.markRelocation(ext, binary::Relocation::i386Direct32);
				dataSection.addData(dummyData, 4);

				ext.setStorageClass(binary::Symbol::ClassExternal);
				imp.setStorageClass(binary::Symbol::ClassExternal);
				ext.setType(it->getType());
			}

			//
			// generate in memory file data.
			//
			utils::MemoryFile mem;
			obj.update(mem);

			//
			// create on disk file
			//
			utils::Path file = utils::Path::getTemporary(".parity.static.import.XXXXXX.o");
			mem.save(file);

			utils::Statistics::instance().addInformation("static-import-count", imports_.size());

			//
			// update context
			//
			utils::Context& ctx = utils::Context::getContext();
			ctx.getObjectsLibraries().push_back(file);
			ctx.getTemporaryFiles().push_back(file);
		}
	}
}

