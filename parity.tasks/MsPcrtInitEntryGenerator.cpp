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

#include "MsPcrtInitEntryGenerator.h"

#include <CoffObject.h>
#include <CoffFileHeader.h>
#include <CoffDirectiveSection.h>
#include <Log.h>
#include <Statistics.h>

//
// we lend the entry points from the loader generator :)
//
#include "MsLoaderGenerator.h"

namespace parity
{
	namespace tasks
	{
		MsPcrtInitEntryGenerator::MsPcrtInitEntryGenerator()
		{
			utils::Log::verbose("initialized PcrtInit entry point generator.\n");
		}

		static const unsigned char dataEmptyPtr[] = { 0x00, 0x00, 0x00, 0x00 };

		void MsPcrtInitEntryGenerator::doWork()
		{
			//
			// generate a temporary object file and add it to the
			// objects list. The object contains only the entrypoint
			// for calling PcrtInit and the real entry after that.
			//
			utils::Context& ctx = utils::Context::getContext();

			//
			// only do this when linking an executable, since all of this
			// is valid for the whole process!
			//
			if(ctx.getSharedLink())
				return;

			binary::Object obj;
			binary::FileHeader& hdr = obj.getHeader();
			hdr.setMachine(binary::FileHeader::TargetMachineType());
			
			binary::Section& sectText = hdr.addSection(".text");
			sectText.setCharacteristics( binary::Section::CharAlign16Bytes | binary::Section::CharMemoryExecute | binary::Section::CharMemoryRead | binary::Section::CharContentCode);
			
			std::string entry = ctx.getSharedLink() ? ctx.getSharedEntryPoint() : ctx.getEntryPoint();

			if(entry.empty())
				entry = ctx.calculateDefaultEntrypoint();

			//
			// in any case we need the loaders init routine.
			//
			binary::Symbol& symPcrtInit = hdr.addSymbol("_PcrtInit");
			symPcrtInit.setSectionNumber(0);
			symPcrtInit.setStorageClass(binary::Symbol::ClassExternal);
			symPcrtInit.setType(binary::Symbol::ComplexFunction);

			//
			// extern definition for real entry point.
			//
			binary::Symbol& symRealEntry = hdr.addSymbol("_" + entry);
			symRealEntry.setSectionNumber(0);
			symRealEntry.setStorageClass(binary::Symbol::ClassExternal);
			symRealEntry.setType(binary::Symbol::ComplexFunction);

			//
			// extern definition for PcrtCxxEhStartup.
			//
			binary::Symbol& symCxxEhEntry = hdr.addSymbol("_PcrtCxxEhStartup");
			symCxxEhEntry.setSectionNumber(0);
			symCxxEhEntry.setStorageClass(binary::Symbol::ClassExternal);
			symCxxEhEntry.setType(binary::Symbol::ComplexFunction);

			std::string ename = "ParityPcrtInitEntry";

			if(ctx.getSharedLink())
				ename.append("@12");

			binary::Symbol& symPcrtEntry = hdr.addSymbol("_" + ename);
			symPcrtEntry.setStorageClass(binary::Symbol::ClassExternal);
			symPcrtEntry.setType(binary::Symbol::ComplexFunction);

			sectText.markSymbol(symPcrtEntry);

			if(ctx.getSharedLink())
			{
				//
				// generate entry point that is able of handling arguments and stdcall.
				//

				sectText.addData(dataDllEntryPart1, sizeof(dataDllEntryPart1));
				sectText.markRelocation(symPcrtInit, binary::Relocation::i386Relative32);
				sectText.addData(dataEmptyPtr, sizeof(dataEmptyPtr));
				sectText.addData(dataDllEntryPart2, sizeof(dataDllEntryPart2));
				sectText.markRelocation(symRealEntry, binary::Relocation::i386Relative32);
				sectText.addData(dataEmptyPtr, sizeof(dataEmptyPtr));
				sectText.addData(dataDllEntryPart3, sizeof(dataDllEntryPart3));

				sectText.padSection();
			} else {
				//
				// generate entry point for void entry point without arguments
				//

				switch(ctx.getRuntime()) {
					case parity::utils::RuntimeDynamic:
					case parity::utils::RuntimeDynamicDebug:
						sectText.addData(dataExeEntryPart1, sizeof(dataExeEntryPart1));
						break;
					case parity::utils::RuntimeStatic:
					case parity::utils::RuntimeStaticDebug:
						sectText.addData(dataExeEntryPart1StaticCrt, sizeof(dataExeEntryPart1StaticCrt));
						break;
				}
				
				sectText.markRelocation(symPcrtInit, binary::Relocation::i386Relative32);
				sectText.addData(dataEmptyPtr, sizeof(dataEmptyPtr));

				// push real entry point
				sectText.addData(dataExeEntryPart2, sizeof(dataExeEntryPart2));
				sectText.markRelocation(symRealEntry, binary::Relocation::i386Direct32);
				sectText.addData(dataEmptyPtr, sizeof(dataEmptyPtr));
				
				//PcrtCxxEhStartup
				sectText.addData(dataExeEntryPart2a, sizeof(dataExeEntryPart2a));
				sectText.markRelocation(symCxxEhEntry, binary::Relocation::i386Relative32);
				sectText.addData(dataEmptyPtr, sizeof(dataEmptyPtr));

				sectText.addData(dataExeEntryPart3, sizeof(dataExeEntryPart3));

				sectText.padSection();
			}

			ctx.setEntryPoint(ename);
			ctx.setSharedEntryPoint(ename);

			//
			// generate in memory file data.
			//
			utils::MemoryFile mem;
			obj.update(mem);

			//
			// create on disk file
			//
			utils::Path file = utils::Path::getTemporary(".parity.pcrt.entry.XXXXXX.o");
			mem.save(file);

			//
			// update context
			//
			ctx.getObjectsLibraries().push_back(file);
			ctx.getTemporaryFiles().push_back(file);
		}
	}
}

