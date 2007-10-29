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

#include "CoffFile.h"

namespace parity
{
	namespace binary
	{
		File::Type File::getType(const utils::GenericFile* ptr)
		{
			switch(*(const unsigned short*)ptr->getBase())
			{
			case 0x5A4D: // Signature for the MS-DOS 2.0 Compatability Header
				return TypeImage;
			default:
				if(FileHeader::isValidMachine(*(const unsigned short*)ptr->getBase()))
					return TypeObject;
				else if(strncmp((const char*)ptr->getBase(), "!<arch>\n", 8) == 0)
					return TypeLibrary;
				else
					return TypeInvalid;
			}
		}

	}
}

