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

#include "CommandLine.h"

#include <Log.h>

#include <string>
#include <cstring>

namespace parity
{
	namespace options
	{
		bool CommandLine::process(int argc, char **argv, ArgumentTableEntry* triggers, UnknownArgumentVector* vec)
		{
			bool bRet = true;

			for(int i = 0; i < argc; i++)
			{
				ArgumentTableEntry* entry = triggers;
				bool bMatched = false;

				size_t argvLen = ::strlen(argv[i]);

				if(argvLen == 0)
					continue;

				while(entry->option)
				{
					size_t entryLen = ::strlen(entry->option);

					if(entryLen == 0 ||	(entryLen >= 1 && argvLen >= 1 && ((entry->option[0] == '-' && argv[i][0] == '-') ||
						(entry->option[0] == '/' && (argv[i][0] == '-' || argv[i][0] == '/'))) &&
						::strncmp(&entry->option[1], &argv[i][1], entryLen - 1) == 0) ||
						(entry->option[0] == '.' && /* (breaks gcc abs. paths) argv[i][0] != '/' && */ argv[i][0] != '-' 
							&& argvLen > entryLen && ::strncmp(entry->option, &argv[i][argvLen - entryLen], entryLen) == 0) ||
						(entry->option[0] == '.' && /* (breaks gcc abs. paths) argv[i][0] != '/' && */ argv[i][0] != '-' 
							&& argvLen > entryLen && ::strstr(argv[i], entry->option) && ::strstr(argv[i], entry->option)[entryLen] == '.'))
					{
						const char* argument = 0;
						bool argumentUsed = false;

						if(i+1 < argc)
							argument = argv[i+1];

						if(!entry->argument)
							throw utils::Exception("trigger function pointer for %s (%s) is invalid!", argv[i], entry->option);

						if(entry->argument(argv[i], argument, argumentUsed))
						{
							parity::utils::Log::verbose("accepted \"%s\"",argv[i]);

							if(argumentUsed)
							{
								++i;
								parity::utils::Log::verbose(" with argument \"%s\"", argv[i]);
							} else {
								parity::utils::Log::verbose(" without argument");
							}

							parity::utils::Log::verbose(" by entry \"%s\".\n", entry->option);

							bMatched = true;
						}
					}

					if(bMatched)
						break;

					++entry;
				}

				if(!bMatched)
				{
					if(vec)
					{
						vec->push_back(argv[i]);
					} else {
						parity::utils::Log::verbose("ignoring unknown option: %s!\n", argv[i]);
					}
				}
			}

			return bRet;
		}

		#ifdef stristr
		#  undef stristr
		#endif
		#ifdef strnicmp
		#  undef strnicmp
		#endif
	}
}

