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

#include "ContextGen.h"
#include "Context.h"
#include "Log.h"
#include "Color.h"
#include "Exception.h"

#include <cstdio>

#ifndef _WIN32
#  define _stricmp strcasecmp
#endif

namespace parity
{
	namespace utils
	{
		#undef CTX_GETSET_I
		#undef CTX_GETSET_CI
		#undef  CTX_GETSET_C
		#define CTX_GETSET_C(type, name, init) CTX_GETSET(type, name, init)
		#define CTX_GETSET_I(type, name, init) CTX_GETSET(type, name, init)
		#define CTX_GETSET_CI(type, name, init) CTX_GETSET(type, name, init)

		#undef  CTX_GETSET
		#define CTX_GETSET(type, name, init) name(init),

		ContextGen::ContextGen()
			: CTX_MEMBERS
			initTerminator(0)
		{
		}

		#undef  CTX_GETSET
		#define CTX_GETSET(type, name, init) if(name != other.name) return false;

		bool ContextGen::operator==(const ContextGen& other)
		{
			CTX_MEMBERS;
			return true;
		}

		#undef  CTX_GETSET
		#define CTX_GETSET(type, name, init) name = other.name;

		ContextGen& ContextGen::operator= (const ContextGen& other)
		{
			CTX_MEMBERS;
			return *this;
		}

		void ContextGen::convert(bool& target, const std::string& val)
		{
			if(val == "1" || val == "yes" || val == "on" || val == "true")
				target = true;
			else if(val == "0" || val == "no" || val == "off" || val == "false")
				target = false;
			else
				throw Exception("cannot convert %s to a boolean value!", val.c_str());
		}

		void ContextGen::convert(Color::ColorMode &target, const std::string &val)
		{
			if(_stricmp(val.c_str(), "bright") == 0)
				target = Color::Bright;
			else if(_stricmp(val.c_str(), "dark") == 0)
				target = Color::Dark;
			else
				throw Exception("cannot convert %s to a valid ColorMode!", val.c_str());

			//
			// Somehow hackish, but this seems the only location where i'm
			// able to set this, without allways polling from context.
			//
			Log::setColor(Color(target));
		}

		void ContextGen::convert(ToolchainType& target, const std::string& val)
		{
			if(_stricmp(val.c_str(), "microsoft") == 0
				|| _stricmp(val.c_str(), "ms") == 0)
				target = ToolchainMicrosoft;
			else if(_stricmp(val.c_str(), "interixms") == 0)
				target = ToolchainInterixMS;
			else if(_stricmp(val.c_str(), "gnu") == 0
				|| _stricmp(val.c_str(), "gcc") == 0)
				target = ToolchainInterixGNU;
			else
				throw Exception("cannot convert %s to a valid ToolchainType!", val.c_str());
		}

		void ContextGen::convert(SubsystemType& target, const std::string& val)
		{
			if(_stricmp(val.c_str(), "console") == 0)
				target = SubsystemWindowsCui;
			else if(_stricmp(val.c_str(), "windows") == 0)
				target = SubsystemWindowsGui;
			else if(_stricmp(val.c_str(), "windowsce") == 0
				|| _stricmp(val.c_str(), "wince") == 0)
				target = SubsystemWindowsCeGui;
			else if(_stricmp(val.c_str(), "posix") == 0)
				target = SubsystemPosixCui;
			else
				throw Exception("cannot convert %s to a valid SubsystemType!", val.c_str());
		}

		void ContextGen::convert(DefineMap& target, const std::string& ref)
		{
			std::string key;
			std::string value;

			std::string::size_type pos_equals = ref.find("=");

			if(pos_equals == std::string::npos) {
				key = ref;
			} else {
				key = ref.substr(0, pos_equals);
				value = ref.substr(pos_equals + 1);
			}

			target[key] = value;
		}

		void ContextGen::convert(PathVector& target, const std::string& ref)
		{
			target.push_back(ref);
		}

		void ContextGen::convert(RuntimeType& target, const std::string& ref)
		{
			if(ref == "dynamic")
				target = RuntimeDynamic;
			else if(ref == "static")
				target = RuntimeStatic;
			else if(ref == "dynamic debug")
				target = RuntimeDynamicDebug;
			else if(ref == "static debug")
				target = RuntimeStaticDebug;
			else
				throw Exception("cannot convert %s to a valid RuntimeType!", ref.c_str());
		}

		void ContextGen::convert(LanguageType& target, const std::string& ref)
		{
			//
			// LanguageAssembler intentionally left out
			//

			if(ref == "c" || ref == "C")
				target = LanguageC;
			else if(ref == "c++" || ref == "C++")
				target = LanguageCpp;
			else if(ref == "resource")
				target = LanguageResource;
			else
				throw Exception("cannot convert %s to a valid LanguageType!", ref.c_str());
		}

		void ContextGen::convert(SourceMap& target, const std::string& ref)
		{
			Context& ctx = Context::getContext();

			if(ref.compare(ref.length() - 2, 2, ".c") == 0
				|| ref.compare(ref.length() - 2, 2, ".i") == 0)
				if(ctx.getForcedLanguage() != LanguageInvalid) {
					Log::verbose("adding forced source file (originally c): %s\n", ref.c_str());
					target[ref] = ctx.getForcedLanguage();
				} else {
					Log::verbose("adding c source file: %s\n", ref.c_str());
					target[ref] = LanguageC;
				}
			else if(ref.compare(ref.length() - 3, 3, ".cc") == 0
				|| ref.compare(ref.length() - 4, 4, ".cpp") == 0
				|| ref.compare(ref.length() - 4, 4, ".cxx") == 0
				|| ref.compare(ref.length() - 3, 3, ".ii") == 0
				|| ref.compare(ref.length() - 2, 2, ".C") == 0) {
				if(ctx.getForcedLanguage() != LanguageInvalid) {
					Log::verbose("adding forced source file (originally c++): %s\n", ref.c_str());
					target[ref] = ctx.getForcedLanguage();
				} else {
					Log::verbose("adding c++ source file: %s\n", ref.c_str());
					target[ref] = LanguageCpp;
				}
			} else if(ref.compare(ref.length() - 4, 4, ".asm") == 0
				|| ref.compare(ref.length() - 2, 2, ".s") == 0
				|| ref.compare(ref.length() - 2, 2, ".S") == 0) {
				if(ctx.getForcedLanguage() != LanguageInvalid)
					Log::warning("ignoring forced language for assembler, continuing normally!\n");
				Log::verbose("adding assembler source file: %s\n", ref.c_str());
				target[ref] = LanguageAsssembler;
			} else if (_stricmp(ref.substr(ref.length() - 3).c_str(), ".rc") == 0) {
				Log::verbose("adding resource file: %s\n", ref.c_str());
				target[ref] = LanguageResource;
			} else if (_stricmp(ref.substr(ref.length() - 4).c_str(), ".res") == 0) {
				Log::verbose("adding compiled resource file: %s\n", ref.c_str());
				target[ref] = LanguageCompiledResource;
			} else if (_stricmp(ref.substr(ref.length() - 4).c_str(), ".def") == 0) {
				Log::verbose("adding module definition file: %s\n", ref.c_str());
				target[ref] = LanguageModuleDefinition;
			} else {
				target[ref] = LanguageUnknown;
			}
		}

		std::string ContextGen::printable(const long& val)
		{
			char buf[255];
			::sprintf(buf, "%ld", val);
			std::string ret(buf);
			::sprintf(buf, "%lx", val);
			ret.append(" (0x");
			ret.append(buf);
			ret.append(")");
			
			return ret;
		}

		std::string ContextGen::printable(const ToolchainType& val)
		{
			switch(val)
			{
			case ToolchainInterixGNU:
				return "GCC";
			case ToolchainMicrosoft:
				return "Microsoft";
			case ToolchainInterixMS:
				return "Interix Microsoft";
			case ToolchainInvalid:
				return "Invalid";
			}

			return "Unknown";
		}

		std::string ContextGen::printable(const Color::ColorMode& val)
		{
			switch(val)
			{
			case Color::Bright:
				return "Bright";
			case Color::Dark:
				return "Dark";
			}

			return "Unknown";
		}

		std::string ContextGen::printable(const DefineMap& val)
		{
			std::string res;

			if(val.empty())
				return "(empty map)";
			else {
				res = "(map)";
			}

			Color col(getColorMode());

			for(DefineMap::const_iterator it = val.begin(); it != val.end(); ++it)
			{
				res.append(col.red("\n   * "));
				res.append(col.cyan("<"));
				res.append(it->first);
				if(it->second.empty()) {
					res.append(col.cyan(">"));
				} else {
					res.append(", ");
					res.append(it->second);
					res.append(col.cyan(">"));
				}
			}

			return res;
		}

		std::string ContextGen::printable(const PathVector& val)
		{
			std::string res;

			if(val.empty())
				return "(empty vector)";
			else
				res = "(vector)";

			Color col(getColorMode());

			for(PathVector::const_iterator it = val.begin(); it != val.end(); ++it)
			{
				res.append(col.red("\n   * "));
				if(it->isNative()) res.append(col.yellow("[NATIVE] "));
				else if(it->isForeign()) res.append(col.yellow("[FOREIGN] "));
				else res.append(col.yellow("[NEITHER] ")); // is ok, if windows parity build and unix path with windows backend
				res.append(it->get());
			}

			return res;
		}

		std::string ContextGen::printable(const RuntimeType& val)
		{
			switch(val)
			{
			case RuntimeDynamic:
				return "dynamic";
				break;
			case RuntimeInvalid:
				return "invalid";
				break;
			case RuntimeStatic:
				return "static";
				break;
			case RuntimeDynamicDebug:
				return "dynamic debug";
				break;
			case RuntimeStaticDebug:
				return "static debug";
				break;
			}
			return "unknown";
		}

		std::string ContextGen::printable(const LanguageType& val)
		{
			//
			// LanguageAssembler intentionally left out, since it may not be a forced language
			//

			switch(val)
			{
			case LanguageC:
				return "C";
			case LanguageCpp:
				return "C++";
			case LanguageModuleDefinition:
				return "DEF";
			case LanguageResource:
				return "RC";
			case LanguageCompiledResource:
				return "RES";
			case LanguageUnknown:
			case LanguageInvalid:
				return "None";
			default:
				//
				// invalid, just because of GCC warning
				//
				return "Invalid";
			}

			return "unknown";
		}

		std::string ContextGen::printable(const SubsystemType& val)
		{
			switch(val)
			{
			case SubsystemWindowsCui:
				return "Console";
			case SubsystemPosixCui:
				return "POSIX";
			case SubsystemWindowsGui:
				return "Windows";
			case SubsystemWindowsCeGui:
				return "Windows CE";
			case SubsystemUnknown:
				return "Invalid";
			default:
				return "Not Supported";
			}

			return "unknown";
		}

		std::string ContextGen::printable(const SourceMap& val)
		{
			Color col(getColorMode());
			std::string ret;

			if(val.empty())
				return "(empty map)";
			else
				ret = "(map)";

			for(SourceMap::const_iterator it = val.begin(); it != val.end(); ++it)
			{
				ret.append("\n   * ");
				switch(it->second)
				{
				case LanguageAsssembler:
					ret.append(col.blue("[ASM] "));
					break;
				case LanguageC:
					ret.append(col.blue("[ C ] "));
					break;
				case LanguageCpp:
					ret.append(col.blue("[C++] "));
					break;
				case LanguageModuleDefinition:
					ret.append(col.blue("[DEF] "));
					break;
				case LanguageResource:
					ret.append(col.blue("[RC ] "));
					break;
				case LanguageCompiledResource:
					ret.append(col.blue("[RES] "));
					break;
				default:
					throw Exception("disallowed source file type found in %s: %d", it->first.get().c_str(), it->second);
				}
				ret.append(it->first.get());
			}

			return ret;
		}

	}
}

