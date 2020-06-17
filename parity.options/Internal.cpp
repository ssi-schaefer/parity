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

#include "Internal.h"
#include "Version.h"
#include <Configuration.h>

#include <iostream>

namespace parity
{
	namespace options
	{
		static const char *helpText =
			"supports many of the common gcc/g++ options and\n"
			"translates them to the host compiler ones as far as possible.\n"
			"\n"
			"everything that can't be mapped accordingly can be forwarded to the\n"
			"compiler or the linker via the -X and -Y options, respectively.\n"
			"\n"
			PACKAGE_NAME " itself has the following options:\n"
            "\n"
            "    -v              show " PACKAGE_NAME "'s configuration\n"
            "    -dbg level      turn on debugging (levels 1 - 4 where 4 is verbose)\n"
            "    -ctxdump        dump " PACKAGE_NAME "'s internal state\n"
            "    -cfg value      set a config string e. g. -cfg 'KeepTemporary=1'\n"
            "                    look for configuration.txt.bz2 in your doc path\n"
            "                    (most probably \$EPREFIX/usr/share/doc/parity*)\n"
            "    -X value        forward value to the compiler\n"
            "    -Y value        forward value to the linker\n"
            "\n"
            "to show your toolchain's help, forward the help-request:\n"
            "\n"
            "    compiler help:  g++ -X /?\n"
            "    linker help:    g++ -Y /?\n"
            "\n"
            "you probably also may want to have a look at " PACKAGE_NAME "'s man page:\n"
            "\n"
            "    man " PACKAGE_NAME "\n"
			;

		bool setDebugLevel(const char* option, const char* argument, bool& used)
		{
			if(!argument)
				throw utils::Exception("%s requires an argument!", option);

			used = true;
			
			utils::Context& ctx = utils::Context::getContext();
			ctx.setDebugLevel(argument);
			return true;
		}

		bool setLinkerPassthrough(const char* option, const char* argument, bool& used)
		{
			if(!argument)
				throw utils::Exception("%s requires an argument!", option);

			used = true;

			utils::Context& ctx = utils::Context::getContext();

			std::string val = ctx.getLinkerPassThrough();
			val.append(" ");
			val.append(argument);

			ctx.setLinkerPassThrough(val);
			return true;
		}

		bool setCompilerPassthrough(const char* option, const char* argument, bool& used)
		{
			if(!argument)
				throw utils::Exception("%s requires an argument!", option);

			used = true;

			utils::Context& ctx = utils::Context::getContext();

			std::string val = ctx.getCompilerPassThrough();
			val.append(" ");
			val.append(argument);

			ctx.setCompilerPassThrough(val);
			return true;
		}

		bool setUnhandledSourceFilePassthrough(const char* option, const char* OPT_UNUSED(argument), bool& OPT_UNUSED(used))
		{
			bool optused = false;
			return setCompilerPassthrough("-X", option, optused);
		}

		bool setUnhandledObjectFilePassthrough(const char* option, const char* OPT_UNUSED(argument), bool& OPT_UNUSED(used))
		{
			bool optused = false;
			return setLinkerPassthrough("-Y", option, optused);
		}

		bool showParityHelp(const char* OPT_UNUSED(option), const char* OPT_UNUSED(argument), bool& OPT_UNUSED(used))
		{
			std::cout << PACKAGE_NAME << " " << PACKAGE_VERSION << " " << "(" << __DATE__ << ") ";
			std::cout << helpText << std::endl;
			exit(0);

			#ifndef _WIN32
			/* never reached (this is there for some gcc versions)! */
			return false;
			#endif
		}

		bool showParityVersion(const char* OPT_UNUSED(option), const char* OPT_UNUSED(argument), bool& OPT_UNUSED(used))
		{
			std::cout << PACKAGE_NAME << " " << PACKAGE_VERSION << " " << "(" << __DATE__ << ") ";
#ifdef _WIN32
			std::cout << "native windows build" << std::endl;
#elif defined(__INTERIX)
			std::cout << "interix g++ build" << std::endl;
#elif defined(__CYGWIN__)
			std::cout << "cygwin g++ build" << std::endl;
#endif
			std::cout << "Copyright (c) 2007 - 2009 Markus Duft <markus.duft@salomon.at>" << std::endl;
			std::cout << PACKAGE_NAME << " comes with ABSOLUTELY NO WARRANTY; This is free software, and you are" << std::endl;
			std::cout << "welcome to redistribute it under certain conditions; see COPYING.LESSER for details." << std::endl;
			std::cout << "Use " << PACKAGE_NAME << " like GNU compilers from the Free Software Foundation." << std::endl;
			exit(0);

			#ifndef _WIN32
			/* never reached (this is there for some gcc versions)! */
			return false;
			#endif
		}

		bool showParityConfig(const char* OPT_UNUSED(option), const char* OPT_UNUSED(argument), bool& OPT_UNUSED(used))
		{
			std::cout << PACKAGE_NAME << " " << PACKAGE_VERSION << " " << "(" << __DATE__ << ") ";
#ifdef _WIN32
			std::cout << "native windows build" << std::endl;
#elif defined(__INTERIX)
			std::cout << "interix g++ build" << std::endl;
#elif defined(__CYGWIN__)
			std::cout << "cygwin g++ build" << std::endl;
#endif

			/* output some configuration values... */
			#ifdef HAVE_CONFIG_H
				std::cout << std::endl;
				std::cout << "Sysconf Directory: " << PARITY_SYSCONFDIR << std::endl;
				std::cout << "Include Directory: " << PARITY_INCLUDEDIR << std::endl;
				std::cout << "Library Directory: " << PARITY_LIBDIR << std::endl;
				std::cout << "Libexec Directory: " << PARITY_LIBEXECDIR << std::endl;
				std::cout << "Binaries Directory: " << PARITY_BINDIR << std::endl;
				std::cout << "Local State Directory: " << PARITY_LOCALSTATEDIR << std::endl;
			#endif

			exit(0);

			#ifndef _WIN32
			/* never reached (this is there for some gcc versions)! */
			return false;
			#endif
		}

		bool printProperty(const char* option, const char* OPT_UNUSED(argument), bool& OPT_UNUSED(used))
		{
			utils::Context& ctx = utils::Context::getContext();
			if (strcmp(option, "-dumpmachine") == 0) {
				std::cout << ctx.getHostTriplet() << std::endl;
				exit(0);
			}
			if (strcmp(option, "-dumpversion") == 0
			 || strcmp(option, "-dumpfullversion") == 0
			) {
				std::cout << ctx.getCompilerVersion() << std::endl;
				exit(0);
			}
			if (strncmp(option, "-print-prog-name=", 17) == 0) {
				option += 17;
				if (*option) {
					utils::Path prog(ctx.getKnownProgramsPath());
					prog.append(option);
					if (prog.isFile()) {
						std::cout << prog.get() << std::endl;
						exit(0);
					}
				}
				std::cout << option << std::endl;
				exit(0);
			}
			if (strcmp(option, "-print-search-dirs") == 0) {
				std::cout << "install: " << PARITY_LIBEXECDIR;
				utils::PathVector const & execPaths = ctx.getAdditionalExecPaths();
				if (!execPaths.empty()) {
					std::cout << "\nprograms: ";
				}
				std::string sep = "="; // seen with gcc
				for(utils::PathVector::const_iterator it = execPaths.begin()
				  ; it != execPaths.end()
				  ; ++it
				) {
					std::cout << sep;
					sep = ":";
					if (it->isNative()) {
						std::cout << it->get();
					} else {
						utils::Path nat(it->get());
						nat.toNative();
						std::cout << nat.get();
					}
				}
				utils::PathVector const & libPaths = ctx.getSysLibraryPaths();
				if (!libPaths.empty()) {
					std::cout << "\nlibraries: ";
				}
				sep = "="; // seen with gcc
				for(utils::PathVector::const_iterator it = libPaths.begin()
				  ; it != libPaths.end()
				  ; ++it
				) {
					std::cout << sep;
					sep = ":";
					if (it->isNative()) {
						std::cout << it->get();
					} else {
						utils::Path nat(it->get());
						nat.toNative();
						std::cout << nat.get();
					}
				}
				std::cout << std::endl;
				exit(0);
			}
			return false;
		}

		bool setCtxDump(const char* OPT_UNUSED(option), const char* OPT_UNUSED(argument), bool& OPT_UNUSED(used))
		{
			utils::Context& ctx = utils::Context::getContext();
			ctx.setCtxDump(true);
			return true;
		}

		bool setColorMode(const char* option, const char* argument, bool& used)
		{
			utils::Context& ctx = utils::Context::getContext();
			
			if(!argument || ::strlen(argument) < 1)
				throw utils::Exception("%s requires an argument!", option);

			if(::strcmp(argument, "bright"))
				ctx.setColorMode(utils::Color::Bright);
			else if(::strcmp(argument, "dark"))
				ctx.setColorMode(utils::Color::Dark);
			else
				throw utils::Exception("%s is not a valid argument to %s", argument, option);

			used = true;

			return true;
		}

		bool addConfigString(const char* option, const char* argument, bool& used)
		{
			if(!argument || ::strlen(argument) < 1)
				throw utils::Exception("%s requires an argument!", option);

			used = true;

			if(!utils::Config::parseString(utils::Context::getContext(), argument, strlen(argument)))
				throw utils::Exception("cannot parse configuration string \"%s\"\n", argument);

			return true;
		}

		bool ignoreOption(const char* option, const char* argument, bool& used)
		{
			return true;
		}

		bool ignoreWithArgument(const char* option, const char* argument, bool& used)
		{
			used = true;
			return true;
		}
	}
}

