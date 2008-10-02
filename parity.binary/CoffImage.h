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

#ifndef __IMAGE_H__
#define __IMAGE_H__

#include <GenericFile.h>
#include <Pointer.h>

#include "CoffFileHeader.h"
#include "CoffMsDosStub.h"

namespace parity
{
	namespace binary
	{
		class Image {
		public:
			//
			// the offset to the PE header is at offset 0x3C, the header starts
			// 4 bytes after that (signature)
			//
			Image(const utils::GenericFile* ptr)
				: file_(ptr), stub_(ptr->getBase(), *(MAKEPTR(unsigned int*, ptr->getBase(), 0x3C)))
				, hdr_(ptr, MAKEPTR(void*, ptr->getBase(), *(MAKEPTR(unsigned int*, ptr->getBase(), 0x3C)) + 4), false)
				, opt_(hdr_.getOptionalHeader())
			{}

			Image(Image const& rhs) : file_(rhs.file_), stub_(rhs.stub_), hdr_(rhs.hdr_), opt_(rhs.opt_) {}
			Image& operator=(Image const& rhs) { file_ = rhs.file_; stub_ = rhs.stub_; hdr_ = rhs.hdr_; opt_ = rhs.opt_; return *this; }

			FileHeader& getHeader() { return hdr_; }
			OptionalHeader& getOptionalHeader() { return opt_; }
			MsDosStub& getMsDosStub() { return stub_; }

			void update(utils::MemoryFile& file) { stub_.update(file); file.append("PE\0\0", 4); hdr_.update(file, &opt_); }

		private:
			const utils::GenericFile* file_;
			MsDosStub stub_;
			FileHeader hdr_;
			OptionalHeader opt_;
		};
	}
}

#endif

