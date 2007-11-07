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

#include "MsExportGenerator.h"

#include <CoffObject.h>
#include <CoffFileHeader.h>
#include <CoffDirectiveSection.h>
#include <Log.h>
#include <Statistics.h>

namespace parity
{
	namespace tasks
	{
		MsExportGenerator::MsExportGenerator(binary::Symbol::SymbolVector& exp)
			: exports_(exp)
		{
			utils::Log::verbose("generating export directives for %d symbols...\n", exp.size());
		}

		void MsExportGenerator::doWork()
		{
			if(exports_.empty())
				return;

			//
			// generate a temporary object file and add it to the
			// objects list. The object contains only directives
			// which name all the exported symbols.
			//
			binary::Object obj;
			binary::FileHeader& hdr = obj.getHeader();
			binary::Section& rawSection = hdr.addSection(".drectve");

			//
			// WARNING: characteristics must be set before constructing
			// the DirectiveSection, since it checks for it.
			//
			rawSection.setCharacteristics(binary::Section::CharAlign1Bytes
				| binary::Section::CharLinkInfo | binary::Section::CharLinkRemove);

			binary::DirectiveSection drectveSection(rawSection);

			//
			// at the moment this is 32bit!
			//
			hdr.setMachine(binary::FileHeader::MachineI386);

			for(binary::Symbol::SymbolVector::const_iterator it = exports_.begin(); it != exports_.end(); ++it)
			{
				std::string drectve("/EXPORT:");
				std::string name = it->getName();

				drectve.append(name);

				if(!(it->getType() & binary::Symbol::ComplexFunction))
					drectve.append(",DATA");

				drectveSection.addDirective(drectve);
			}

			//
			// generate in memory file data.
			//
			utils::MemoryFile mem;
			obj.update(mem);

			//
			// create on disk file
			//
			utils::Path file = utils::Path::getTemporary(".parity.export.XXXXXX.o");
			mem.save(file);

			utils::Statistics::instance().addInformation("export-count", exports_.size());

			//
			// update context
			//
			utils::Context& ctx = utils::Context::getContext();
			ctx.getObjectsLibraries().push_back(file);
			ctx.getTemporaryFiles().push_back(file);
		}
	}
}

