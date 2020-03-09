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
#include "CoffDecoder.h"

#include <Log.h>

#include <climits>

#define SIZEOF_RELOCATION 10

#define BUFFER_GROWSTEP			4096
#define BUFFER_ALLOCSIZE(s)		((((s)+BUFFER_GROWSTEP-1)/BUFFER_GROWSTEP)*BUFFER_GROWSTEP)

namespace parity
{
	namespace binary
	{
		Section::Section(const FileHeader* fh, int idx, void* ptr)
			: struct_(*reinterpret_cast<SecStruct*>(ptr))
			, idx_(idx)
			, data_(0)
			, allocated_(0)
			, allocated_size_(0)
			, relocs_()
			, name_()
		{
			lookupRelocations(fh);
			calcName(fh);

			data_ = MAKEPTR(char*, fh->getBasePointer(), struct_.PointerToRawData);
		}

		Section::Section(FileHeader* fh, int idx, const std::string& name)
			: struct_()
			, idx_(idx)
			, data_(0)
			, allocated_(0)
			, allocated_size_(0)
			, relocs_()
			, name_(name)
		{
			//
			// set name. this is the reverse to calcName.
			//
			// should not be required anymore hopefully...
			//::memset(&struct_, 0, sizeof(struct_));

			char buf[22] = {};
			if(name.length() <= 8)
			{
				::strncpy(buf, name.c_str(), 8);
			} else {
				::snprintf(buf, sizeof(buf), "/%-7d", fh->addString(name));
			}
			memcpy(struct_.Name, buf, 8);
		}

		Section::Section(Section const& rhs)
			: struct_(rhs.struct_)
			, idx_(rhs.idx_)
			, data_(0)
			, allocated_(0)
			, allocated_size_(rhs.allocated_size_)
			, relocs_(rhs.relocs_)
			, name_(rhs.name_)
		{
			if(!allocated_size_) {
				data_ = rhs.data_;
			} else {
				allocated_ = malloc(allocated_size_);
				memcpy(allocated_, rhs.allocated_, allocated_size_);
				data_ = allocated_;
			}
		}

		Section& Section::operator=(Section const& rhs)
		{
			struct_ = rhs.struct_;
			idx_ = rhs.idx_;
			allocated_ = 0;
			allocated_size_ = rhs.allocated_size_;
			relocs_ = rhs.relocs_;
			name_ = rhs.name_;

			if(!allocated_size_) {
				data_ = rhs.data_;
			} else {
				allocated_ = malloc(allocated_size_);
				memcpy(allocated_, rhs.allocated_, allocated_size_);
				data_ = allocated_;
			}

			return *this;
		}

		Section::~Section()
		{
			if(allocated_size_ > 0 && allocated_) {
				free(allocated_);
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

			if(struct_.Characteristics & CharExtendedRelocations)
			{
				if(struct_.NumberOfRelocations != 0xFFFF)
					throw utils::Exception("relocation overflow, but less than 0xFFFF relocations in section!");

				Relocation first(ptr);
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
				name_ = fh->getStringFromOffset(atoi(reinterpret_cast<const char*>(&struct_.Name[1])));

				if(name_.empty()) {
					name_ = "<invalid>";
				}
			} else {
				bool sh = false;
				for(int i = 0; i < 8; ++i)
					if(struct_.Name[i] == '\0')
						sh = true;

				if(sh)
					name_ = std::string(reinterpret_cast<char*>(struct_.Name));
				else
					name_ = std::string(reinterpret_cast<char*>(struct_.Name), 8);
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
			rel.setType(static_cast<unsigned short>(type));
			rel.setVirtualAddress(struct_.SizeOfRawData);

			relocs_.push_back(rel);
		}

		//
		// amd64 version
		//
		void Section::markRelocation(Symbol& sym, Relocation::Amd64TypeFlags type)
		{
			//
			// Maybe check if there is a relocation at this position already?
			//
			struct_.NumberOfRelocations++;

			Relocation rel;

			// relocation from current position to given symbol's index.
			rel.setSymbolTableIndex(sym.getIndex());
			rel.setType(static_cast<unsigned short>(type));
			rel.setVirtualAddress(struct_.SizeOfRawData);

			relocs_.push_back(rel);
		}

		void Section::markSymbol(Symbol& sym)
		{
			//
			// set the symbols Value field to the end of the current data
			//
			sym.setValue(struct_.SizeOfRawData);
			sym.setSectionNumber(static_cast<short>(idx_));
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

		void Section::insert(FileHeader& hdr, const void *data, size_t len, size_t pos) {

			//
			// do the real insertion
			//
			insertData(hdr, data, len, pos);

			while(1) {
				//
				// pos is updated if there was the need to insert bytes before pos!
				//
				if(updateOffsets(hdr, &pos, len) <= 0)
					break;
			}
		}

		//
		// returns 1 if operation needs to be restarted!
		//
		int Section::updateOffsets(FileHeader& hdr, size_t* pos, size_t length) {
			//
			// Determine the symbol we are in and how long it is.
			//
			const Symbol* lower = 0;
			const Symbol* upper = 0;

			for(Symbol::IndexedSymbolMap::const_iterator it = hdr.getAllSymbols().begin(); it != hdr.getAllSymbols().end(); ++it)
			{
				if(it->second.getSectionNumber() == idx_)
				{
					if(it->second.getValue() <= *pos) {
						if(!lower || (lower && it->second.getValue() >= lower->getValue()))
							lower = &it->second;
					} else if(it->second.getValue() >= *pos) {
						if(!upper || (upper && it->second.getValue() <= upper->getValue()))
							upper = &it->second;
					}
				}

				if(lower && upper)
					break;
			}

			if(!lower)
				throw utils::Exception("not inside a symbol where insertion would be allowed");

			size_t end = struct_.SizeOfRawData;

			if(upper)
				end = upper->getValue();

			//
			// Decode the "upper" symbol and fix all direct jumps inside it.
			// WARNING: i don't really know, but _maybe_ we get an object where there
			// are direct jumps crossing symbol boundaries. if this is encountered, the
			// resulting code _will_ be broken, since only the changed symbol is fixed.
			// If this is ever the case, this code has to be changed to decode the whole
			// sections instead of just the affected symbol.
			//
			Decoder dec(MAKEPTR(void*, data_, lower->getValue()), end - lower->getValue());
			Decoder::InstructionVector const& instructions = dec.getInstructions();

			utils::Log::verbose("decoded content of section no. %d, %ld instructions.\n", lower->getSectionNumber(), instructions.size());

			//
			// for fast lookup i prepare a map of addresses and booleans which indicate wether
			// there is a relocation at a given address.
			//
			std::map<int, bool> relocationAt;
			Relocation::RelocationVector const& relocations = hdr.getSection(lower->getSectionNumber()).getRelocations();

			for(Relocation::RelocationVector::const_iterator crel = relocations.begin(); crel != relocations.end(); ++crel)
			{
				relocationAt[crel->getVirtualAddress()] = true;
			}

			//
			// WARNING: this is quite slow in the case of an address overflow in a direct jump.
			// In that case the instruction needs to be changed to one that recieves a longer
			// operand, and thus an extra byte (or more) need to be inserted into the section
			// data. this requires the whole process of updating to be restarted from the beginning
			// (i.e. return 1).
			//
			int position = 0;
			int symrel_isertionpoint = *pos - lower->getValue();

			for(Decoder::InstructionVector::const_iterator insn = instructions.begin(); insn != instructions.end(); ++insn) {
				//
				// check wether the instruction has an address that needs fixing.
				//
				switch(insn->getOpcode()) {
				case 0xE8:								// call rel16/32		-> may need patching.
				case 0xE9:								// jmp rel16/32			-> may need patching.
					{
						if(relocationAt.find(position + 1) != relocationAt.end())
							break;
						//
						// we assume that this fits in 32bits, so there are no safety nets here.
						// TODO: check wether this really works.. reading this a while after i
						// wrote it, i'm not sure wether the addresses are patched correctly here.
						//
						char* target = MAKEPTR(char*, data_, position + 1 /* offset for insn byte */);
						
						if(position < symrel_isertionpoint && (position + *target + insn->getLength()) >= symrel_isertionpoint) {
							*target += length;
						} else if(position > symrel_isertionpoint && (position + *target + insn->getLength()) <= symrel_isertionpoint) {
							*target -= length;
						}
					}
					break;
				case 0x9A:								// call indir 16/32		-> indirect, so can't do anything anyway
					break;
				case 0xEA:								// jmp far abs.			-> same as 0xFF mod 0x18
					utils::Log::warning("absolute address hardcoded for jump in unlinked object code!\n");
					break;
				case 0xFF:								// check mod R/M byte for insn group.
					switch(insn->getModRm() & 0xC7) {	// check only mod field
					case 0x10:							// call indir r/m16/32	-> indirect, can't change.
						break;
					case 0x18:							// call abs indir 16/32	-> indirect, but through hardcoded abs addr.
						utils::Log::warning("absolute address hardcoded for call instruction in unlinked object code!\n");
						break;
					case 0x20:							// jmp r/m16/32			-> indirect, can't change.
						break;
					case 0x28:							// jmp abs indir 16/32	-> indirect, but through hardcoded abs addr.
						utils::Log::warning("absolute address hardcoded for jump instruction in unlinked object code!\n");
						break;
					}
					break;

				case 0x70: case 0x71: case 0x72:
				case 0x73: case 0x74: case 0x75:
				case 0x76: case 0x77: case 0x78:
				case 0x79: case 0x7A: case 0x7B:
				case 0x7C: case 0x7D: case 0x7E:		// J??? rel8.
				case 0x7F: case 0xE3:
				case 0xEB:								// JMP rel8
				case 0xE0: case 0xE1: case 0xE2:		// LOOP?? rel8
					{
						if(relocationAt.find(position + 1) != relocationAt.end())
							break;

						char* target = MAKEPTR(char*, data_, position + 1 /* offset for insn byte */);
						int new_target = 0;

						//
						// check against symbol relative insertion point if jump crosses.
						// no need to check for position == symrel_insertionpoint [+length], since at
						// least one byte was added for which we don't have any responsability.
						//
						if(position < symrel_isertionpoint) {
							if((position + *target + insn->getLength()) >= symrel_isertionpoint) {
								// Jump forward that crosses.
								// sanity check if the target is positive.
								if(*target <= 0)
									throw utils::Exception("forward jump with negative or zero offset value (%d)!", *target);

								new_target = *target + length;
							}
						} else if(position > symrel_isertionpoint + static_cast<int>(length)) {
							if((position + *target + insn->getLength()) <= symrel_isertionpoint) {
								// Jump backward that crosses.
								// sanity check if the target is negative.
								if(*target >= 0)
									throw utils::Exception("backward jump with positive or zero offset value (%d)!", *target);

								new_target = *target - length;
							}
						}

						if(new_target > CHAR_MAX || new_target < CHAR_MIN) {
							//
							// uh oh. need a bigger instruction.
							// we need to insert exactly 4 bytes, growing the instruction
							// from 2 to 6 bytes, 1 byte for the additional opcode, and 3 bytes
							// for the bigger address.
							//
							switch(insn->getOpcode()) {
							case 0xE0: case 0xE1: case 0xE2: case 0xE3:
								//
								// these can't grow!
								//
								throw utils::Exception("cannot grow JCXZ/JECXZ or LOOP instruction!");
							default:
								break;
							}

							utils::Log::verbose("growing jump instruction at %d, target offset: %d.\n", position, new_target);

							unsigned char grow_data2[] = { 0x00, 0x00, 0x00, 0x00 };
							unsigned char grow_data1[] = { 0x00, 0x00, 0x00, 0x00 };

							if(insn->getOpcode() == 0xEB) // JMP rel8
								insert(hdr, grow_data1, sizeof(grow_data1), position);
							else
								insert(hdr, grow_data2, sizeof(grow_data2), position);

							unsigned char* insn_op = MAKEPTR(unsigned char*, data_, position);

							//
							// change the opcode to the 16/32 bit version of the instruction.
							//
							if(insn->getOpcode() == 0xEB) { // JMP rel8
								insn_op[0] = 0xE9;
							} else {
								insn_op[0] = 0x0F;
								insn_op[1] = insn->getOpcode() + 0x10;
							}

							int* insn_data = MAKEPTR(int*, insn_op, (insn->getOpcode() == 0xEB) ? 1 : 2);
							*insn_data = new_target;

							//
							// need to restart processing here, since positions changed again!
							// before restarting, update pos if it changed!
							//
							if(position < static_cast<int>(*pos)) {
								*pos += (insn->getOpcode() == 0xEB) ? sizeof(grow_data1) : sizeof(grow_data2);
							}

							return 1;
						} else {
							*target = static_cast<char>(new_target);
						}
					}
					break;

				case 0x0F:
					// 16/32 bit relative jump instructions
					switch(insn->getOpcode2()) {
					case 0x80: case 0x81: case 0x82:
					case 0x83: case 0x84: case 0x85:
					case 0x86: case 0x87: case 0x88:
					case 0x89: case 0x8A: case 0x8B:
					case 0x8C: case 0x8D: case 0x8E:		// J??? rel16/32.
					case 0x8F:
						{
							if(relocationAt.find(position + 2) != relocationAt.end())
								break;

							//
							// just like for the "big" call instructions, there are no safety nets here,
							// and i just assume, that things fit in 32 bits.
							// TODO: same as above: check wether this really patches correctly, since i
							// don't think so reading this again.
							//
							char* target = MAKEPTR(char*, data_, position + 2 /* offset for (2!) insn bytes */);
						
							if(position < symrel_isertionpoint && (position + *target + insn->getLength()) >= symrel_isertionpoint) {
								*target += length;
							} else if(position > symrel_isertionpoint && (position + *target + insn->getLength()) <= symrel_isertionpoint) {
								*target -= length;
							}
						}
						break;
					}
					break;
				}

				position += insn->getLength();
			}

			//
			// all done.
			//
			return 0;
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

				if(padEnd == 0 && (reinterpret_cast<unsigned char*>(data_))[end] == 0xcc)
					padEnd = end;
				else if((reinterpret_cast<unsigned char*>(data_))[end] != 0xcc) {
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
				// update symEnd to reflect the current value.
				//
				symEnd += (padPos - padOldPos) + len;
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
					addData(reinterpret_cast<const void*>(&chInt3), 1);
				else
					addData(reinterpret_cast<const void*>(&chZero), 1);
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

