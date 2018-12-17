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

#include "TableGnuWindres.h"

#include "Internal.h"
#include "Misc.h"
#include "Optimize.h"
#include "Preprocessor.h"
#include "Linker.h"
#include "Compiler.h"
#include "Windres.h"

namespace parity
{
	namespace options
	{
		static ArgumentTableEntry OptionTableGnuWindresArr[] = {
			//
			// parity internal command line switches.
			//
			{ "-dbg"				,setDebugLevel				},
			{ "-X"					,setCompilerPassthrough		},
			{ "-V"					,showParityVersion			},
			{ "--version"			,showParityVersion			},
			{ "-ctxdump"			,setCtxDump					},
			{ "-cfg"				,addConfigString			},
			//
			// GCC command line switches
			//
			{ "--input-format"		,setInputFormat				},
			{ "--input"				,addSource					},
			{ "--output-format"		,setOutputFormat			},
			{ "--output"			,setOutputFile				},
			{ "-J"					,setInputFormat				},
			{ "-i"					,addSource					},
			{ "-O"					,setOutputFormat			},
			{ "-o"					,setOutputFile				},
			{ "-F"					,setBfdTarget				},
			{ "--target"			,setBfdTarget				},
			{ "--preprocessor-arg"	,ignoreWithArgument			},
			{ "--preprocessor"		,ignoreWithArgument			},
			{ "-I"					,addIncludePath				},
			{ "--include-dir"		,addIncludePath				},
			{ "-D"					,addDefine					},
			{ "--define"			,addDefine					},
			{ "-U"					,removeDefine				},
			{ "--undefine"			,removeDefine				},
			{ "-r"					,ignoreOption				},
			{ "-v"					,ignoreOption				},
			{ "--yydebug"			,ignoreOption				},
			{ "-c"					,setCodepage				},
			{ "--codepage"			,setCodepage				},
			{ "-l"					,setLanguage				},
			{ "--language"			,setLanguage				},
			{ "--use-temp-file"		,ignoreOption				},
			{ "--no-use-temp-file"	,ignoreOption				},

			{ "-"					,addSourceFromStdin			},
			{ "@"					,setCommandFile				},

			// --- must be last ---
			{ 0				,0							}
		};

		ArgumentTableEntry* OptionTableGnuWindres = OptionTableGnuWindresArr;
	}
}

