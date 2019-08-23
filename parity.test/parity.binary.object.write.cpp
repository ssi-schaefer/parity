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

#include "TestSuite.h"

#include <MemoryFile.h>
#include <Exception.h>
#include <CoffFile.h>
#include <CoffFileHeader.h>
#include <CoffImage.h>
#include <CoffObject.h>
#include <CoffArchive.h>
#include <CoffDirectiveSection.h>

#include <Timing.h>

#include <errno.h>

namespace utils = parity::utils;
using namespace parity::binary;

namespace parity
{
	namespace testing
	{
		extern void testFileHeader(binary::FileHeader& hdr);

		bool TestSuite::testParityBinaryObjectWrite()
		{
			try {
				utils::MemoryFile file;

				utils::Log::verbose("creating file header...\n");

				Object obj;
				FileHeader& hdr = obj.getHeader();
				hdr.setMachine(FileHeader::TargetMachineType());

				utils::Log::verbose("creating sections...\n");
				
				Section& dataSection = hdr.addSection(".data");
				Section& textSection = hdr.addSection(".text");

				dataSection.setCharacteristics(Section::CharAlign4Bytes 
					| Section::CharContentInitData | Section::CharMemoryRead | Section::CharMemoryWrite);
				textSection.setCharacteristics(Section::CharContentCode 
					| Section::CharMemoryExecute | Section::CharMemoryRead | Section::CharAlign16Bytes);

				utils::Log::verbose("creating symbols...\n");

				Symbol& dataSym1 = hdr.addSymbol("_datasym1");
				Symbol& dataSym2 = hdr.addSymbol("_datasym2");
				Symbol& ptrSym1 = hdr.addSymbol("_ptrsym1");
				Symbol& extSym1 = hdr.addSymbol("_markus_var");
				Symbol& txtSym1 = hdr.addSymbol("_func");
				Symbol& txtSym2 = hdr.addSymbol("_func2");

				extSym1.setSectionNumber(0);

				int dataSym1Data = 0xBAADF00D;
				int dataSym2Data = 0xDEADBEEF;
				int ptrSym1Data  = 0x00000000;

				utils::Log::verbose("marking and adding data for datasym1.\n");

				dataSection.markSymbol(dataSym1);
				dataSection.addData(&dataSym1Data, 4);

				utils::Log::verbose("marking and adding data for datasym2.\n");

				dataSection.markSymbol(dataSym2);
				dataSection.addData(&dataSym2Data, 4);

				utils::Log::verbose("marking and adding relocation and data for ptrsym1.\n");

				dataSection.markSymbol(ptrSym1);
				dataSection.markRelocation(dataSym1, Relocation::i386Direct32);
				dataSection.addData(&ptrSym1Data, 4);

				utils::Log::verbose("marking and adding data for txtSym1.\n");

				const unsigned char txtSym1Data1[] = { 0x55, 0x8B, 0xEC, 0xE8 };
				const unsigned char txtSym1Data2[] = { 0x00, 0x00, 0x00, 0x00, 0x85, 0xC0, 0x75, 0x06, 0x0F, 0x84, 0x05, 0x00, 0x00, 0x00, 0xE8 };
				const unsigned char txtSym1Data3[] = { 0x00, 0x00, 0x00, 0x00, 0x5D, 0xC3 };

				textSection.markSymbol(txtSym1);
				textSection.addData(reinterpret_cast<const char*>(txtSym1Data1), sizeof(txtSym1Data1));
				textSection.markRelocation(txtSym2, Relocation::i386Direct32);
				textSection.addData(reinterpret_cast<const char*>(txtSym1Data2), sizeof(txtSym1Data2));
				textSection.markRelocation(txtSym2, Relocation::i386Direct32);
				textSection.addData(reinterpret_cast<const char*>(txtSym1Data3), sizeof(txtSym1Data3));
				textSection.padSection();

				textSection.markSymbol(txtSym2);
				textSection.addData(reinterpret_cast<const char*>(txtSym1Data1), 4);
				textSection.markRelocation(extSym1, Relocation::i386Direct32);
				textSection.addData(reinterpret_cast<const char*>(txtSym1Data2), 6);
				textSection.padSection();

				utils::Log::verbose("setting storage class for symbols.\n");

				dataSym1.setStorageClass(Symbol::ClassExternal);
				dataSym2.setStorageClass(Symbol::ClassExternal);
				ptrSym1.setStorageClass(Symbol::ClassExternal);
				extSym1.setStorageClass(Symbol::ClassExternal);

				txtSym1.setStorageClass(Symbol::ClassExternal);
				txtSym1.setType(Symbol::ComplexFunction);
				txtSym2.setStorageClass(Symbol::ClassExternal);
				txtSym2.setType(Symbol::ComplexFunction);

				utils::Log::verbose("updating memory from file information.\n");

				obj.update(file);

				utils::Path pth(".test.o");

				file.save(pth);
				pth.mode(0777);

				utils::Log::verbose("testing in memory...\n");
				testFileHeader(hdr);

				utils::Log::verbose("testing from file...\n");
				utils::MappedFile f(pth, utils::ModeRead);

				if(File::getType(&f) != File::TypeObject)
				{
					utils::Log::error("file type is not an object!\n");
					return false;
				}

				Object obj2(&f);
				testFileHeader(obj.getHeader());

				utils::Path tmp2(".test.edited.o");

				if(pth.exists())
				{
					utils::Log::verbose("trying to do a parsed copy with insertion of %s.\n", pth.get().c_str());
					FileHeader& tmph = obj2.getHeader();

					Section& textSection = tmph.getSection(".text");

					unsigned char txtInsertData[] = { 0xE8, 0x00, 0x01, 0x02, 0x03 };
					textSection.insert(tmph, txtInsertData, sizeof(txtInsertData), 18);
					textSection.insert(tmph, txtInsertData, sizeof(txtInsertData), 18);
					textSection.insert(tmph, txtInsertData, sizeof(txtInsertData), 18);
					textSection.insert(tmph, txtInsertData, sizeof(txtInsertData), 18);

					utils::MemoryFile tmpm;
					obj2.update(tmpm);

					tmpm.save(tmp2);
				}

				tmp2.mode(0777);

				f.close();

				if(!pth.remove())
					utils::Log::warning("cannot remove %s (%s)\n", pth.get().c_str(), strerror(errno));
				if(!tmp2.remove())
					utils::Log::warning("cannot remove %s (%s)\n", tmp2.get().c_str(), strerror(errno));

				return true;
			} catch(const utils::Exception& e)
			{
				utils::Log::warning("catched: %s\n", e.what());
			}

			return false;
		}
	}
}

