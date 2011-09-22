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
#include <Context.h>

#include <string>
#include <cstring>
#include <fstream>

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
							&& argvLen > entryLen && ::strstr(argv[i], entry->option) && ::strstr(argv[i], entry->option)[entryLen] == '.') ||
						(argvLen >= entryLen && ::strncmp(entry->option, argv[i], entryLen) == 0))
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

				//
				// recursively process contents of argument files
				//
				utils::ArgumentVector extraArguments = utils::Context::getContext().pullExtraArguments();
				std::vector<char *> extraArgv;
				for(utils::ArgumentVector::const_iterator it = extraArguments.begin()
				  ; it != extraArguments.end()
				  ; ++ it
				) {
					extraArgv.push_back(const_cast<char*>(it->c_str()));
				}
				extraArgv.push_back(0);
				process(extraArgv.size() - 1, &extraArgv.front(), triggers, vec);
			}

			return bRet;
		}

		class CommandFile {
		public:
			CommandFile(std::string const &filename);
			int read();
			utils::ArgumentVector const& arguments() const { return arguments_; }
		private:
			parity::utils::Path file_;
			utils::ArgumentVector arguments_;
		};

		CommandFile::CommandFile(std::string const &filename)
		  : file_(filename)
		  , arguments_()
		{
			file_.toNative();
		}

		int CommandFile::read()
		{
			std::ifstream argsFile;
			argsFile.exceptions(std::ifstream::badbit);
			try {
				parity::utils::Log::verbose("reading argument file \"%s\"\n", file_.get().c_str());
				argsFile.open(file_.get().c_str(), std::fstream::in);

				std::vector<char*> toParse;

				while(argsFile.good() && !argsFile.eof()) {
					std::string line;
					getline(argsFile, line);

					bool quoting = false;
					std::string::iterator begin;
					std::string::iterator current;
					begin = current = line.begin();
					while(current != line.end()) {
						bool isEnd = false;
						if (*current == '"') {
							quoting = !quoting;
							current = line.erase(current);
							continue;
						}
						if (!quoting && isspace(*current)) {
							isEnd = true;
						}

						if (isEnd) {
							if (begin != current) {
								arguments_.push_back(std::string(begin, current));
							}
							begin = current;
							++ begin;
							quoting = false;
						}
						++ current;
					}

					if (current != begin) {
						arguments_.push_back(std::string(begin, current));
					}
				}
			}
			catch (std::ifstream::failure e) {
				throw; // throw the file error
			}

			return arguments_.size();
		}

		bool setCommandFile(const char *option, const char *argument, bool &used)
		{
			if (option[0] != '@') {
				return false;
			}
			used = true;
			if (option[1]) {
				used = false;
				argument = &option[1];
			}

			CommandFile file(argument);
			file.read();
			utils::Context::getContext().storeExtraArguments(file.arguments());
			return true;
		}

		#ifdef stristr
		#  undef stristr
		#endif
		#ifdef strnicmp
		#  undef strnicmp
		#endif
	}
}

