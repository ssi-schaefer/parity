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

#include "CoffSection.h"
#include "CoffFileHeader.h"

#include <Log.h>

#define SIZEOF_RELOCATION 10

#define BUFFER_GROWSTEP			4096
#define BUFFER_ALLOCSIZE(s)		((((s)+BUFFER_GROWSTEP-1)/BUFFER_GROWSTEP)*BUFFER_GROWSTEP)

namespace parity
{
	namespace binary
	{
		Section::Section(const FileHeader* fh, int idx, void* ptr)
			: struct_(*(SecStruct*)ptr)
			, idx_(idx)
			, allocated_(0)
			, allocated_size_(0)
		{
			lookupRelocations(fh);
			calcName(fh);

			data_ = MAKEPTR(char*, fh->getBasePointer(), struct_.PointerToRawData);
		}

		Section::Section(FileHeader* fh, int idx, const std::string& name)
			: idx_(idx)
			, data_(0)
			, allocated_(0)
			, allocated_size_(0)
			, name_(name)
		{
			//
			// set name. this is the reverse to calcName.
			//
			::memset(&struct_, 0, sizeof(struct_));

			if(name.length() <= 8)
			{
				::strncpy((char*)struct_.Name, name.c_str(), 8);
			} else {
				::sprintf((char*)struct_.Name, "/%-7d", fh->addString(name));
			}
		}

		void Section::lookupRelocations(const FileHeader* fh)
		{
			if(struct_.NumberOfRelocations == 0 || fh->getCharacteristics() & FileHeader::CharRelocsStripped)
			{
				struct_.NumberOfRelocations = 0;
				return;
			}

			char* ptr = MAKEPTR(char*, fh->getBasePointer(), struct_.PointerToRelocations);

			unsigned int numRelocs = struct_.NumberOfRelocations;

			if(struct_.Characteristics & CharExtendedRelocations)
			{
				if(struct_.NumberOfRelocations != 0xFFFF)
					throw utils::Exception("relocation overflow, but less than 0xFFFF relocations in section!");

				Relocation first(ptr);
				numRelocs = first.getVirtualAddress();
			}

			for(int i = 0; i < struct_.NumberOfRelocations; ++i)
			{
				relocs_.push_back(Relocation(ptr));
				ptr += SIZEOF_RELOCATION;
			}
		}

		void Section::calcName(const FileHeader* fh)
		{
			if(struct_.Name[0] == '/')
			{
				name_ = fh->getStringFromOffset(atoi((const char*)&struct_.Name[1]));

				if(name_.empty()) {
					name_ = "<invalid>";
				}
			} else {
				bool sh = false;
				for(int i = 0; i < 8; ++i)
					if(struct_.Name[i] == '\0')
						sh = true;

				if(sh)
					name_ = std::string((char*)struct_.Name);
				else
					name_ = std::string((char*)struct_.Name, 8);
			}
		}

		//
		// x86 version
		//
		void Section::markRelocation(Symbol& sym, Relocation::i386TypeFlags type)
		{
			//
			// Maybe check if there is a relocation at this position already?
			//
			struct_.NumberOfRelocations++;

			Relocation rel;

			// relocation from current position to given symbol's index.
			rel.setSymbolTableIndex(sym.getIndex());
			rel.setType(type);
			rel.setVirtualAddress(struct_.SizeOfRawData);

			relocs_.push_back(rel);
		}

		void Section::markSymbol(Symbol& sym)
		{
			//
			// set the symbols Value field to the end of the current data
			//
			sym.setValue(struct_.SizeOfRawData);
			sym.setSectionNumber(idx_);
		}

		void Section::takeBufferControl()
		{
			//
			// allocated data for the first time
			//
			if(!allocated_ && struct_.SizeOfRawData > 0)
			{
				allocated_size_ = BUFFER_ALLOCSIZE(struct_.SizeOfRawData);
				allocated_ = malloc(allocated_size_);
				memcpy(allocated_, data_, struct_.SizeOfRawData);
			}
			data_ = allocated_;
		}

		void Section::addData(const void* data, size_t len)
		{
			size_t new_size = BUFFER_ALLOCSIZE(struct_.SizeOfRawData + len);

			if(!allocated_)
			{
				//
				// allocated data for the first time
				//
				if(data_)
					takeBufferControl();

				if(new_size > allocated_size_) {
					allocated_size_ = new_size;
					allocated_ = realloc(allocated_, allocated_size_);
				}
			} else {
				//
				// realloc data
				//
				if(new_size > allocated_size_) {
					allocated_size_ = new_size;
					allocated_ = realloc(allocated_, allocated_size_);
				}
			}
			data_ = allocated_;

			if(!allocated_)
				throw utils::Exception("cannot allocate memory for section data!");

			memcpy(MAKEPTR(void*, allocated_, struct_.SizeOfRawData), data, len);
			struct_.SizeOfRawData += len;
			struct_.VirtualSize += len;
		}

		void Section::insertData(FileHeader& hdr, const void *data, size_t len, size_t pos)
		{
			//
			// insertData makes it possible to insert data at any given position inside
			// a section. this function needs to take care of:
			//  *) alignment (pad at and of symbol that is modified)
			//   *) need to detect existing padding and shorten it if possible.
			//   *) this makes it unneccessary (if successfull) to patch things after the
			//      current symbol.
			//  *) patch all symbol's addresses after this position.
			//  *) patch all relocation with offsets higher than this position
			//

			//
			// It doesn't make sence to insert into data sections, since one can always
			// append data. Also one can't change the interpretation of data, so it
			// doesn't make sence to insert bytes there.
			//
			if(!(struct_.Characteristics & CharContentCode))
				throw utils::Exception("insertData can only operate on sections that contain code!");

			//
			// Assure padding is correct. This is required to detect padding
			// shrinking possibilities for functions at end of sections.
			//
			padSection();

			//
			// take control over the buffer!
			//
			takeBufferControl();

			//
			// Determine the symbol we are in and the symbol following it.
			//
			const Symbol* lower = 0;
			const Symbol* upper = 0;

			for(Symbol::IndexedSymbolMap::const_iterator it = hdr.getAllSymbols().begin(); it != hdr.getAllSymbols().end(); ++it)
			{
				if(it->second.getSectionNumber() == idx_)
				{
					if(it->second.getValue() <= pos) {
						if(!lower || (lower && it->second.getValue() >= lower->getValue()))
							lower = &it->second;
					} else if(it->second.getValue() >= pos) {
						if(!upper || (upper && it->second.getValue() <= upper->getValue()))
							upper = &it->second;
					}
				}
			}

			if(!lower)
				throw utils::Exception("not inside a symbol where insertion would be allowed");

			//
			// Step 2: Search from the following symbol, or from the end of the section
			// backwards for a valid padding (which is only 0xCC (int 3) multiple times)
			//
			size_t end = struct_.SizeOfRawData;

			if(upper)
				end = upper->getValue();

			size_t symEnd = end;
			size_t padEnd = 0;
			size_t padStart = 0;

			while(end >= pos)
			{
				--end;

				if(padEnd == 0 && (unsigned char)((char*)data_)[end] == 0xcc)
					padEnd = end;
				else if((unsigned char)((char*)data_)[end] != 0xcc) {
					padStart = end;
					break;
				}
			}

			size_t padCount = padEnd - padStart;

			if(padEnd != 0 && padCount > 1 && padCount > len)
			{
				//
				// Padding detected, more than one 0xCC found at end and padding
				// is long enough to fit new data inside and still have one 0xCC
				// standing at the end.
				//

				char* buffer = new char[(padStart - pos) + 1];
				::memcpy(buffer, MAKEPTR(void*, data_, pos), (padStart - pos) + 1);
				::memcpy(MAKEPTR(void*, data_, pos), data, len);
				::memcpy(MAKEPTR(void*, data_, pos + len), buffer, (padStart - pos) + 1);
				delete[] buffer;
				
				//
				// Update: from pos to padStart, no symbols affected, only relocs.
				//
				for(Relocation::RelocationVector::iterator it = relocs_.begin(); it != relocs_.end(); ++it)
				{
					if(it->getVirtualAddress() > pos && it->getVirtualAddress() < symEnd)
					{
						if(it->getVirtualAddress() + len >= symEnd)
							throw utils::Exception("patched relocation address would exceed symbols space!");

						it->setVirtualAddress(it->getVirtualAddress() + len);
					}
				}

				//
				// TODO: FIXXME: need to look at all jump directives that are in
				// the symbols code. Anyone that has no relocated address and jumps
				// over the inserted data (forward or backward) needs to be fixed.
				//
				utils::Log::warning("TODO: relative jumps not patched!\n");
			} else {
				//
				// No padding found, symbol will grow bigger than it was, and we
				// need to insert more bytes to keep up alignment of the section.
				// This also means recalculation of every symbol and relocation.
				//

				char* buffer = new char[(struct_.SizeOfRawData - pos) + 1];
				::memcpy(buffer, MAKEPTR(void*, data_, pos), (struct_.SizeOfRawData - pos));
				::memcpy(MAKEPTR(void*, data_, pos), data, len);
				::memcpy(MAKEPTR(void*, data_, pos + len), buffer, (symEnd - pos));
				size_t padPos = pos + len + (symEnd - pos);
				size_t padAlign = getAlignment();

				unsigned char padChar = 0xCC;
				size_t padOldPos = padPos;

				while(padPos % padAlign)
					::memcpy(MAKEPTR(void*, data_, padPos++), &padChar, 1);

				struct_.SizeOfRawData += (padPos - padOldPos) + len;
				struct_.VirtualSize += (padPos - padOldPos) + len;

				size_t new_size = BUFFER_ALLOCSIZE(struct_.SizeOfRawData);

				if(new_size > allocated_size_) {
					allocated_size_ = new_size;
					allocated_ = realloc(allocated_, allocated_size_);
				}

				data_ = allocated_;

				::memcpy(MAKEPTR(void*, data_, padPos), MAKEPTR(void*, buffer, (symEnd - pos)), (struct_.SizeOfRawData - (padPos - padOldPos) - len) - symEnd);

				delete[] buffer;
				
				//
				// Update: from pos to padStart, no symbols affected, only relocs (for locals)
				//
				for(Relocation::RelocationVector::iterator it = relocs_.begin(); it != relocs_.end(); ++it)
				{
					if(it->getVirtualAddress() > pos && it->getVirtualAddress() < symEnd)
					{
						if(it->getVirtualAddress() + len >= symEnd)
							throw utils::Exception("patched relocation address would exceed symbols space!");

						it->setVirtualAddress(it->getVirtualAddress() + len);
					}
				}

				//
				// Update: from pos to section end. then insert padding bytes and
				// update all following with len + padding.
				//
				for(Relocation::RelocationVector::iterator it = relocs_.begin(); it != relocs_.end(); ++it)
				{
					if(it->getVirtualAddress() >= symEnd)
					{
						it->setVirtualAddress(it->getVirtualAddress() + len + (padPos - padOldPos));
					}
				}

				for(Symbol::IndexedSymbolMap::iterator it = hdr.getAllSymbols().begin(); it != hdr.getAllSymbols().end(); ++it)
				{
					if(it->second.getSectionNumber() == idx_)
					{
						if(it->second.getValue() >= symEnd)
						{
							it->second.setValue(it->second.getValue() + len + (padPos - padOldPos));
						}
					}
				}

				//
				// TODO: FIXXME: need to look at all jump directives that are in
				// the sections code. Anyone that has no relocated address and jumps
				// over the inserted data (forward or backward) needs to be fixed.
				//
				utils::Log::warning("TODO: relative jumps not patched!\n");
			}
		}

		unsigned int Section::getAlignment()
		{
			unsigned short alignment = 16;
			unsigned int masked = struct_.Characteristics & 0x00F00000;

			if(masked == CharAlign1Bytes)
				alignment = 1;
			else if(masked == CharAlign2Bytes)
				alignment = 2;
			else if(masked == CharAlign4Bytes)
				alignment = 4;
			else if(masked == CharAlign8Bytes)
				alignment = 8;
			else if(masked == CharAlign16Bytes)
				alignment = 16;
			else if(masked == CharAlign32Bytes)
				alignment = 32;
			else if(masked == CharAlign64Bytes)
				alignment = 64;
			else if(masked == CharAlign128Bytes)
				alignment = 128;
			else if(masked == CharAlign256Bytes)
				alignment = 256;
			else if(masked == CharAlign512Bytes)
				alignment = 512;
			else if(masked == CharAlign1024Bytes)
				alignment = 1024;
			else if(masked == CharAlign2048Bytes)
				alignment = 2048;
			else if(masked == CharAlign4096Bytes)
				alignment = 4096;
			else if(masked == CharAlign8192Bytes)
				alignment = 8192;

			return alignment;
		}

		void Section::padSection(unsigned int alignment)
		{
			const unsigned char chInt3 = 0xCC;
			const unsigned char chZero = 0x00;

			while(struct_.SizeOfRawData % alignment)
			{
				if(struct_.Characteristics & Section::CharContentCode)
					addData((void*)&chInt3, 1);
				else
					addData((void*)&chZero, 1);
			}
		}

		void Section::padSection()
		{
			padSection(getAlignment());
		}

		void Section::updateDataAndRelocs(utils::MemoryFile& file)
		{
			if(struct_.SizeOfRawData > 0)
			{
				file.append(data_, struct_.SizeOfRawData);

				for(Relocation::RelocationVector::iterator it = relocs_.begin(); it != relocs_.end(); ++it)
				{
					it->update(file);
				}
			}
		}

	}
}

