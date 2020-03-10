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

#include "TableGnuGcc.h"

#include "Internal.h"
#include "Misc.h"
#include "Optimize.h"
#include "Preprocessor.h"
#include "Linker.h"
#include "Compiler.h"

namespace parity
{
	namespace options
	{
		static ArgumentTableEntry OptionTableGnuGccArr[] = {
			//
			// parity internal command line switches.
			//
			{ "-dbg"		,setDebugLevel				},
			{ "-X"			,setCompilerPassthrough		},
			{ "-Y"			,setLinkerPassthrough		},	// GCC knows -Xlinker here.
			{ "--version"	,showParityVersion			},
			{ "-v"			,showParityConfig			},
			{ "-ctxdump"	,setCtxDump					},
			{ "-cfg"		,addConfigString			},
			//
			// GCC command line switches
			//
			{ "-C"			,setKeepComments			},
			{ "-dumpmachine", printProperty				},
			{ "-dumpversion", printProperty				},
			{ "-dumpfullversion", printProperty			},
			{ "-print-prog-name=", printProperty		},
			{ "-print-search-dirs", printProperty		},
			{ "-O"			,setOptimizeLevel			},
			{ "-fomit-fr"	,setOmitFramePointer		},
			{ "-finline"	,setInlineFunctions			},
			{ "-fno-inlin"	,setInlineFunctions			},
			{ "-msse"		,setUseSSE					},
			{ "-msse2"		,setUseSSE2					},
			{ "-ffor-sc"	,setForScope				},
			{ "-fno-for"	,setForScope				},
			{ "-fms-ext"	,setMsExtensions			},
			{ "-fno-ms-"	,setMsExtensions			},
			{ "-ansi"		,setAnsiMode				},
			{ "-Wl,-rpath"	,addRunPath					},  // must be before -W to match correctly
			{ "-Wl,-e"		,setEntryPoint				},  // -- " --
			{ "-Wl,--out-implib",setOutImplib			},  // must be before -W to match correctly
			{ "-Werror"		,setWarningAsError			},	// -- " --
			{ "-w"			,setWarningLevel			},
			{ "-W"			,setWarningLevel			},
			{ "-fsyntax"	,setSyntaxOnly				},
			{ "-fnostdinc"	,setNoStdIncludes			},
			{ "-D"			,addDefine					},
			{ "-U"			,removeDefine				},
			{ "-c"			,setCompileOnly				},
			{ "-fexcept"	,setExceptionHandling		},
			{ "-fno-except"	,setExceptionHandling		},
			{ "-frtti"		,setRuntimeTypes			},
			{ "-fno-rtti"	,setRuntimeTypes			},
			{ "-shared"		,setSharedLink				},
			{ "-fpic"		,setPositionIndep			},
			{ "-fPIC"		,setPositionIndep			},
			{ "-E"			,setPreprocess				},
			{ "-I"			,addIncludePath				},
			{ "-include"	,addIncludeFile				},
			{ "-L"			,addLibraryPath				},
			{ "-static"		,setPreferStatic			},
			{ "-g"			,setDebugable				},
			{ "-x"			,setForcedLanguage			},
			{ "-o"			,setOutputFile				},
			{ "-ansi"		,setCxxStandard				},
			{ "-std="		,setCxxStandard				},
			{ "-subsys"		,setSubsystem				},
			{ "-fshort-wc"	,setShortWchar				},
			{ "-fnoshort-w"	,setShortWchar				},
			{ "-MM"			,setDependencyTracking		},
			{ "-MD"			,setDependencyTracking		},
			{ "-MMD"		,setDependencyTracking		},
			{ "-MT"			,setDependencyTarget		},
			{ "-MF"			,setDependencyOutput		},
			{ "-MG"			,setDependencyGenerated		},
			{ "-MP"			,setDependencyPhony			},
			{ "-M"			,setDependencyTracking		},	// must be after all other -M* to not match!
			{ "-rpath"		,addRunPath					},
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
			{ ".def"		,addSource					},
			{ ".Def"		,addSource					},
			{ ".DEF"		,addSource					},
			{ ".res"		,setUnhandledObjectFilePassthrough	},
			{ "-"			,addSourceFromStdin			},
			// --- must be last ---
			{ 0				,0							}
		};

		ArgumentTableEntry* OptionTableGnuGcc = OptionTableGnuGccArr;
	}
}

