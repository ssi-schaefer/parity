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

#include "TableMsCl.h"

#include "Internal.h"
#include "Misc.h"
#include "Optimize.h"
#include "Preprocessor.h"
#include "Linker.h"
#include "Compiler.h"
#include "CommandLine.h"

namespace parity
{
	namespace options
	{
		static ArgumentTableEntry OptionTableMsClArr[] = {
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
			// CL and LINK command line switches
			//
			{ "-o"			,setOutputFile				}, // deprecated, but still in use
			{ "/link"		,setLinkerPassthrough		},
			{ "/X"			,setNoStdIncludes			},
			{ "/D"			,addDefine					},
			{ "/U"			,removeDefine				},
			{ "/c"			,setCompileOnly				},
			{ "/LD"			,setSharedLink				},
			{ "/EH"			,setExceptionHandling		},
			{ "/GR"			,setRuntimeTypes			},
			{ "/E"			,setPreprocess				},
			{ "/P"			,setPreprocess				},
			{ "/I"			,addIncludePath				},
			{ "/LIBPATH:"	,addLibraryPath				},
			{ "/Zi"			,setDebugable				},
			{ "-g"			,setDebugable				},
			{ "/Fo"			,setOutputFile				},
			{ "/Fe"			,setOutputFile				},
			{ "/TP"			,setForcedLanguage			},
			{ "/TC"			,setForcedLanguage			},
			{ "/SUBSYSTEM:"	,setSubsystem				},
			{ "/subsystem:"	,setSubsystem				},
			{ "/std:"		,setCxxStandard				},
			{ "/Zc:__cplusplus"	,setCxxStandard			},
			{ "/MT"			,setRuntime					},
			{ "/MD"			,setRuntime					},
			{ ".lib"		,addObjectsLibraries		},
			{ ".Lib"		,addObjectsLibraries		},
			{ ".LIB"		,addObjectsLibraries		},
			{ ".obj"		,addObjectsLibraries		},
			{ ".Obj"		,addObjectsLibraries		},
			{ ".OBJ"		,addObjectsLibraries		},
			{ ".o"			,addObjectsLibraries		},
			{ ".a"			,addObjectsLibraries		},
			{ ".so"			,addObjectsLibraries		},
			{ ".c"			,addSource					},
			{ ".cc"			,addSource					},
			{ ".i"			,addSource					},
			{ ".ii"			,addSource					},
			{ ".cpp"		,addSource					},
			{ ".cxx"		,addSource					},
			{ ".C"			,addSource					},
			{ ".asm"		,addSource					},
			{ ".s"			,addSource					},
			{ ".S"			,addSource					},
			{ "/Tp"			,addSource					},
			{ "/Tc"			,addSource					},
			{ "@"			,setCommandFile				},

			// --- must be last ---
			{ 0				,0							}
		};

		ArgumentTableEntry* OptionTableMsCl = OptionTableMsClArr;
	}
}

