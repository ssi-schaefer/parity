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

#include "Context.h"

#include "Log.h"
#include "Exception.h"
#include "Timing.h"
#include "Environment.h"
#include "Statistics.h"

#include <iomanip>

namespace parity
{
	namespace utils
	{
		static Context gContext;

		#undef  CTX_GETSET_C
		#undef CTX_GETSET_I
		#undef CTX_GETSET_CI
		#define CTX_GETSET_C(type, name, init) CTX_GETSET(type, name, init)
		#define CTX_GETSET_I(type, name, init)
		#define CTX_GETSET_CI(type, name, init)

		#undef	CTX_GETSET
		#define CTX_GETSET(type, name, init) { #name, &Context::set##name##String },

		const ContextMap Context::mapping_[] = {
			//
			// Explicit (manual) members
			//
			{ "DebugLevel", &Context::setDebugLevel },
			//
			// Implicit members... defined in Context.h
			//
			CTX_MEMBERS
			{ "", 0 }
		};

		Context::Context()
			: ContextGen()
		{
			#if defined(_WIN32) && !defined(HAVE_CONFIG_H)
				//
				// on native windows disable color by default (can still
				// be enabled by config (Color=on)) since it doesn't look
				// too good on windows terminals...
				//
				// on native windows built with autotools (HAVE_CONFIG_H),
				// we can enable color by default, since we can assume, that
				// parity is installed in some interix (cygwin...) environment.
				//

				Colored = false;
			#endif
		}

		Context::~Context()
		{
			dump(true);

			//
			// Do cleanup of temporary files and so on here
			//
			cleanup();
		}

		void Context::cleanup()
		{
			if(!TemporaryFiles.empty())
			{
				if(KeepTemporary)
				{
					Log::verbose("please manually clean the following files:\n");

					for(PathVector::iterator it = TemporaryFiles.begin(); it != TemporaryFiles.end(); ++it)
					{
						Log::verbose(" * %s\n", it->get().c_str());
					}

					return;
				}

				Log::verbose("cleaning %d files.\n", TemporaryFiles.size());

				//
				// TODO: allow wildcards!
				//
				for(PathVector::iterator it = TemporaryFiles.begin(); it != TemporaryFiles.end(); ++it)
				{
					it->toNative();

					if(it->exists()) {
						//
						// need to change mode to be able to delete
						//
						it->mode(0777);

						if(!it->remove())
							Log::warning("cannot cleanup %s!\n", it->get().c_str());
					} else {
						Log::warning("temporary file missing: %s\n", it->get().c_str());
					}
				}
			}
		}

		Context& Context::getContext()
		{
			return gContext;
		}

		const ContextMap* Context::getMapping(const std::string& ref) const
		{
			const ContextMap* ptr = mapping_;

			while(ptr->setter)
			{
				if(ptr->name == ref)
					return ptr;

				++ptr;
			}

			return 0;
		}

		void Context::setDebugLevel(const std::string& ref)
		{
			if(ref == "1" || ref == "error")
				utils::Log::setLevel(utils::Log::Error);
			else if(ref == "2" || ref == "warning")
				utils::Log::setLevel(utils::Log::Warning);
			else if(ref == "3" || ref == "profile")
				utils::Log::setLevel(utils::Log::Profile);
			else if(ref == "4" || ref == "verbose")
				utils::Log::setLevel(utils::Log::Verbose);
			else
				throw utils::Exception("Invalid Debug Level: %s", ref.c_str());
		}

		void Context::dump(bool intern)
		{
			if(!getCtxDump())
				return;

			Color col(Color::Bright);

			#undef CTX_GETSET_I
			#undef CTX_GETSET_CI
			#define CTX_GETSET_I(type, name, init) if(intern) { CTX_GETSET(type, name, init) }
			#define CTX_GETSET_CI(type, name, init) if(intern) { CTX_GETSET(type, name, init) }

			#undef  CTX_GETSET
			#define CTX_GETSET(type, name, init) utils::Log::verbose("%s%-20s%s%s%s%*s: %s\n", \
				col.green(" * ").c_str(), #name, col.cyan("(").c_str(), #type, col.cyan(")").c_str(), \
				20 - strlen(#type), "", printable(name).c_str());

			utils::Log::verbose("\n%s\n", col.red("   Automatic Dump of Context Members:").c_str());
			utils::Log::verbose("%s\n", col.red("   ----------------------------------").c_str());

			CTX_MEMBERS
		}

		void Context::setDefaultOutput(const Path& val)
		{
			//
			// This is overridden from ContextGen to be able to set
			// the output filename to the new default filename if it
			// still is the same.
			//

			if(getDefaultOutput().get() == getOutputFile().get())
				setOutputFile(val);

			DefaultOutput = val;
		}

#if 0
		bool Context::isBadLinkerPath(const Path &pth)
		{
			const std::string& str = pth.get();

			if(BackendType == ToolchainMicrosoft)
			{
				if(		str.find("usr/lib") != std::string::npos
					||	str.find("usr/local/lib") != std::string::npos
					||	str.find("usr\\lib") != std::string::npos
					||	str.find("usr\\local\\lib") != std::string::npos)
					return true;
			}

			return false;
		}

		bool Context::isBadCompilerPath(const Path& pth)
		{
			const std::string& str = pth.get();

			if(BackendType == ToolchainMicrosoft) {
				if(		str.find("usr/include") != std::string::npos
					||	str.find("usr/local/include") != std::string::npos
					||	str.find("usr\\include") != std::string::npos
					||	str.find("usr\\local\\include") != std::string::npos)
					return true;
			}

			return false;
		}
#endif

		Path Context::lookupLibrary(const std::string& name, bool isMinusL)
		{
			std::vector<std::string> names;

			if(isMinusL)
			{
				if(!PreferStatic)
				{
					names.push_back(name + ".so");
					names.push_back("lib" + name + ".so");
				}
				names.push_back(name + ".a");
				names.push_back("lib" + name + ".a");
				names.push_back(name + ".lib");
				names.push_back("lib" + name + ".lib");

				//
				// Implicit libraries are looked up through minusL,
				// so this must be here, that a complete library name
				// is found too.
				//
				names.push_back(name);
			} else {
				Path direct(name);
				direct.toNative();
				if(direct.exists())
					return direct;

				names.push_back(name);
			}

			static utils::Environment envLibPath("LIBRARY_PATH");
			static utils::Environment envLdLibPath("LD_LIBRARY_PATH");
			static utils::PathVector pthLibPath = envLibPath.getPathVector();
			static utils::PathVector pthLdLibPath = envLdLibPath.getPathVector();

			utils::PathVector vecArray[] = {
				LibraryPaths,
				SysLibraryPaths,
				pthLibPath,
				pthLdLibPath,
			};

			for(int i = 0; i < 4; ++i)
			{
				for(PathVector::iterator it = vecArray[i].begin(); it != vecArray[i].end(); ++it)
				{
					for(std::vector<std::string>::iterator lib = names.begin(); lib != names.end(); ++lib)
					{
						it->toNative();

						Path pth(*it);
						pth.append(*lib);
#if 0
						if(pth.exists() && !isBadLinkerPath(pth))
#else
						if(pth.exists())
#endif
							return pth;
					}
				}
			}

			throw utils::Exception("cannot find %s in any of the %d library paths", name.c_str(), LibraryPaths.size() + SysLibraryPaths.size());
		}

		void Context::setObjectsLibrariesString(const std::string& val)
		{
			//
			// This is overridden from Context to be able to lookup
			// a library given from any source (command line or config)
			// since both call this function (command line manually through
			// the options::setObjectsLibraries function and config
			// automatically through generated mapping.
			//

			std::string arg;
			bool isMinusL = false;

			if((val[0] == '-' || val[0] == '/') && val[1] == 'l')
			{
				if(val[2])
					arg = val.substr(2);
				else {
					throw Exception("missing directly attached library name to -l");
				}
				isMinusL = true;
			} else {
				arg = val;
			}

			try {
				Path pth = lookupLibrary(arg, isMinusL);
				ObjectsLibraries.push_back(pth);

				Statistics::instance().addInformation("file-binary", pth.get());
			} catch(const Exception& e) {
				Log::error("lookup of %s failed: %s\n", arg.c_str(), e.what());
				exit(1);
			}
		}

		void Context::setStatisticsFileString(const std::string& val)
		{
			setStatisticsFile(utils::Path(val));
			Statistics::instance().start();
		}

		std::string Context::calculateDefaultEntrypoint()
		{

			//
			// try to figure out default entry points.
			//
			if(getSharedLink())
			{
				//
				// entry point is __stdcall and has three arguments ("@12")
				//
				switch(getSubsystem())
				{
				case SubsystemWindowsCui:
					if(getBackendType() == ToolchainInterixMS)
						return "__DllMainCRTStartup@12";
					/* Fallthrough */
				case SubsystemWindowsGui:
				case SubsystemWindowsCeGui:
					return "_DllMainCRTStartup@12";
				case SubsystemPosixCui:
					return "__DllMainCRTStartup@12";
				default:
					throw Exception("subsystem default entry point not implemented!");
				}
			} else {
				//
				// entry point without arguments or return, and __cdecl
				//
				switch(getSubsystem())
				{
				case SubsystemWindowsCui:
					if(getBackendType() == ToolchainInterixMS)
						return "__MixedModeProcessStartup";
					else
						return "mainCRTStartup";
				case SubsystemWindowsGui:
				case SubsystemWindowsCeGui:
					return "WinMainCRTStartup";
				case SubsystemPosixCui:
					return "__PosixProcessStartup";
				default:
					throw Exception("subsystem default entry point not implemented!");
				}
			}
		}

	}
}

