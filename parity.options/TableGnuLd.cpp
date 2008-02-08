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

#include "TableGnuLd.h"

#include "Internal.h"
#include "Misc.h"
#include "Linker.h"

namespace parity
{
	namespace options
	{
		static ArgumentTableEntry OptionTableGnuLdArr[] = {
			//
			// parity internal command line switches.
			//
			{ "-dbg"		,setDebugLevel				},
			{ "-X"			,setCompilerPassthrough		},
			{ "-Y"			,setLinkerPassthrough		},	// GCC knows -Xlinker here.
			{ "-v"			,showParityVersion			},
			{ "-ctxdump"	,setCtxDump					},
			//
			// GCC command line switches
			//
			{ "-L"			,addLibraryPath				},
			{ "-Bstatic"	,setPreferStatic			},
			{ "-Bdynamic"	,setPreferStatic			},
			{ "-shared"		,setSharedLink				},
			{ "-g"			,setDebugable				},
			{ "-o"			,setOutputFile				},
			{ "-subsys"		,setSubsystem				},
			{ "-rpath"		,addRunPath					},
			{ "-e"			,setEntryPoint				},
			{ "-l"			,addObjectsLibraries		},
			{ ".lib"		,addObjectsLibraries		},
			{ ".Lib"		,addObjectsLibraries		},
			{ ".LIB"		,addObjectsLibraries		},
			{ ".obj"		,addObjectsLibraries		},
			{ ".Obj"		,addObjectsLibraries		},
			{ ".OBJ"		,addObjectsLibraries		},
			{ ".o"			,addObjectsLibraries		},
			{ ".a"			,addObjectsLibraries		},
			{ ".so"			,addObjectsLibraries		},
			// --- must be last ---
			{ 0				,0							}
		};

		ArgumentTableEntry* OptionTableGnuLd = OptionTableGnuLdArr;
	}
}

