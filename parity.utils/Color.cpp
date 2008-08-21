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

#include "Color.h"
#include "Context.h"

namespace parity
{
	namespace utils
	{
		//
		// Color escapes for interix terminals (also xterm, mrxvt, ...)
		//
		const Color::ColorValues Color::colors_[] = {
			{ "\033[01;31m", "\033[01;32m", "\033[01;33m", "\033[01;34m", "\033[01;35m", "\033[01;36m", "\033[00m" },
			{ "\033[00;31m", "\033[00;32m", "\033[00;33m", "\033[00;34m", "\033[00;35m", "\033[00;36m", "\033[00m" },
		};

		std::string Color::color(const std::string& value, Color::ColorName color)
		{
			if(!Context::getContext().getColored())
				return value;

			std::string result = colors_[mode_].values[color];
			result.append(value);
			result.append(colors_[mode_].values[Color::Reset]);

			return result;
		}
	}
}

