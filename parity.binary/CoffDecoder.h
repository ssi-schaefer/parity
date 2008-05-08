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

#ifndef __DECODER_H__
#define __DECODER_H__

#include <vector>

#include <Exception.h>

#include "CoffInstruction.h"

namespace parity
{
	namespace binary
	{
		class Decoder {

		public:
			//
			// Contructors and Destructors
			//
			Decoder(void* ptr, size_t length) { decode(ptr, length); }

			typedef std::vector<Instruction> InstructionVector;

			InstructionVector const& getInstructions() const { return instructions_; }

		private:

			InstructionVector instructions_;

			void decode(void* what, size_t length);
			int decodeNext(void* position);
		};
	}
}

#endif

