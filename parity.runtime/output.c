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

void PcrtOutFormatString(char* buffer, const char* fmt, va_list args)
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
					char buf[64];
					unsigned char* wh = buf + 63;

					int num = va_arg(args, int);

					*wh = '\0';

					do
					{
						unsigned long temp;

						temp = (unsigned long)num % radix;
						wh--;
						if(temp < 10)
							*wh = (unsigned char)temp + '0';
						else
							*wh = (unsigned char)temp - 10 + 'a';
						num = (unsigned long)num / radix;
					}
					while(num != 0);

					if(*end == 'p') {
						//
						// prepend 0's
						//
						unsigned int req = (sizeof(void*) * 2) - lstrlen(wh);

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
					const char msgErrUnsupported[] = "<unsupported specifier>";
					lstrcat(buffer, msgErrUnsupported);
				}
			}
		}

		if(*end != '\0')
			++end;
	}
}

void PcrtOutPrint(HANDLE dest, char const* fmt, ...)
{
	//
	// FIXXME: uah... hack :(
	//
	char buffer[2048];
	long iBytesWritten;
	va_list args;
	va_start(args, fmt);

	PcrtOutFormatString(buffer, fmt, args);

	va_end(args);

	WriteFile(dest, buffer, lstrlen(buffer), &iBytesWritten, NULL);
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
