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

#include <TableGnuWindres.h>

#include <CollectorStubs.h>
#include <CollectorOther.h>

#include <Version.h>
#include <Compiler.h>

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
// CONFIX:EXENAME('parity.gnu.windres')
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
	// we're windres here, so set frontend appropriatly, overruling configuration
	//
	context.setFrontendType(utils::ToolchainInterixGNU);

	//
	// treat the '-' (source from stdin) argument as .rc
	//
	context.setForcedLanguageString("resource");

	try {
		/* backup configuration-set paths. */
		parity::utils::PathVector cfgIncludePaths = parity::utils::Context::getContext().getIncludePaths();

		/* clear configuration-set paths. */
		parity::utils::Context::getContext().getIncludePaths().clear();

		/* ATTENTION: see parity.gnu.gcc.cpp for comments on this: */
		parity::utils::Context::getContext().getSysIncludePaths().insert(
			parity::utils::Context::getContext().getSysIncludePaths().begin(),
			cfgIncludePaths.begin(), cfgIncludePaths.end());

		parity::options::UnknownArgumentVector unknown;
		parity::options::CommandLine::process(argc - 1, &argv[1], parity::options::OptionTableGnuWindres, &unknown);

		for(parity::options::UnknownArgumentVector::iterator it = unknown.begin(); it != unknown.end(); ++it) {
			parity::utils::Path pth(*it);
			pth.toNative();

			if (context.getSources().empty()) {
				Log::verbose("assuming source: %s\n", pth.get().c_str());
				context.setSourcesString(pth.get());
			} else
			if (context.getOutputFile() == context.getDefaultOutput()) {
				Log::verbose("assuming output: %s\n", pth.get().c_str());
				context.setOutputFileString(pth.get());
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
	if(context.getSources().empty()) {
		bool used = false;
		options::addSourceFromStdin("-", "", used);
	}

	//
	// Initialization complete, startup can proceed.
	//
	Log::verbose("%s\n", color.yellow(PACKAGE_STRING).c_str());
	Log::verbose("entering resource compiler stage...\n");

	tasks::runResourceCompilerStage();

	return 0;
}


