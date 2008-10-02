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

#ifndef __OBJECT_H__
#define __OBJECT_H__

#include <GenericFile.h>
#include <Pointer.h>

#include "CoffFileHeader.h"

namespace parity
{
	namespace binary
	{
		class Object {
		public:
			Object(const utils::GenericFile* ptr)
				: file_(ptr), hdr_(ptr, ptr->getBase(), false) {}
			Object() : file_(0), hdr_() {}
			Object(Object const& rhs) : file_(rhs.file_), hdr_(rhs.hdr_) {}
			Object& operator=(Object const& rhs) { file_ = rhs.file_; hdr_ = rhs.hdr_; return *this; }

			FileHeader& getHeader() { return hdr_; }

			void update(utils::MemoryFile& file) { hdr_.update(file, 0); }

		private:
			const utils::GenericFile* file_;
			FileHeader hdr_;
		};
	}
}

#endif

