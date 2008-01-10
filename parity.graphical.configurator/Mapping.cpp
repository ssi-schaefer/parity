#include "Display.h"

using namespace System;
using namespace parity::utils;

#undef CTX_GETSET
#undef CTX_GETSET_C
#undef CTX_GETSET_I
#undef CTX_GETSET_CI
#define CTX_GETSET_I(type, name, init)
#define CTX_GETSET_CI(type, name, init)

#define CTX_GETSET_C(type, name, init) \
	CTX_GETSET(type, name, init)

#define CTX_GETSET(type, name, init) \
	static bool is##name##Default(parity::utils::Context& ctx) { return (type(init) == ctx.get##name()); }

CTX_MEMBERS

#undef CTX_GETSET
#define CTX_GETSET(type, name, init) \
	static System::String^ get##name##ForDisplay(parity::utils::Context& ctx) { return FormatForDisplay(ctx.get##name()); }

CTX_MEMBERS

#undef CTX_GETSET
#define CTX_GETSET(type, name, init) \
	{ #name, #type, #init, get##name##ForDisplay, is##name##Default },

static DisplayMappingStruct mappingArray[] = {
	CTX_MEMBERS
	{ NULL, NULL, NULL, NULL }
};

DisplayMappingStruct* DisplayMapping = mappingArray;

System::String^ FormatForDisplay(const bool& val) 
{ 
	return val ? gcnew String("yes") : gcnew String("no"); 
}

System::String^ FormatForDisplay(const long& val) 
{ 
	return Convert::ToString(val); 
}

System::String^ FormatForDisplay(const std::string& val) 
{ 
	return gcnew String(val.c_str()); 
}

System::String^ FormatForDisplay(const parity::utils::Path& val)
{
	return gcnew String(val.get().c_str());
}

System::String^ FormatForDisplay(const parity::utils::DefineMap& val)
{
	if(val.size() > 0)
		return "(Collection)";
	else
		return "(Empty Collection)";
}

//System::String^ FormatForDisplay(const parity::utils::SourceMap& val)
//{
//}

System::String^ FormatForDisplay(const parity::utils::PathVector& val)
{
	if(val.size() > 0)
		return "(Collection)";
	else
		return "(Empty Collection)";
}

System::String^ FormatForDisplay(const parity::utils::LanguageType& val)
{
	switch(val) {
		case LanguageAsssembler:
			return "Assembler";
		case LanguageC:
			return "C";
		case LanguageCpp:
			return "C++";
	}

	return "Unknown";
}

System::String^ FormatForDisplay(const parity::utils::SubsystemType& val)
{
	switch(val) {
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

System::String^ FormatForDisplay(const parity::utils::ToolchainType& val)
{
	switch(val)	{
	case ToolchainInterixGNU:
		return "GNU (GCC)";
	case ToolchainMicrosoft:
		return "Microsoft";
	case ToolchainInvalid:
		return "Invalid";
	}

	return "Unknown";
}

System::String^ FormatForDisplay(const parity::utils::RuntimeType& val)
{
	switch(val)	{
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

System::String^ FormatForDisplay(const parity::utils::Color::ColorMode& val)
{
	switch(val)	{
	case Color::Bright:
		return "Bright";
	case Color::Dark:
		return "Dark";
	}

	return "Unknown";
}

