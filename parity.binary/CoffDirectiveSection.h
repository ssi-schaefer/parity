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

#ifndef __DIRECTIVESECTION_H__
#define __DIRECTIVESECTION_H__

#include <cstring>
#include <string>
#include <vector>

#include <GenericFile.h>
#include <Pointer.h>
#include <Exception.h>

#include "CoffRelocation.h"
#include "CoffSymbol.h"
#include "CoffSection.h"

namespace parity
{
	namespace binary
	{
		class DirectiveSection {
		public:
			//
			// Contructors and Destructors
			//
			DirectiveSection(Section& sect) : vector_(), section_(sect) { calcDirectives(sect); }
			
			typedef std::vector<std::string> DirectiveVector;
			//
			// Complex Methods
			//
			const DirectiveVector& getDirectives() const { return vector_; }
			void addDirective(const std::string& dir);

		private:
			void calcDirectives(const Section& sect);

			//
			// Attributes
			//
			DirectiveVector vector_;
			Section& section_;
		};
	}
}

#endif

