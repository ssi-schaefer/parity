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

#include "CoffFileHeader.h"
#include "CoffFile.h"
#include "CoffSectionAuxSymbol.h"

#include <ctime>

#include <Pointer.h>
#include <Exception.h>
#include <Log.h>

#define SIZEOF_FILE_HEADER 20
#define SIZEOF_SECTION_HEADER 40
#define SIZEOF_SYMBOL 18

namespace parity
{
	namespace binary
	{

		unsigned short FileHeader::TargetMachineType()
		{
			utils::Context& ctx = utils::Context::getContext();
			return ctx.getMachine();
		}

		FileHeader::FileHeader(const utils::GenericFile* file, void* ptr, bool isArchive)
			: start_(ptr), file_(file), hdr_(*reinterpret_cast<FHStruct*>(ptr)), stringtable_(), map_(), sections_()
			, nextStringOffset_(sizeof(unsigned int)), nextSectionIndex_(1), nextSymbolIndex_(0), isArchive_(isArchive)
		{
			lookupStringTab();
			lookupSections();
			lookupSymbols();
		}

		FileHeader::FileHeader() 
			: start_(0), file_(0), hdr_(), stringtable_(), map_(), sections_()
			, nextStringOffset_(sizeof(unsigned int)), nextSectionIndex_(1), nextSymbolIndex_(0) , isArchive_(false)
		{
			// should not be required anymore hopefully...
			//::memset(&hdr_, 0, sizeof(hdr_));
		}

		OptionalHeader FileHeader::getOptionalHeader() const
		{
			if(File::getType(file_) == File::TypeImage)
			{
				return OptionalHeader(MAKEPTR(void*, start_, sizeof(FHStruct)));
			} else {
				throw utils::Exception("Optional Header is only present in image files!");
			}
		}

		void FileHeader::lookupSections()
		{
			char * ptr = MAKEPTR(char*, (reinterpret_cast<char*>(start_) + sizeof(FHStruct)), hdr_.SizeOfOptionalHeader);

			for(unsigned int i = 0; i < hdr_.NumberOfSections; ++i)
			{
				sections_.insert(Section::IndexedSectionMap::value_type(i+1, Section(this, i+1, reinterpret_cast<void*>(ptr))));
				ptr += SIZEOF_SECTION_HEADER;

				nextSectionIndex_ = i+2;
			}
		}

		bool FileHeader::isValid() const
		{
			return isValidMachine(hdr_.Machine);
		}
		
		bool FileHeader::isValidMachine(unsigned short mach)
		{
			switch(mach)
			{
			case utils::MachineUnknown:
			case utils::MachineAMD64:
			case utils::MachineARM:
			case utils::MachineI386:
			case utils::MachineIA64:
				return true;
			default:
				return false;
			}
		}

		void FileHeader::lookupStringTab()
		{
			if(hdr_.PointerToSymbolTable != 0)
			{
				//
				// FIXXME: if an object is contained within a library, we need to use getStartPointer
				// instead of getBasePointer...
				//
				// Note: If we have to, why the hell is parity working, then?
				//
				char* ptr = MAKEPTR(char*, getBasePointer(), hdr_.PointerToSymbolTable + (hdr_.NumberOfSymbols * SIZEOF_SYMBOL));

				//
				// pay attention: the first 4 bytes specify the string table size!
				// but: every offset is given correctly paying attention to this.
				//
				unsigned int sz = *reinterpret_cast<unsigned int*>(ptr);

				if(sz > 4 /* sizeof(unsigned int) */)
				{
					const char * start = ptr + 4;
					for(unsigned int i = 4; i < sz; ++i)
					{
						if(ptr[i] == '\0' && &ptr[i] > start)
						{
							unsigned int off = start - ptr;
							std::string str(start);
							stringtable_[off] = str;

							nextStringOffset_ = off + str.length() + 1;
						
							start = &ptr[i] + 1;
						}
					}
				}
			}
		}

		const std::string& FileHeader::getStringFromOffset(unsigned int offset) const
		{
			OffsetStringMap::const_iterator pos = stringtable_.find(offset);

			if(pos == stringtable_.end())
				throw utils::Exception("string offset %d not found in file header!", offset);

			return pos->second;
		}

		Section& FileHeader::getSection(const std::string& name)
		{
			for(Section::IndexedSectionMap::iterator it = sections_.begin(); it != sections_.end(); ++it)
			{
				if(it->second.getName() == name)
					return it->second;
			}

			throw utils::Exception("cannot find section with name %s!", name.c_str());
		}

		Section& FileHeader::getSection(unsigned int idx)
		{
			if(idx > sections_.size() + 1)
				throw utils::Exception("section index out of bounds");

			return sections_[idx];
		}

		Section& FileHeader::getSectionForRVA(unsigned int rva)
		{
			for(Section::IndexedSectionMap::iterator it = sections_.begin(); it != sections_.end(); ++it)
			{
				unsigned int size = it->second.getVirtualSize();

				if(size == 0)
					size = it->second.getSizeOfRawData();

				if(rva >= it->second.getVirtualAddress() && rva < (it->second.getVirtualAddress() + size))
					return it->second;
			}

			throw utils::Exception("RVA out of bounds, cannot locate suitable section header!");
		}

		void* FileHeader::getPointerFromRVA(unsigned int rva)
		{
			try {
				if(getBasePointer() == 0)
					throw utils::Exception("not operating on an existing file!");

				Section sec = getSectionForRVA(rva);

				int delta = sec.getVirtualAddress() - sec.getPointerToRawData();
				return reinterpret_cast<void*>(reinterpret_cast<char*>(getBasePointer()) + rva - delta);
			} catch(const utils::Exception& e) {
				utils::Log::warning("cannot find pointer for given RVA (%p): %s\n", reinterpret_cast<void*>(rva), e.what());
				return 0;
			}
		}

		void* FileHeader::getPointerFromVA(unsigned int va)
		{
			return getPointerFromRVA(va - getOptionalHeader().getImageBase());
		}

		void FileHeader::lookupSymbols()
		{
			if(hdr_.PointerToSymbolTable == 0)
				return;

			char* ptr = MAKEPTR(char *, getBasePointer(), hdr_.PointerToSymbolTable);

			for(unsigned int i = 0; i < hdr_.NumberOfSymbols; i++)
			{
				Symbol sym(this, i, reinterpret_cast<void*>(ptr));
				ptr += SIZEOF_SYMBOL;

				int num = sym.getNumberOfAuxSymbols();

				if(num > 0)
				{
					for(int y = 0; y < num; ++y, ++i)
					{
						sym.addAuxSymbol(ptr);
						ptr += SIZEOF_SYMBOL;
					}
				}

				map_.insert(Symbol::IndexedSymbolMap::value_type(sym.getIndex(), sym));
				nextSymbolIndex_ = i+1;
			}
		}

		unsigned int FileHeader::addString(const std::string& str)
		{
			//
			// zero is not a valid offset, since the first 4 bytes are the
			// size of the string table
			//
			if(nextStringOffset_ == 0)
				nextStringOffset_ += sizeof(unsigned int);

			unsigned int ret = nextStringOffset_;
			nextStringOffset_ += str.length() + 1; // trailing 0

			stringtable_.insert(OffsetStringMap::value_type(ret, str));
			return ret;
		}

		Section& FileHeader::addSection(const std::string& name)
		{
			Section sect(this, nextSectionIndex_++, name);

			//
			// Add a Symbol table entry for this section!
			//
			Symbol& entry = addSymbol(name);

			//
			// The aux symbol is updated before writing to the file!
			// We need the intermediate variable for gcc 3.3 to be
			// able to pass as reference
			//
			AuxSymbol aux;
			addAuxSymbol(entry, aux);

			entry.setSectionNumber(static_cast<short>(sect.getIndex()));
			entry.setStorageClass(Symbol::ClassStatic);

			sections_.insert(Section::IndexedSectionMap::value_type(sect.getIndex(), sect));
			return sections_[sect.getIndex()];
		}

		Symbol& FileHeader::addSymbol(const std::string& name)
		{
			Symbol sym(this, nextSymbolIndex_++, name);

			//
			// Return value stored in map, so we have a reference that we
			// can find again.
			//
			map_.insert(Symbol::IndexedSymbolMap::value_type(sym.getIndex(), sym));
			return map_[sym.getIndex()];
		}

		void FileHeader::addAuxSymbol(Symbol& sym, AuxSymbol& which)
		{
			++nextSymbolIndex_;
			sym.addAuxSymbol(which);
		}

		void FileHeader::update(utils::MemoryFile& file, OptionalHeader* opt)
		{
			if(!file.isWriteable())
				throw utils::Exception("given file is not writeable for update!");

			//
			// order of writing to file/memory
			//
			// 1) File Header
			// 2) Section Headers
			// 3) Section Datas and Relocations
			// 4) Symbol Table
			//  a) Update Section Symbol's AuxSymbols to match the corresponding Section
			// 5) String Table
			//

			//
			// calculate offsets of different things we need
			//
			unsigned int offsetToSectionTable = file.getSize() + FileHeader::getSizeInFile();
			unsigned int offsetToSymbolTable = offsetToSectionTable;
			unsigned int sectionAlignment = (opt ? opt->getFileAlignment() : 0);
			unsigned int virtualAlignment = (opt ? opt->getSectionAlignment() : 0);

			//
			// first section's virtual address doesn't start at zero.
			//
			unsigned int currentVirtualAddress = virtualAlignment;

			for(Section::IndexedSectionMap::iterator it = sections_.begin(); it != sections_.end(); ++it)
			{
				//
				// either pad to file alignment, or, if object, pad to
				// sections default alignment.
				//
				if(sectionAlignment > 0)
					it->second.padSection(sectionAlignment);
				else
					it->second.padSection();

				//
				// calculate the Virtual Address for the next section.
				//
				if(virtualAlignment > 0)
				{
					it->second.setVirtualAddress(currentVirtualAddress);
					
					unsigned int temp = 0;

					while(it->second.getVirtualSize() > temp)
						temp += virtualAlignment;

					currentVirtualAddress += temp;
				}

				//
				// Add up size of section including header, data and relocations.
				//
				offsetToSymbolTable += Section::getSizeInFile()
					+ it->second.getSizeOfRawData()
					+ (hdr_.Characteristics & CharRelocsStripped ? 0 : (Relocation::getSizeInFile() * it->second.getNumberOfRelocations()));

				//
				// As a side effect of calulation, aux symbols for sections are updated for correctness.
				//
				for(Symbol::IndexedSymbolMap::iterator sym = map_.begin(); sym != map_.end(); ++sym)
				{
					if(sym->second.getSectionNumber() == static_cast<short>(it->first)
						&& sym->second.getStorageClass() == Symbol::ClassStatic
						&& sym->second.getName() == it->second.getName())
					{
						//
						// found section's' symbol
						//
						if(sym->second.getAuxSymbols().size() != 1)
							throw utils::Exception("missing auxiliary symbol describing section!");

						AuxSymbol& aux = sym->second.getAuxSymbols().front();
						SectionAuxSymbol* ptr = reinterpret_cast<SectionAuxSymbol*>(&aux);

						if(it->second.getSizeOfRawData() > it->second.getVirtualSize())
							ptr->setLength(it->second.getSizeOfRawData());
						else
							ptr->setLength(it->second.getVirtualSize());

						ptr->setNumberOfRelocations(it->second.getNumberOfRelocations());

						//
						// TODO: comdats, etc. (see PE/COFF specs v8)
						// is there any more to do here?
						//

						break;
					}						
				}
			}

			//
			// if sectionAlignment not zero, align symbol table start
			//
			if(sectionAlignment != 0)
			{
				while(offsetToSymbolTable % sectionAlignment)
					++offsetToSymbolTable;
			}

			//
			// Update the FileHeader with some fresh values.
			//

			hdr_.NumberOfSections = static_cast<unsigned short>(sections_.size());
			hdr_.NumberOfSymbols = nextSymbolIndex_;
			hdr_.TimeDateStamp = static_cast<unsigned int>(::time(0));

			if(opt)
				hdr_.SizeOfOptionalHeader = OptionalHeader::getSizeInFile();
			else
				hdr_.SizeOfOptionalHeader = 0;

			if(hdr_.NumberOfSymbols)
				hdr_.PointerToSymbolTable = offsetToSymbolTable;
			else
				hdr_.PointerToSymbolTable = 0;

			//
			// Ready for writing...
			//
			file.append(&hdr_, getSizeInFile());

			//
			// write possible optional header
			//
			if(opt)
			{
				opt->update(file);
			}

			//
			// offsetToCurrentSectionData points to after the section table
			//
			unsigned int offsetToCurrentSectionData = offsetToSectionTable + (Section::getSizeInFile() * sections_.size());

			if(sectionAlignment > 0)
			{
				//
				// align sections properly
				//
				while(offsetToCurrentSectionData % sectionAlignment)
					++offsetToCurrentSectionData;
			}

			for(Section::IndexedSectionMap::iterator it = sections_.begin(); it != sections_.end(); ++it)
			{
				//
				// Update some values from here, since only here they are known.
				//
				if(it->second.getSizeOfRawData() > 0)
				{
					it->second.setPointerToRawData(offsetToCurrentSectionData);

					if(it->second.getNumberOfRelocations() > 0)
					{
						it->second.setPointerToRelocations(offsetToCurrentSectionData + it->second.getSizeOfRawData());
					}

					offsetToCurrentSectionData += it->second.getSizeOfRawData() + (/* hdr_.Characteristics & CharRelocsStripped ? 0 : */(Relocation::getSizeInFile() * it->second.getNumberOfRelocations()));
				}

				it->second.updateHeader(file);
			}

			if(sectionAlignment > 0)
			{
				//
				// align first section to SectionAlignment
				//
				const unsigned char nullData[] = { '\0' };
				while(file.getSize() % sectionAlignment)
					file.append(nullData, 1);
			}

			for(Section::IndexedSectionMap::iterator it = sections_.begin(); it != sections_.end(); ++it)
			{
				it->second.updateDataAndRelocs(file);
			}

			//
			// Now the Symbol Table
			//
			for(Symbol::IndexedSymbolMap::iterator it = map_.begin(); it != map_.end(); ++it)
			{
				it->second.update(file);
			}

			//
			// last is the string table
			// first calculate the size of the string table
			//
			unsigned int sizeOfStringTable = 4;
			for(OffsetStringMap::iterator it = stringtable_.begin(); it != stringtable_.end(); ++it)
			{
				// + 1 for trailing zero
				sizeOfStringTable += it->second.length() + 1;
			}

			file.append(&sizeOfStringTable, sizeof(unsigned int));
			
			for(OffsetStringMap::iterator it = stringtable_.begin(); it != stringtable_.end(); ++it)
			{
				// + 1 for trailing zero
				file.append(it->second.c_str(), it->second.length() + 1);
			}

			//
			// All done ;o)
			//
		}

	}
}

