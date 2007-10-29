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

#ifndef __COLOR_H__
#define __COLOR_H__

#include <string>

namespace parity
{
	namespace utils
	{
		class Color
		{
		public:

			typedef enum {
				Bright,
				Dark
			} ColorMode;

			typedef enum {
				Red,
				Green,
				Yellow,
				Blue,
				Magenta,
				Cyan,
				Reset
			} ColorName;

			typedef struct {
				const char* values[7];
			} ColorValues;

			Color(ColorMode mode) : mode_(mode) {}

			std::string red(const std::string& value) { return color(value, Color::Red); }
			std::string green(const std::string& value) { return color(value, Color::Green); }
			std::string yellow(const std::string& value) { return color(value, Color::Yellow); }
			std::string blue(const std::string& value) { return color(value, Color::Blue); }
			std::string magenta(const std::string& value) { return color(value, Color::Magenta); }
			std::string cyan(const std::string& value) { return color(value, Color::Cyan); }

			std::string color(const std::string& value, ColorName color);

			bool isColored() { return (strlen(colors_[0].values[0]) > 0); }

		private:

			ColorMode mode_;
			static const ColorValues colors_[];
		};
	}
}

#endif

