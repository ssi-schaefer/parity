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

#include <Log.h>
#include <Context.h>
#include <Timing.h>

#include <TableGnuGcc.h>

#include <CollectorStubs.h>
#include <CollectorOther.h>

#include <Version.h>

#ifdef _WIN32
#  include <io.h>
#  include <fcntl.h>
#endif

//
// when building with parity itself, we need to be carfull, since
// all MS header files are included with _POSIX_ defined, and thus
// some things are different (fex. we need to use "old" POSIX names).
//
#ifdef __PARITY__
# define _fileno fileno
#endif

using namespace parity::utils;
using namespace parity;

//
// The following is needed for Confix to get the Executable
// name right:
// CONFIX:EXENAME('parity.gnu.gcc')
//

int main(int argc, char** argv)
{
#ifdef _WIN32
	//
	// set stream modes to binary to suppress the annoying \r
	//
	_setmode(_fileno(stdout), _O_BINARY);
	_setmode(_fileno(stderr), _O_BINARY);
#endif

	parity::tasks::runConfigurationLoading(argc, argv);

	//
	// Get out context which is needed for nearly every operation.
	//
	Context& context = Context::getContext();

	//
	// After config loading: need color mode
	//
	Color color(context.getColorMode());

	//
	// Initialize the Argument parser with the correct frontend
	//
	Timing::instance().start("Command Line Processing");

	//
	// we're gcc here, so set frontend appropriatly, overruling configuration
	//
	context.setFrontendType(utils::ToolchainInterixGNU);

	try {
		char * argv0 = strrchr(argv[0], '/'); // find last path sep
		if (argv0 == NULL) {
			argv0 = strrchr(argv[0], '\\'); // find last path sep
		}
		if (argv0 == NULL) {
			argv0 = argv[0]; // no path component
		} else {
			++argv0; // skip last path sep
		}
		if (strstr(argv0, "g++") != NULL) {
			context.setDefaultLanguage(utils::LanguageCpp);
		} else {
			context.setDefaultLanguage(utils::LanguageC);
		}

		/* backup configuration-set paths. */
		parity::utils::PathVector cfgIncludePaths = parity::utils::Context::getContext().getIncludePaths();
		parity::utils::PathVector cfgLibraryPaths = parity::utils::Context::getContext().getLibraryPaths();

		/* clear configuration-set paths. */
		parity::utils::Context::getContext().getIncludePaths().clear();
		parity::utils::Context::getContext().getLibraryPaths().clear();

		/* Avoid interference with the command line set library paths. If configuration set
		 * library paths are still set, the order will be wrong (command line set paths
		 * should take precedence. For this reason (as system library paths are already set),
		 * configuration set library paths are prepended to the system library paths. this
		 * way we end up with the desired result.
		 * The old method was to append the paths to library paths again after command line
		 * parsing, which broke, because during command line parsing, library search is already
		 * done, which may not work if the configuration set paths are absent. */
		parity::utils::Context::getContext().getSysIncludePaths().insert(
			parity::utils::Context::getContext().getSysIncludePaths().begin(),
			cfgIncludePaths.begin(), cfgIncludePaths.end());
		parity::utils::Context::getContext().getSysLibraryPaths().insert(
			parity::utils::Context::getContext().getSysLibraryPaths().begin(),
			cfgLibraryPaths.begin(), cfgLibraryPaths.end());

		parity::options::UnknownArgumentVector unknown;
		parity::options::CommandLine::process(argc - 1, &argv[1], parity::options::OptionTableGnuGcc, &unknown);

		for(parity::options::UnknownArgumentVector::iterator it = unknown.begin(); it != unknown.end(); ++it) {
			parity::utils::Path pth(*it);
			pth.toNative();

			if(pth.exists()) {
				if(context.getPreprocess() || context.getCompileOnly()) {
					// assume source file for all unknown things...
					Log::verbose("assuming source: %s\n", pth.get().c_str());
					context.setSourcesString(pth.get());
				} else {
					Log::verbose("assuming object: %s\n", pth.get().c_str());
					context.setObjectsLibrariesString(pth.get());
				}
			} else {
				Log::verbose("ignoring unknown argument: %s\n", it->c_str());
			}
		}
	} catch(const Exception& e) {
		Log::error("while processing command line: %s\n", e.what());
		exit(1);
	}

	//
	// setup default defines.
	//
	context.setDefinesString("__PARITY_GNU__");
	context.setDefinesString("__PARITY__");

	Timing::instance().stop("Command Line Processing");

	//
	// Check if there is something to do.
	//
	if(context.getCompileOnly() && context.getSources().empty()) {
		Log::error("no source files to compile!\n");
		exit(1);
	}

	//
	// Initialization complete, startup can proceed.
	//
	Log::verbose("%s\n", color.yellow(PACKAGE_STRING).c_str());
	Log::verbose("entering compiler stage...\n");

	tasks::runCompilerStage();

	//
	// If we get here, we've successfully compiled (if we compiled)
	// and are sure that we're not in compile only mode, etc. So we
	// can go ahead with linking.
	//

	if(context.getObjectsLibraries().empty())
	{
		//
		// This should barely ever happen, since ObjectsLibraries contains the
		// default libraries set by the parity configuration.
		//
		Log::error("no input files for linking stage, did the compiler fail?");
		exit(1);
	}

	Log::verbose("entering linker stage...\n");

	tasks::runLinkerStage();

	return 0;
}


