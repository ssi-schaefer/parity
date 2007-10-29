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

#include "Argument.h"

#include <Context.h>
#include <Exception.h>

namespace parity
{
	namespace options
	{
		bool genericTrigger(const char * map, const char * arg)
		{
			utils::Context& ctx = utils::Context::getContext();
			const utils::ContextMap* mapping = ctx.getMapping(map);

			if(!arg)
				throw utils::Exception("Argument may not be NULL for generic context mapping trigger!");

			if(!mapping)
				throw utils::Exception("Argument for %s implemented but the context mapping is missing!", map);

			(ctx.*(mapping->setter))(arg);

			return true;
		}
	}
}

