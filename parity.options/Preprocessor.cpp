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

#include "Preprocessor.h"
#include <Log.h>

namespace parity
{
	namespace options
	{
		bool setKeepComments(const char* option, const char* argument, bool& used)
		{
			utils::Context& ctx = utils::Context::getContext();
			ctx.setKeepComments(true);
			return true;
		}
		
		bool setNoStdIncludes(const char* option, const char* argument, bool& used)
		{
			utils::Context& ctx = utils::Context::getContext();
			ctx.setNoStdIncludes(true);
			return true;
		}

		bool addDefine(const char* option, const char* argument, bool& used)
		{
			utils::Context& ctx = utils::Context::getContext();

			const char* posD = ::strchr(option, 'D');

			if(*(++posD) == '\0')
			{
				posD = argument;
				used = true;
			}

			ctx.setDefinesString(posD);
			return true;
		}

		bool removeDefine(const char* option, const char* argument, bool& used)
		{
			utils::Context& ctx = utils::Context::getContext();

			const char* posU = ::strchr(option, 'U');

			if(*(++posU) == '\0')
			{
				posU = argument;
				used = true;
			}

			utils::DefineMap& ref = ctx.getDefines();
			utils::DefineMap::iterator pos = ref.find(posU);

			if(pos != ref.end()) {
				ref.erase(pos);
			} else {
				utils::Log::verbose("\"%s\" not defined, trying to undefine compiler builtin by passing through!\n", posU);
				std::string val = ctx.getCompilerPassThrough();

				if(!val.empty())
					val.append(" ");

				val.append(std::string(option, 2));
				val.append(posU);

				ctx.setCompilerPassThrough(val);
			}

			return true;
		}

		bool setPreprocess(const char* option, const char* argument, bool& used)
		{
			utils::Context& ctx = utils::Context::getContext();
			ctx.setPreprocess(true);

			if(::strstr(option, "EP"))
				ctx.setKeepHashLine(false);
			else if(::strchr(option, 'P'))
				ctx.setPreprocToFile(true);

			return true;
		}

		bool addIncludePath(const char* option, const char* argument, bool& used)
		{
			utils::Context& ctx = utils::Context::getContext();
			static std::map<utils::Path, bool> cache;
			
			utils::PathVector& vect = ctx.getIncludePaths();
			utils::PathVector& sysvect = ctx.getSysIncludePaths();

			const char* arg = ::strchr(option, 'I');
			
			if(*(++arg) == '\0') {
				arg = argument;
				used = true;
			}

			utils::Path pth(arg);
			pth.toNative();

			bool& cached = cache[pth];

			if(!cached)
				cached = true;
			else
				return true;

			//
			// check for duplicates and for system include paths.
			// since we should never get that many tons of include paths
			// that speed matters.
			//
			for(utils::PathVector::iterator sysp = sysvect.begin(); sysp != sysvect.end(); ++sysp)
			{
				sysp->toNative();

				//
				// TODO: This check stinks...
				//
				if(pth == *sysp) {
					utils::Log::verbose("ignoring system path given on commandline: %s\n", pth.get().c_str());
					return true;
				}
			}

			vect.push_back(pth);
			return true;
		}

		bool setDependencyTracking(const char* option, const char* argument, bool& used)
		{
			utils::Context& ctx = utils::Context::getContext();
			ctx.setDependencyTracking(true);

			if(::strstr(option, "MM"))
				ctx.setDependencySystem(false);

			if(::strstr(option, "MD"))
				ctx.setDependencyOnly(false);
			else
				ctx.setDependencyOnly(true);

			return true;
		}

		bool setDependencyPhony(const char* option, const char* argument, bool& used)
		{
			utils::Context& ctx = utils::Context::getContext();
			ctx.setDependencyPhony(true);
			return true;
		}

		bool setDependencyGenerated(const char* option, const char* argument, bool& used)
		{
			utils::Context& ctx = utils::Context::getContext();
			ctx.setDependencyGenerated(true);
			return true;
		}

		bool setDependencyOutput(const char* option, const char* argument, bool& used)
		{
			utils::Context& ctx = utils::Context::getContext();

			if(!argument)
				throw utils::Exception("%s requires an argument!", option);

			used = true;
			ctx.setDependencyOutput(utils::Path(argument));
			return true;
		}

		bool setDependencyTarget(const char* option, const char* argument, bool& used)
		{
			utils::Context& ctx = utils::Context::getContext();

			if(!argument)
				throw utils::Exception("%s requires an argument!", option);

			used = true;
			ctx.setDependencyTarget(argument);
			return true;
		}

	}
}

