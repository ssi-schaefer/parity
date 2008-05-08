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

#ifndef __INSTRUCTION_H__
#define __INSTRUCTION_H__

#include <cstring>
#include <string>
#include <vector>

namespace parity
{
	namespace binary
	{
		class Decoder;

		class Instruction {

		private:
			friend class Decoder;
			Instruction() :
				length_(0), prefix66_(false), prefix67_(false), prefixLock_(false),
				prefixRep_(false), prefixSeg_(false), prefixOpcode2_(false), prefixModRm_(false),
				prefixSib_(false), segment_(0), rep_(0), opcode_(0), opcode2_(0), modrm_(0),
				sib_(0), memsize_(0), datasize_(0) {}

		public:
			int getLength() const { return length_; }

			bool has66Prefix() const { return prefix66_; }
			bool has67Prefix() const { return prefix67_; }
			bool hasLockPrefix() const { return prefixLock_; }
			bool hasRepPrefix() const { return prefixRep_; }
			bool hasSegmentPrefix() const { return prefixSeg_; }
			bool hasOpcode2Prefix() const { return prefixOpcode2_; }
			bool hasModRmPrefix() const { return prefixModRm_; }
			bool hasSibPrefix() const { return prefixSib_; }

			unsigned char getSegment() const { return segment_; }
			unsigned char getRep() const { return rep_; }
			unsigned char getOpcode() const { return opcode_; }
			unsigned char getOpcode2() const { return opcode2_; }
			unsigned char getModRm() const { return modrm_; }
			unsigned char getSib() const { return sib_; }
			unsigned char const * getMemory() const { return mem_; }
			unsigned char const * getData() const { return data_; }

			int getMemorySize() { return memsize_; }
			int getDataSize() { return datasize_; }

		private:
			int length_;

			bool prefix66_;
			bool prefix67_;
			bool prefixLock_;
			bool prefixRep_;
			bool prefixSeg_;
			bool prefixOpcode2_;
			bool prefixModRm_;
			bool prefixSib_;

			unsigned char segment_;
			unsigned char rep_;
			unsigned char opcode_;
			unsigned char opcode2_;
			unsigned char modrm_;
			unsigned char sib_;
			unsigned char mem_[8];
			unsigned char data_[8];

			int memsize_;
			int datasize_;
		};
	}
}

#endif

