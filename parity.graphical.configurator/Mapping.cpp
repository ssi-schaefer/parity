#include "Mapping.h"

using namespace System;
using namespace System::Windows::Forms;
using namespace parity::utils;

#undef CTX_GETSET
#undef CTX_GETSET_C
#undef CTX_GETSET_I
#undef CTX_GETSET_CI
#define CTX_GETSET_I(type, name, init)
#define CTX_GETSET_CI(type, name, init)

#define CTX_GETSET_C(type, name, init) \
	CTX_GETSET(type, name, init)

#undef CTX_GETSET
#define CTX_GETSET(type, name, init) \
	static bool is##name##Default(parity::utils::Context& ctx) { return (type(init) == ctx.get##name()); }

CTX_MEMBERS

#undef CTX_GETSET
#define CTX_GETSET(type, name, init) \
	static System::String^ get##name##ForDisplay(parity::utils::Context& ctx) { return FormatForDisplay(ctx.get##name()); }

CTX_MEMBERS

#undef CTX_GETSET
#define CTX_GETSET(type, name, init) \
	static void edit##name(const MappingStruct* mapping, parity::utils::Context& ctx) { return DisplayEditDialog(mapping, ctx.get##name()); }

CTX_MEMBERS

#undef CTX_GETSET
#define CTX_GETSET(type, name, init) \
	static void save##name(System::IO::TextWriter^ stream, const MappingStruct* mapping, parity::utils::Context& ctx) { return SaveSetting(stream, mapping, ctx.get##name()); }

CTX_MEMBERS

#undef CTX_GETSET
#define CTX_GETSET(type, name, init) \
	{ #name, #type, #init, get##name##ForDisplay, is##name##Default, edit##name, save##name },

static MappingStruct mappingArray[] = {
	CTX_MEMBERS
	{ NULL, NULL, NULL, NULL, NULL, NULL }
};

MappingStruct* SettingMapping = mappingArray;

//
// Formatter functions.
//

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

//
// Edit Dialog functions.
//

void DisplayEditDialog(const MappingStruct* mapping, const bool& val)
{
	MessageBox::Show("Edit Dialog for type bool to be implemented", "Not Implemented", MessageBoxButtons::OK, MessageBoxIcon::Information);
}

void DisplayEditDialog(const MappingStruct* mapping, const long& val)
{
	MessageBox::Show("Edit Dialog for type long to be implemented", "Not Implemented", MessageBoxButtons::OK, MessageBoxIcon::Information);
}

void DisplayEditDialog(const MappingStruct* mapping, const std::string& val)
{
	MessageBox::Show("Edit Dialog for type string to be implemented", "Not Implemented", MessageBoxButtons::OK, MessageBoxIcon::Information);
}

void DisplayEditDialog(const MappingStruct* mapping, const parity::utils::Path& val)
{
	MessageBox::Show("Edit Dialog for type path to be implemented", "Not Implemented", MessageBoxButtons::OK, MessageBoxIcon::Information);
}

void DisplayEditDialog(const MappingStruct* mapping, const parity::utils::DefineMap& val)
{
	MessageBox::Show("Edit Dialog for type define map to be implemented", "Not Implemented", MessageBoxButtons::OK, MessageBoxIcon::Information);
}

void DisplayEditDialog(const MappingStruct* mapping, const parity::utils::PathVector& val)
{
	MessageBox::Show("Edit Dialog for type path vector  to be implemented", "Not Implemented", MessageBoxButtons::OK, MessageBoxIcon::Information);
}

void DisplayEditDialog(const MappingStruct* mapping, const parity::utils::LanguageType& val)
{
	MessageBox::Show("Edit Dialog for type language type to be implemented", "Not Implemented", MessageBoxButtons::OK, MessageBoxIcon::Information);
}

void DisplayEditDialog(const MappingStruct* mapping, const parity::utils::SubsystemType& val)
{
	MessageBox::Show("Edit Dialog for type subsystem type to be implemented", "Not Implemented", MessageBoxButtons::OK, MessageBoxIcon::Information);
}

void DisplayEditDialog(const MappingStruct* mapping, const parity::utils::ToolchainType& val)
{
	MessageBox::Show("Edit Dialog for type toolchain type to be implemented", "Not Implemented", MessageBoxButtons::OK, MessageBoxIcon::Information);
}

void DisplayEditDialog(const MappingStruct* mapping, const parity::utils::RuntimeType& val)
{
	MessageBox::Show("Edit Dialog for type runtime type to be implemented", "Not Implemented", MessageBoxButtons::OK, MessageBoxIcon::Information);
}

void DisplayEditDialog(const MappingStruct* mapping, const parity::utils::Color::ColorMode& val)
{
	MessageBox::Show("Edit Dialog for type color mode to be implemented", "Not Implemented", MessageBoxButtons::OK, MessageBoxIcon::Information);
}

//
// Saving Functions.
//

void SaveSetting(System::IO::TextWriter^ stream, const MappingStruct* mapping, const bool& val)
{
	stream->Write("# Name: {0}, Default Value: {1}, Type {2}\n", gcnew String(mapping->Name), gcnew String(mapping->Default), gcnew String(mapping->Type));
	stream->Write("{0} = {1}\n\n", gcnew String(mapping->Name), val ? "yes" : "no");
}

void SaveSetting(System::IO::TextWriter^ stream, const MappingStruct* mapping, const long& val)
{
}

void SaveSetting(System::IO::TextWriter^ stream, const MappingStruct* mapping, const std::string& val)
{
}

void SaveSetting(System::IO::TextWriter^ stream, const MappingStruct* mapping, const parity::utils::Path& val)
{
}

void SaveSetting(System::IO::TextWriter^ stream, const MappingStruct* mapping, const parity::utils::DefineMap& val)
{
}

void SaveSetting(System::IO::TextWriter^ stream, const MappingStruct* mapping, const parity::utils::PathVector& val)
{
}

void SaveSetting(System::IO::TextWriter^ stream, const MappingStruct* mapping, const parity::utils::LanguageType& val)
{
}

void SaveSetting(System::IO::TextWriter^ stream, const MappingStruct* mapping, const parity::utils::SubsystemType& val)
{
}

void SaveSetting(System::IO::TextWriter^ stream, const MappingStruct* mapping, const parity::utils::ToolchainType& val)
{
}

void SaveSetting(System::IO::TextWriter^ stream, const MappingStruct* mapping, const parity::utils::RuntimeType& val)
{
}

void SaveSetting(System::IO::TextWriter^ stream, const MappingStruct* mapping, const parity::utils::Color::ColorMode& val)
{
}

