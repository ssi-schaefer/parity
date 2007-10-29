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

#include <TableMsCl.h>

#include <CollectorStubs.h>
#include <CollectorOther.h>

#include <Version.h>

#ifdef _WIN32
#  include <io.h>
#  include <fcntl.h>
#endif

using namespace parity::utils;
using namespace parity;

//
// The following is needed for Confix to get the Executable
// name right:
// CONFIX:EXENAME('parity.ms.cl')
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

	parity::tasks::runConfigurationLoading();

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
	context.setFrontendType(utils::ToolchainMicrosoft);

	try {
		if(context.getBackendType() == utils::ToolchainMicrosoft)
		{
			parity::options::UnknownArgumentVector vec;
			parity::options::CommandLine::process(argc - 1, &argv[1], parity::options::OptionTableMsCl, &vec);

			if(vec.size() > 0)
			{
				for(parity::options::UnknownArgumentVector::iterator it = vec.begin(); it != vec.end(); ++it)
					context.setLinkerPassThrough(context.getLinkerPassThrough() + " " + *it);
			}
		} else {
			throw utils::Exception("Microsoft frontend may only be used with Microsoft backend!");
		}
	} catch(const Exception& e) {
		Log::error("while processing command line: %s\n", e.what());
		exit(1);
	}

	//
	// setup default defines.
	//
	context.setDefinesString("__PARITY_MS__");
	context.setDefinesString("__PARITY__");

	Timing::instance().stop("Command Line Processing");

	//
	// Check if there is something to do.
	//
	if(context.getCompileOnly() && context.getSources().empty()) {
		Log::error("no source files to compile!\n");
		exit(1);
	}

	if(context.getCompilerExe().get().empty() || context.getAssemblerExe().get().empty())
	{
		Log::error("compiler or assembler executable not set by configuration, cannot continue!\n");
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
		Log::error("no input files for linking stage, did the compiler fail?");
		exit(1);
	}

	Log::verbose("entering linker stage...\n");

	if(context.getLinkerExe().get().empty())
	{
		Log::error("linker executable not set by configuration, cannot continue!\n");
		exit(1);
	}
	
	tasks::runLinkerStage();

	return 0;
}

