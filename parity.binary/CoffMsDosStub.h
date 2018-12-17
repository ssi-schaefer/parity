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

#ifndef __MSDOSSTUB_H__
#define __MSDOSSTUB_H__

#include <MemoryFile.h>
#include <Exception.h>

#include <cstdlib>
#include <cstring>

namespace parity
{
	namespace binary
	{
		class MsDosStub {
		public:
			MsDosStub(void* ptr, size_t length);
			~MsDosStub();

			MsDosStub(MsDosStub const& rhs) : data_(::malloc(rhs.size_)), size_(rhs.size_) {
				if (!data_)
					throw utils::Exception("cannot allocate memory for MsDos Stub");
				
				::memcpy(data_, rhs.data_, size_);
			}

			MsDosStub& operator=(MsDosStub const& rhs) {
				if(data_) 
					free(data_);

				size_ = rhs.size_;
				data_ = ::malloc(size_);

				if(!data_)
					throw utils::Exception("cannot allocate memory for MsDos Stub");

				::memcpy(data_, rhs.data_, size_);
				return *this;
			}

			//unsigned int getSignaturePointer() { return *(MAKEPTR(unsigned int*, data_, 0x3C)); }
			//void setSignaturePointer(unsigned int val) { *(MAKEPTR(unsigned int*, data_, 0x3C)) = val; }

			unsigned short getSizeInFile() { return static_cast<unsigned short>(size_); }
			void update(utils::MemoryFile& file) { file.append(data_, size_); }

		private:
			void* data_;
			size_t size_;
		};
	}
}

#endif

