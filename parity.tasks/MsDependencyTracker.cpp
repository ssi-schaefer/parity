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

#include "MsDependencyTracker.h"

#include <Log.h>
#include <MappedFile.h>
#include <Threading.h>
#include <Statistics.h>

#include <iostream>

namespace parity
{
	namespace tasks
	{
		MsDependencyTracker::MsDependencyTracker()
			: dependencies_(), lock_()
		{
			utils::Context& ctx = utils::Context::getContext();
			utils::PathVector& incPaths = ctx.getIncludePaths();
			utils::PathVector& sysPaths = ctx.getSysIncludePaths();

			for(utils::PathVector::iterator it = incPaths.begin(); it != incPaths.end(); ++it)
				it->toNative();

			for(utils::PathVector::iterator it = sysPaths.begin(); it != sysPaths.end(); ++it)
				it->toNative();
		}

		void MsDependencyTracker::doWork()
		{
			utils::Context& ctx = utils::Context::getContext();
			utils::SourceMap src = ctx.getSources();

			for(utils::SourceMap::iterator it = src.begin(); it != src.end(); ++it)
			{
				//
				// convert to native path, since we need it here!
				//
				utils::Path file = it->first;
				file.toNative();

				//
				// check wether the source file is C or C++
				//
				if(it->second != utils::LanguageC && it->second != utils::LanguageCpp)
				{
					utils::Log::verbose("skipping non C/C++ file %s\n", file.get().c_str());
				}

				//
				// calculate the output filename
				//
				utils::Path output = ctx.getDependencyOutput();

				if(output.get().empty() && ctx.getDependencyOnly())
					output = utils::Path();
				else if(output.get().empty() && ctx.getDefaultOutput() != ctx.getOutputFile())
					output = ctx.getOutputFile();

				if((output.get().empty() && !ctx.getDependencyOnly()) || src.size() > 1)
					output = file.file().substr(0, file.file().rfind(".")).append(".d");

				utils::Log::verbose("tracking %s\n", file.get().c_str());

				dependencies_.clear();
				lock_.clear();

				try {
					trackFile(file);
				} catch(const utils::Exception& e)
				{
					utils::Log::error("error while tracking %s: %s\n", file.get().c_str(), e.what());
					exit(1);
				}

				std::ofstream fstr;
				if(!output.get().empty())
					fstr.open(output.get().c_str());

				std::ostream& str = *(output.get().empty() ? &std::cout : &fstr);
				std::string target = ctx.getDependencyTarget();

				file.convertToUnix();

				if(target.empty())
				{
					target = file.get().substr(0, file.get().rfind(".")).append(".o");
				}

				str << target << ": " << file.get();

				utils::PathVector converted;
				for(DependencyTrackMap::iterator dep = dependencies_.begin(); dep != dependencies_.end(); ++dep)
				{
					utils::Path current(dep->first);
					current.convertToUnix();

					converted.push_back(current);
				}

				for(utils::PathVector::iterator dep = converted.begin(); dep != converted.end(); ++dep)
				{
					if(*dep != file)
						str << " \\" << std::endl << "\t" << dep->get();
				}

				str << std::endl << std::endl;

				if(ctx.getDependencyPhony())
				{
					for(utils::PathVector::iterator dep = converted.begin(); dep != converted.end(); ++dep)
					{
						if(*dep != file)
							str << dep->get() << ":" << std::endl << std::endl;
					}
				}

				str.flush();
				utils::Statistics::instance().addInformation("dependencies(" + file.get() + ")", converted.size());
			}
		}

		utils::Path MsDependencyTracker::lookupPath(const std::string& file)
		{
			utils::Context& ctx = utils::Context::getContext();
			static utils::PathVector incPaths = ctx.getIncludePaths();
			static utils::PathVector sysPaths = ctx.getSysIncludePaths();

			typedef std::pair<bool, utils::Path> cachePair;
			static std::map<std::string, cachePair> cache;

			cachePair& entry = cache[file];

			if(entry.first)
				return entry.second;

			entry.first = true;

			for(utils::PathVector::iterator it = incPaths.begin(); it != incPaths.end(); ++it)
			{
				it->toNative();

				entry.second = *it;
				entry.second.append(file);

				if(entry.second.exists())
					return entry.second;
			}

			if(ctx.getDependencySystem())
			{
				for(utils::PathVector::iterator it = sysPaths.begin(); it != sysPaths.end(); ++it)
				{
					it->toNative();

					entry.second = *it;
					entry.second.append(file);

					if(entry.second.exists())
						return entry.second;
				}
			}

			entry.second = utils::Path();

			if(!ctx.getDependencyGenerated()) {
				utils::Log::verbose("ignoring missing or system %s\n", file.c_str());
			} else {
				dependencies_[file] = true; // directly manipulate to prevent further touching.
			}

			return entry.second;
		}

		void MsDependencyTracker::getIncludes(const utils::Path& file, utils::PathVector& target)
		{
			utils::MappedFile map(file, utils::ModeRead);
			const char* content = reinterpret_cast<char*>(map.getBase());
			const char* top = reinterpret_cast<char*>(map.getTop());

			while(*content != '\0' && content < top)
			{
				while(content < top && (*content == ' ' || *content == '\t' || *content == '\n' || *content == '\r'))
					++content;

				if(content >= top || content+1 >= top || *content == '\0')
					break;

				if(*content++ == '#')
				{
					while(*content == ' ' || *content == '\t')
						++content;

					if(::strncmp(content, "include", 7) == 0)
					{
						/* WARNING: this strchr _may_ be dangerous if the memory mapped file
						 * happens to just be (for example) exactly 4096 bytes long! In this
						 * case, the string is _not_ zero terminated, and accessing one char
						 * beyond the end may (or may not) page fault.
						 *
						 * After reviewing the code, the only situation where this is a prob
						 * is, when the last line of the file is an include statement which
						 * for some reason has no newline at the end of the line, _and_ also
						 * happens to end at byte 4096 ... */

						const char* end = ::strchr(content, '\n');
						const char* open = content + 7;
						const char* close;
						int isComplex = 0;

						/* search for the beginning of the included filename */
						while(open && *open != '\0') {
							if(*open == '"' || *open == '<')
								break;

							if((*open != ' ' && *open != '\t') || open >= top ) {
								//
								// this is triggered by the runtime includes (RUNTIME_INC)
								//
								isComplex = 1;
								utils::Log::verbose("skipping complex include directive %s\n", std::string(open, end).c_str());
								break;
							}

							++open;
						}

						if(!isComplex) {
							if(*open == '"') {
								close = ::strchr(open + 2, '"');
							} else {
								close = ::strchr(open + 2, '>');
							}

							if(open >= end)
								continue;

							std::string inc(open + 1, close - open - 1);

							if(*open == '"')
							{
								utils::Path chk(file.base());
								chk.append(inc);
								if(chk.exists())
									target.push_back(chk);
								else {
									target.push_back(lookupPath(inc));
								}
							} else {
								target.push_back(lookupPath(inc));
							}
						}

						content = end + 1;
					}
				} else {
					//
					// skip until after next EOL
					//
					while(*content != '\n' && *content != '\0' && content < top)
						++content;
				}
			}
		}

		void MsDependencyTracker::trackFile(const utils::Path& file)
		{
			bool& tracked = dependencies_[file];

			if(tracked)
				return;

			bool& locked  = lock_[file];

			if(locked)
			{
				utils::Log::verbose("circular include, %s allready locked!\n", file.get().c_str());
				return;
			}

			locked = true;

			utils::PathVector paths;
			getIncludes(file, paths);

			for(utils::PathVector::iterator it = paths.begin(); it != paths.end(); ++it)
			{
				//
				// if empty, it was a system path which was ignored
				//
				if(it->get().empty())
					continue;

				trackFile(*it);
			}

			tracked = true;
			locked = false;
		}

	}
}

