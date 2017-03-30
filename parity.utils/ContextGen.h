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

#ifndef __CONTEXTGEN_H__
#define __CONTEXTGEN_H__

#include <map>
#include <vector>
#include <string>
#include <cstdlib>

#include "Path.h"
#include "Color.h"
#include "MappedFile.h"

#define CTX_GETSET_SETTER_GETTER(type, name)                                                  \
		type& get##name() { return name; }													  \
		const type& get##name() const { return name; }                                        \
		void set##name(const type& val) { name = val; }                                       \

#define CTX_GETSET_SETTER_GETTER_COMPLEX(type, name)                                          \
		type& get##name() { return name; }                                                    \
		void set##name(const type& val) { name = val; }                                       \

#define CTX_GETSET(type, name, init)                                                          \
	public:                                                                                   \
		CTX_GETSET_SETTER_GETTER(type, name)                                                  \
		void set##name##String(const std::string& val) { convert(name, val); }                \
	private:                                                                                  \
		type name;                                                                            \

#define CTX_GETSET_C(type, name, init)                                                        \
	public:                                                                                   \
		CTX_GETSET_SETTER_GETTER_COMPLEX(type, name)                                          \
		void set##name##String(const std::string& val) { convert(name, val); }                \
	private:                                                                                  \
		type name;                                                                            \

//
// The GETSET_I and GETSET_CI are for members that should not be setable by configuration
//
#define CTX_GETSET_I(type, name, init) CTX_GETSET(type, name, init)
#define CTX_GETSET_CI(type, name, init) CTX_GETSET_C(type, name, init)

namespace parity
{
	namespace utils
	{
		typedef enum {
			ToolchainInvalid = -1,
			ToolchainMicrosoft,
			ToolchainInterixGNU,
			ToolchainInterixMS
		} ToolchainType;

		typedef enum {
			LanguageInvalid = -1,
			LanguageUnknown,
			LanguageC,
			LanguageCpp,
			LanguageAsssembler,
			LanguageModuleDefinition, // .def
			LanguageResource, // .rc
			LanguageCompiledResource // .res
		} LanguageType;

		typedef enum {
			SubsystemUnknown				= 0,
			SubsystemNative					= 1,
			SubsystemWindowsGui				= 2,
			SubsystemWindowsCui				= 3,
			SubsystemPosixCui				= 7,
			SubsystemWindowsCeGui			= 9,
			SubsystemEFIApplication			= 10,
			SubsystemEFIBootServiceDriver	= 11,
			SubsystemEFIRuntimeDriver		= 12,
			SubsystemEFIROM					= 13,
			SubsystemXbox					= 14
		} SubsystemType;

		typedef enum {
			RuntimeInvalid = -1,
			RuntimeStatic,
			RuntimeDynamic,
			RuntimeStaticDebug,
			RuntimeDynamicDebug
		} RuntimeType;

		typedef std::map<std::string, std::string> DefineMap;
		typedef std::map<Path, LanguageType> SourceMap;
		typedef std::vector<Path> PathVector;

		class ContextGen {
			friend class Context;

		public:
			//
			// Convert methods called by generated members.
			//

			void convert(ToolchainType& target, const std::string& ref);
			void convert(Path& target, const std::string& ref) { target = Path(ref); }
			void convert(bool& target, const std::string& ref);
			void convert(Color::ColorMode& target, const std::string& ref);
			void convert(long& target, const std::string& ref) { target = ::atol(ref.c_str()); }
			void convert(std::string& target, const std::string& ref) { target = ref; }
			void convert(DefineMap& target, const std::string& ref);
			void convert(PathVector& target, const std::string& ref);
			void convert(LanguageType& target, const std::string& ref);
			void convert(SubsystemType& target, const std::string& ref);
			void convert(SourceMap& target, const std::string& ref);
			void convert(RuntimeType& target, const std::string& ref);

			std::string printable(const bool& val) { return (val ? "true" : "false"); }
			std::string printable(const ToolchainType& val);
			std::string printable(const Path& val) { return val.get(); }
			std::string printable(const long& val);
			std::string printable(const Color::ColorMode& val);
			std::string printable(const std::string& val) { return val.empty() ? "(empty)" : val; }
			std::string printable(const DefineMap& val);
			std::string printable(const PathVector& val);
			std::string printable(const LanguageType& val);
			std::string printable(const SubsystemType& val);
			std::string printable(const SourceMap& val);
			std::string printable(const RuntimeType& val);

			bool operator==(const ContextGen& other);
			bool operator!=(const ContextGen& other) { return !operator ==(other); }
			ContextGen& operator= (const ContextGen& other);

		private:
			ContextGen();
			virtual ~ContextGen() { }

			//
			// The following is the Context Member Table. From this information
			// all member variables, getters, setters, a complete mapping table
			// for the configuration parser and the initializer list for the
			// Constructor are generated with some preprocessor meta-programming.
			// (isn't THIS cool?)
			//

			// --------------------------------------------------------------------
			//               <type>          <name>              <default value>
			// --------------------------------------------------------------------
			#define CTX_MEMBERS \
				CTX_GETSET_I (ToolchainType	,FrontendType		,ToolchainInterixGNU) /* internal only */ \
				CTX_GETSET   (ToolchainType	,BackendType		,ToolchainMicrosoft)/* internal only */ \
				CTX_GETSET   (bool			,CtxDump			,false			)  /* internal only */ \
				CTX_GETSET   (bool			,Colored			,true			)  /* internal only */ \
				CTX_GETSET   (Color::ColorMode,ColorMode		,Color::Bright	)  /* internal only */ \
				CTX_GETSET   (bool			,GatherSystem		,false			)  /* internal only */ \
				CTX_GETSET   (bool			,GenerateLoader		,true			)  /* internal only */ \
				CTX_GETSET   (bool			,IgnoreForeignLibs	,true			)  /* internal only */ \
				CTX_GETSET   (bool			,LazyLoading		,false			)  /* internal only */ \
				CTX_GETSET   (Path			,ParityLoader		,""				)  /* internal only */ \
				CTX_GETSET   (Path			,PCRTLibrary		,""				)  /* internal only */ \
				CTX_GETSET   (Path			,PCRTInclude		,""				)  /* internal only */ \
				CTX_GETSET   (std::string	,EntryPoint			,""				)  /* internal only */ \
				CTX_GETSET   (std::string	,SharedEntryPoint	,""				)  /* internal only */ \
				CTX_GETSET   (PathVector	,RunPaths			,PathVector()	)  /* internal only */ \
				CTX_GETSET   (bool			,KeepTemporary		,false			)  /* internal only */ \
				CTX_GETSET   (bool			,TimingShowTasks	,true			)  /* internal only */ \
				CTX_GETSET_CI(PathVector	,TemporaryFiles		,PathVector()	)  /* internal only */ \
				CTX_GETSET   (Path			,StatisticsFile		,""				)  /* internal only */ \
				CTX_GETSET   (bool			,IgnoreOutputFilters,false			)  /* internal only */ \
				CTX_GETSET   (PathVector	,AdditionalExecPaths,PathVector()	)  /* internal only */ \
				CTX_GETSET   (bool			,GeneratePCRTEntry	,true			)  /* internal only */ \
				CTX_GETSET   (bool			,GenerateSymbols	,false			)  /* internal only */ \
				CTX_GETSET   (bool			,UseCommandScripts	,true           )  /* internal (MS) */ \
				CTX_GETSET_I (std::string	,CompilerPassThrough,""				)  /* internal (compiler) */ \
				CTX_GETSET_I (std::string	,LinkerPassThrough	,""				)  /* internal (linker) */ \
				CTX_GETSET   (bool			,ExportAll			,false			)  /* internal (linker) */ \
				CTX_GETSET   (bool			,ExportFromExe		,false			)  /* internal (linker) */ \
				CTX_GETSET_I (bool			,Preprocess			,false			)  /* preprocessor */ \
				CTX_GETSET   (bool			,KeepComments		,false			)  /* preprocessor */ \
				CTX_GETSET   (bool			,NoStdIncludes		,false			)  /* preprocessor */ \
				CTX_GETSET_C (DefineMap		,Defines			,DefineMap()	)  /* preprocessor */ \
				CTX_GETSET_C (PathVector	,IncludePaths		,PathVector()	)  /* preprocessor */ \
				CTX_GETSET_C (PathVector	,SysIncludePaths	,PathVector()	)  /* preprocessor (by config only) */ \
				CTX_GETSET_I (bool			,PreprocToFile		,false			)  /* preprocessor (MS) */ \
				CTX_GETSET_I (bool			,KeepHashLine		,true			)  /* preprocessor (MS) */ \
				CTX_GETSET_I (bool			,DependencyTracking	,false			)  /* preprocessor (GCC) */ \
				CTX_GETSET_I (bool			,DependencyOnly		,false			)  /* preprocessor (GCC) */ \
				CTX_GETSET_I (bool			,DependencySystem	,false			)  /* preprocessor (GCC) */ \
				CTX_GETSET_I (bool			,DependencyPhony	,false			)  /* preprocessor (GCC) */ \
				CTX_GETSET_I (bool			,DependencyGenerated,false			)  /* preprocessor (GCC) */ \
				CTX_GETSET_I (Path			,DependencyOutput	,""				)  /* preprocessor (GCC) */ \
				CTX_GETSET_I (std::string	,DependencyTarget	,""				)  /* preprocessor (GCC) */ \
				CTX_GETSET   (long			,OptimizeLevel		,0				)  /* optimization */ \
				CTX_GETSET   (bool			,OmitFramePointer	,false			)  /* optimization */ \
				CTX_GETSET   (bool			,InlineFunctions	,false			)  /* optimization */ \
				CTX_GETSET   (bool			,UseSSE				,false			)  /* feature */ \
				CTX_GETSET   (bool			,UseSSE2			,false			)  /* feature */ \
				CTX_GETSET   (bool			,ForScope			,true			)  /* feature */ \
				CTX_GETSET   (bool			,MsExtensions		,true			)  /* feature */ \
				CTX_GETSET   (long			,WarningLevel		,3				)  /* feature */ \
				CTX_GETSET   (bool			,WarningAsError		,false			)  /* feature */ \
				CTX_GETSET_I (bool			,SyntaxOnly			,false			)  /* compiler */ \
				CTX_GETSET_I (bool			,CompileOnly		,false			)  /* compiler */ \
				CTX_GETSET   (bool			,ExceptionHandling	,true			)  /* compiler */ \
				CTX_GETSET   (bool			,RuntimeTypes		,true			)  /* compiler */ \
				CTX_GETSET_I (bool			,Debugable			,false			)  /* compiler */ \
				CTX_GETSET_CI(SourceMap		,Sources			,SourceMap()	)  /* compiler */ \
				CTX_GETSET   (bool			,ShortWchar			,false			)  /* compiler */ \
				CTX_GETSET   (std::string	,CompilerDefaults	,"/nologo"		)  /* compiler */ \
				CTX_GETSET   (std::string	,AssemblerDefaults	,"/nologo /Cp"	)  /* compiler */ \
				CTX_GETSET   (Path			,CompilerExe		,""				)  /* compiler */ \
				CTX_GETSET   (Path			,AssemblerExe		,""				)  /* compiler (assembler) */ \
				CTX_GETSET   (bool			,TimeT32Bit			,true			)  /* compiler (MS) */ \
				CTX_GETSET   (RuntimeType	,Runtime			,RuntimeDynamic	)  /* compiler (MS) */ \
				CTX_GETSET   (LanguageType	,ForcedLanguage		,LanguageInvalid)  /* compiler (GCC) */ \
				CTX_GETSET   (bool			,AnsiMode			,false			)  /* compiler (GCC) */ \
				CTX_GETSET_I (bool			,PositionIndep		,false			)  /* compiler (GCC) */ \
				CTX_GETSET   (Path			,ResourceCompilerExe,""				)  /* resource compiler */ \
				CTX_GETSET   (Path			,ResourceConverterExe,""			)  /* resource compiler */ \
				CTX_GETSET_I (std::string	,InputFormat		,""				)  /* resource compiler */ \
				CTX_GETSET_I (std::string	,OutputFormat		,""				)  /* resource compiler */ \
				CTX_GETSET   (std::string	,BfdTarget			,""				)  /* resource compiler */ \
				CTX_GETSET_I (std::string	,Codepage			,""				)  /* resource compiler */ \
				CTX_GETSET_I (std::string	,Language			,""				)  /* resource compiler */ \
				CTX_GETSET   (long			,StackReserve		,10485760		)  /* linker (default: 10MB) */ \
				CTX_GETSET   (long			,StackCommit		,65536			)  /* linker (default: 64KB) */ \
				CTX_GETSET   (long			,HeapReserve		,10485760		)  /* linker (default: 10MB) */ \
				CTX_GETSET   (long			,HeapCommit			,65536			)  /* linker (default: 64KB) */ \
				CTX_GETSET_I (bool			,SharedLink			,false			)  /* linker */ \
				CTX_GETSET_C (PathVector	,LibraryPaths		,PathVector()	)  /* linker */ \
				CTX_GETSET   (std::string	,LinkerSharedDefaults	,"/nologo /incremental:no"		)  /* linker */ \
				CTX_GETSET   (std::string	,LinkerDefaults		,"/nologo /incremental:no /fixed:no"		)  /* linker */ \
				CTX_GETSET   (Path			,LinkerExe			,""				)  /* linker */ \
				CTX_GETSET_C (PathVector	,SysLibraryPaths	,PathVector()	)  /* linker (by config only) */ \
				CTX_GETSET_C (PathVector	,ObjectsLibraries	,PathVector()	)  /* linker (must be after syslib paths)*/ \
				CTX_GETSET   (bool			,ForceLink			,false			)  /* linker (MS) */ \
				CTX_GETSET   (SubsystemType	,Subsystem			,SubsystemWindowsCui) /* linker (MS) */ \
				CTX_GETSET   (Path			,ManifestExe		,""				)  /* linker (MS) */ \
				CTX_GETSET   (std::string	,ManifestDefaults	,"-nologo"		)  /* linker (MS) */ \
				CTX_GETSET_I (bool			,PreferStatic		,false			)  /* linker (GCC) */ \
				CTX_GETSET   (Path			,DefaultOutput		,"a.exe"		)  /* all */ \
				CTX_GETSET_I (Path			,OutputFile			,"a.exe"		)  /* all */ \
                CTX_GETSET   (bool          ,WaitForOutputFile  ,true           )  /* all */ \
				CTX_GETSET   (std::string	,DefaultConfigSection	,"default"	)  /* configuration */
			// --------------------------------------------------------------------

			//
			// First instanciation of the above definitions.
			// There are two other instanciations in Context.cpp with different
			// definitions of CTX_GETSET to produce different code for the
			// mapping table and the initializer list of the constructor.
			//
			CTX_MEMBERS

		private:
			//
			// This member is required for the initializer list.
			//
			const int initTerminator;
		};
	}
}

#endif

