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

#include "Options.h"
#include <Log.h>
#include <iostream>

namespace parity 
{
	namespace inspector 
	{

		static bool setVerbose(const char* OPT_UNUSED(option), const char* OPT_UNUSED(argument), bool& OPT_UNUSED(used))
		{
			utils::Log::setLevel(utils::Log::Verbose);
			return true;
		}

		static bool printHelp(const char* OPT_UNUSED(option), const char* OPT_UNUSED(argument), bool& OPT_UNUSED(used))
		{
			std::cout << "parity.inspector [-h | --help] [--verbose] [-s | --symbols]" << std::endl;
			std::cout << "    [--short] [--full] [--flat] binary [binary ...]" << std::endl;

			std::cout << std::endl;
			std::cout << "How to interpret the output:" << std::endl;
			std::cout << " * [<LoadType>, <ImportType>] <SymbolName>" << std::endl;
			std::cout << "<LoadType> may be either CODE, LAZY or DATA. LAZY means" << std::endl;
			std::cout << "that the symbol is loaded when it's needed the first time." << std::endl;
			std::cout << "<ImportType> may be either NAME or ORD. ORD means that the" << std::endl;
			std::cout << "symbol is imported by ordinal number rather than by name." << std::endl << std::endl;
			std::cout << "if a library name is marked with a \"*\", this means, that" << std::endl;
			std::cout << "this library is imported using the standard windows mechanism." << std::endl;
			exit(0);

			//
			// this one is to sattisfy cc89 on vista.
			// seems it doesn't have a noreturn declspec.
			//
			return false;
		}

		static bool addFile(const char* option, const char* OPT_UNUSED(argument), bool& OPT_UNUSED(used))
		{
			utils::Path pth(option);
			pth.toNative();

			if(pth.exists())
				gFilesToProcess.push_back(pth);
			else {
				utils::Log::verbose("cannot find file or unknown option: %s (rejecting)\n", option);
				return false;
			}

			return true;
		}

		static bool setShowSymbols(const char* OPT_UNUSED(option), const char* OPT_UNUSED(argument), bool& OPT_UNUSED(used))
		{
			return (gShowSymbols = true);
		}

		static bool setShortFormat(const char* OPT_UNUSED(option), const char* OPT_UNUSED(argument), bool& OPT_UNUSED(used))
		{
			return (gShortFormat = true);
		}

		static bool setLddLike(const char* OPT_UNUSED(option), const char* OPT_UNUSED(argument), bool& OPT_UNUSED(used))
		{
			return (gShowLddLike = false);
		}

		static bool setLddFlat(const char* OPT_UNUSED(option), const char* OPT_UNUSED(argument), bool& OPT_UNUSED(used))
		{
			return (gShowLddFlat = true);
		}

		static options::ArgumentTableEntry sInspectorOptionTable[] = {
			{ "--verbose"		, setVerbose		},
			{ "-h"				, printHelp			},
			{ "--help"			, printHelp			},
			{ "-s"				, setShowSymbols	},
			{ "--symbols"		, setShowSymbols	},
			{ "--short"			, setShortFormat	},
			{ "-l"				, setLddLike		},
			{ "--full"			, setLddLike		},
			{ "--flat"			, setLddFlat		},
			{ ""				, addFile			},
			{ 0					, 0					}
		};

		options::ArgumentTableEntry* gInspectorOptionTable = sInspectorOptionTable;
		utils::PathVector gFilesToProcess;
		bool gShowSymbols = false;
		bool gShortFormat = false;
		bool gShowLddLike = true;
		bool gShowLddFlat = false;

	}
}

