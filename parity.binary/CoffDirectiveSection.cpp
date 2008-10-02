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

#include "CoffDirectiveSection.h"
#include "CoffSection.h"
#include "CoffFileHeader.h"

#include <Log.h>

#include <cstdio>

namespace parity
{
	namespace binary
	{
		void DirectiveSection::calcDirectives(const Section& sect)
		{
			//
			// Double Check section characteristics, if they match!
			//
			if(!(sect.getCharacteristics() & Section::CharLinkInfo)
				|| sect.getNumberOfLinenumbers() > 0
				|| sect.getNumberOfRelocations() > 0)
				throw utils::Exception("section characteristics don't match a directive section!");

			//
			// Cannot use getPointerFromRVA, since PointerToRawData is a file pointer.
			//
			const char* start = reinterpret_cast<const char*>(sect.getDataPtr());

			if(!start || sect.getSizeOfRawData() == 0)
				return;

			//
			// Check BOM for UTF-8
			//
			if(static_cast<unsigned char>(start[0]) == 0xEF && static_cast<unsigned char>(start[1]) == 0xBB && static_cast<unsigned char>(start[2]) == 0xBF)
			{
				//
				// This is an UTF-8 String!
				//
				throw utils::Exception("directive section is in UTF-8 format, which is not supported!");
			}

			const char* end = start;
			const char* top = start + sect.getSizeOfRawData();

			while(*end != 0 && end < top)
			{
				while(*start == ' ')
					++start;
				
				end = start;

				while(end < top && *end != ' ')
				{
					//
					// Handle Strings
					//
					if(*end == '"')
					{
						++end;

						while(end < top && *end != '"')
							++end;

						if(end == top)
							throw utils::Exception("unterminated string in directives section.");
					}
					++end;
				}

				vector_.push_back(std::string(start, end));

				if(end >= top)
					break;

				start = ++end;
			}
		}

		void DirectiveSection::addDirective(const std::string& dir)
		{
			static const unsigned char space = ' ';

			vector_.push_back(dir);
			section_.addData(dir.c_str(), dir.length());
			section_.addData(&space, 1);
		}

	}
}

