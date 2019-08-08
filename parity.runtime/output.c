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

#include <stdarg.h>
#include <windows.h>

#include "internal/output.h"

void PcrtOutFormatStringVa(char* buffer, const char* fmt, va_list args)
{
	const char* ptr = fmt;
	const char* end = ptr;

	buffer[0] = '\0';

	while(end && *end != '\0')
	{
		while(*end != '%' && *end != '\0')
			++end;

		lstrcpyn(&buffer[lstrlen(buffer)], ptr, (end - ptr) + 1);

		ptr = end+2;

		if(*end == '%')
		{
			int radix = 10;

			++end;

			switch(*end)
			{
			case 'p':
			case 'x':
				radix = 16;
				// fall through
			case 'd':
				{
					char buf[2*sizeof(uintptr_t)+1];
					unsigned char* wh = buf + 2*sizeof(uintptr_t);

					uintptr_t num = va_arg(args, uintptr_t);

					*wh = '\0';

					do
					{
						uintptr_t temp;

						temp = num % radix;
						wh--;
						if(temp < 10)
							*wh = temp + '0';
						else
							*wh = temp - 10 + 'a';
						num = num / radix;
					}
					while(num != 0);

					if(*end == 'p') {
						//
						// prepend 0's
						//
						size_t req = (sizeof(void*) * 2) - lstrlen(wh);

						while(req--) {
							lstrcat(buffer, "0");
						}
					}

					lstrcat(buffer, wh);
				}
				break;
			case 's':
				{
					const char* tmp = va_arg(args, const char*);

					if(tmp)
						lstrcat(buffer, tmp);
					else
						lstrcat(buffer, "(null)");
				}
				break;
			case 'c':
				{
					char buf[2] = { 0, 0 };
					buf[0] = va_arg(args, char);
					lstrcat(buffer, buf);
				}
				break;
			default:
				{
					lstrcat(buffer, "<unsupported specifier>");
				}
			}
		}

		if(*end != '\0')
			++end;
	}
}

void PcrtOutFormatString(char* buffer, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	PcrtOutFormatStringVa(buffer, fmt, args);

	va_end(args);
}

void PcrtOutPrint(HANDLE dest, char const* fmt, ...)
{
	//
	// FIXXME: uah... hack :( there is no _fast_ way of
	// determining the string length. maybe i could approximate it
	// by calculating with maximum lengths for basic data types
	// like int, long, etc. and adding string lengths...
	//
	char buffer[2048];
	long iBytesWritten;
	va_list args;
	va_start(args, fmt);

	PcrtOutFormatStringVa(buffer, fmt, args);

	va_end(args);

	if(dest != INVALID_HANDLE_VALUE)
		WriteFile(dest, buffer, lstrlen(buffer), &iBytesWritten, NULL);
	else
		OutputDebugStringA(buffer);
}

void PcrtOutDebugString(char* buffer)
{
	//
	// write line by line...
	//
	char* start = buffer;
	char* end = start;

	while(1)
	{
		int isEnd = 0;

		if(*end == '\0' || *end == '\n')
		{
			char repl = '\0';

			if(*end == '\0') isEnd = 1;
			else { ++end; repl = *end; *end = '\0'; }

			OutputDebugStringA(start);

			if(isEnd)
				return;

			*end = repl;
			start = end;
		}

		++end;
	}
}
