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

#include "Windres.h"
#include <Log.h>
#include <Statistics.h>

namespace parity
{
	namespace options
	{
		bool setInputFormat(const char* option, const char* argument, bool& used)
		{
			char const * arg = NULL;
			if (strncmp(option, "--input-format", 14) == 0) {
				if (option[14] == '=') {
					arg = option + 15;
				} else
				if (option[14]) {
					return false;
				}
			} else
			if (strlen(option) > 2) {
				arg = option + 2;
			}
			if (!arg) {
				arg = argument;
				used = true;
			}

			utils::Context::getContext().setInputFormat(arg);
			return true;
		}

		bool setOutputFormat(const char* option, const char* argument, bool& used)
		{
			char const * arg = NULL;
			if (strncmp(option, "--output-format", 15) == 0) {
				if (option[15] == '=') {
					arg = option + 16;
				} else
				if (option[15]) {
					return false;
				}
			} else
			if (strlen(option) > 2) {
				arg = option + 2;
			}
			if (!arg) {
				arg = argument;
				used = true;
			}

			utils::Context::getContext().setOutputFormat(arg);
			return true;
		}

		bool setBfdTarget(const char* option, const char* argument, bool& used)
		{
			char const * arg = NULL;
			if (strncmp(option, "--target", 8) == 0) {
				if (option[8] == '=') {
					arg = option + 9;
				} else
				if (option[8]) {
					return false;
				}
			} else
			if (strlen(option) > 2) {
				arg = option + 2;
			}
			if (!arg) {
				arg = argument;
				used = true;
			}

			utils::Context::getContext().setBfdTarget(arg);
			return true;
		}

		bool setCodepage(const char* option, const char* argument, bool& used)
		{
			char const * arg = NULL;
			if (strncmp(option, "--codepage", 10) == 0) {
				if (option[10] == '=') {
					arg = option + 11;
				} else
				if (option[10]) {
					return false;
				}
			} else
			if (strlen(option) > 2) {
				arg = option + 2;
			}
			if (!arg) {
				arg = argument;
				used = true;
			}

			utils::Context::getContext().setCodepage(arg);
			return true;
		}

		bool setLanguage(const char* option, const char* argument, bool& used)
		{
			char const * arg = NULL;
			if (strncmp(option, "--language", 10) == 0) {
				if (option[10] == '=') {
					arg = option + 11;
				} else
				if (option[10]) {
					return false;
				}
			} else
			if (strlen(option) > 2) {
				arg = option + 2;
			}
			if (!arg) {
				arg = argument;
				used = true;
			}

			utils::Context::getContext().setLanguage(arg);
			return true;
		}

	}
}

