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

#include "TableMsLink.h"

#include "Internal.h"
#include "Misc.h"
#include "Linker.h"

namespace parity
{
	namespace options
	{
		static ArgumentTableEntry OptionTableMsLinkArr[] = {
			//
			// parity internal command line switches.
			//
			{ "-dbg"		,setDebugLevel				},
			{ "-X"			,setCompilerPassthrough		},
			{ "-Y"			,setLinkerPassthrough		},
			{ "-v"			,showParityVersion			},
			{ "-ctxdump"	,setCtxDump					},
			{ "-cfg"		,addConfigString			},
			//
			// LINK command line switches
			//
			{ "/DLL"		,setSharedLink				},
			{ "/dll"		,setSharedLink				},
			{ "/LIBPATH:"	,addLibraryPath				},
			{ "/libpath:"	,addLibraryPath				},
			{ "/DEBUG"		,setDebugable				},
			{ "/debug"		,setDebugable				},
			{ "/OUT:"		,setOutputFile				},
			{ "/out:"		,setOutputFile				},
			{ "/SUBSYSTEM:"	,setSubsystem				},
			{ "/subsystem:"	,setSubsystem				},
			{ "/IMPLIB:"	,setOutImplib				},
			{ "/implib:"	,setOutImplib				},
			{ ".lib"		,addObjectsLibraries		},
			{ ".Lib"		,addObjectsLibraries		},
			{ ".LIB"		,addObjectsLibraries		},
			{ ".obj"		,addObjectsLibraries		},
			{ ".Obj"		,addObjectsLibraries		},
			{ ".OBJ"		,addObjectsLibraries		},
			{ ".o"			,addObjectsLibraries		},
			{ ".a"			,addObjectsLibraries		},
			{ ".so"			,addObjectsLibraries		},
			{ "@"			,setCommandFile				},

			// --- must be last ---
			{ 0				,0							}
		};

		ArgumentTableEntry* OptionTableMsLink = OptionTableMsLinkArr;
	}
}

