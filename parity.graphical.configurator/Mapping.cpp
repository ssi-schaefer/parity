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

// hackish: need it after the CTX_MEMBERS stuff to work
#include "SimpleStringEditDialog.h"
#include "DefineMapEditDialog.h"
#include "PathVectorEditDialog.h"
#include "Configurator.h"
using namespace paritygraphicalconfigurator;

//
// Formatter functions.
//

System::String^ FormatForDisplay(const bool& val) 
{ 
	return val ? gcnew String("true") : gcnew String("false"); 
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
	case parity::utils::Color::Bright:
		return "Bright";
	case parity::utils::Color::Dark:
		return "Dark";
	}

	return "Unknown";
}

//
// Edit Dialog functions.
//

void DisplayEditDialog(const MappingStruct* mapping, bool& val)
{
	SimpleStringEditDialog^ dlg = gcnew SimpleStringEditDialog();
	dlg->Value = gcnew String(Context::getContext().printable(val).c_str());

	if(dlg->ShowDialog() == DialogResult::OK)
	{
		Context::getContext().convert(val, Configurator::MarshalSimpleStringToNative(dlg->Value));
	}
}

void DisplayEditDialog(const MappingStruct* mapping, long& val)
{
	SimpleStringEditDialog^ dlg = gcnew SimpleStringEditDialog();
	dlg->Value = gcnew String(Context::getContext().printable(val).c_str());

	if(dlg->ShowDialog() == DialogResult::OK)
	{
		val = Convert::ToInt32(dlg->Value);
	}
}

void DisplayEditDialog(const MappingStruct* mapping, std::string& val)
{
	SimpleStringEditDialog^ dlg = gcnew SimpleStringEditDialog();
	dlg->Value = gcnew String(Context::getContext().printable(val).c_str());

	if(dlg->ShowDialog() == DialogResult::OK)
	{
		val = Configurator::MarshalSimpleStringToNative(dlg->Value);
	}
}

void DisplayEditDialog(const MappingStruct* mapping, parity::utils::Path& val)
{
	SimpleStringEditDialog^ dlg = gcnew SimpleStringEditDialog();
	dlg->Value = gcnew String(Context::getContext().printable(val).c_str());

	if(dlg->ShowDialog() == DialogResult::OK)
	{
		val = Path(Configurator::MarshalSimpleStringToNative(dlg->Value));
	}
}

void DisplayEditDialog(const MappingStruct* mapping, parity::utils::DefineMap& val)
{
	DefineMap orig = val;
	DefineMapEditDialog^ dlg = gcnew DefineMapEditDialog(val);

	if(dlg->ShowDialog() != DialogResult::OK)
	{
		val = orig;
	}
}

void DisplayEditDialog(const MappingStruct* mapping, parity::utils::PathVector& val)
{
	PathVector orig = val;
	PathVectorEditDialog^ dlg = gcnew PathVectorEditDialog(val);

	if(dlg->ShowDialog() != DialogResult::OK)
	{
		val = orig;
	}
}

void DisplayEditDialog(const MappingStruct* mapping, parity::utils::LanguageType& val)
{
	SimpleStringEditDialog^ dlg = gcnew SimpleStringEditDialog();
	dlg->Value = gcnew String(Context::getContext().printable(val).c_str());

	if(dlg->ShowDialog() == DialogResult::OK)
	{
		Context::getContext().convert(val, Configurator::MarshalSimpleStringToNative(dlg->Value));
	}
}

void DisplayEditDialog(const MappingStruct* mapping, parity::utils::SubsystemType& val)
{
	SimpleStringEditDialog^ dlg = gcnew SimpleStringEditDialog();
	dlg->Value = gcnew String(Context::getContext().printable(val).c_str());

	if(dlg->ShowDialog() == DialogResult::OK)
	{
		Context::getContext().convert(val, Configurator::MarshalSimpleStringToNative(dlg->Value));
	}
}

void DisplayEditDialog(const MappingStruct* mapping, parity::utils::ToolchainType& val)
{
	SimpleStringEditDialog^ dlg = gcnew SimpleStringEditDialog();
	dlg->Value = gcnew String(Context::getContext().printable(val).c_str());

	if(dlg->ShowDialog() == DialogResult::OK)
	{
		Context::getContext().convert(val, Configurator::MarshalSimpleStringToNative(dlg->Value));
	}
}

void DisplayEditDialog(const MappingStruct* mapping, parity::utils::RuntimeType& val)
{
	SimpleStringEditDialog^ dlg = gcnew SimpleStringEditDialog();
	dlg->Value = gcnew String(Context::getContext().printable(val).c_str());

	if(dlg->ShowDialog() == DialogResult::OK)
	{
		Context::getContext().convert(val, Configurator::MarshalSimpleStringToNative(dlg->Value));
	}
}

void DisplayEditDialog(const MappingStruct* mapping, parity::utils::Color::ColorMode& val)
{
	SimpleStringEditDialog^ dlg = gcnew SimpleStringEditDialog();
	dlg->Value = gcnew String(Context::getContext().printable(val).c_str());

	if(dlg->ShowDialog() == DialogResult::OK)
	{
		Context::getContext().convert(val, Configurator::MarshalSimpleStringToNative(dlg->Value));
	}
}

//
// Saving Functions.
//

void SaveSetting(System::IO::TextWriter^ stream, const MappingStruct* mapping, const bool& val)
{
	stream->Write("{0} = {1}\n\n", gcnew String(mapping->Name), val ? "yes" : "no");
}

void SaveSetting(System::IO::TextWriter^ stream, const MappingStruct* mapping, const long& val)
{
	stream->Write("{0} = {1}\n\n", gcnew String(mapping->Name), val);
}

void SaveSetting(System::IO::TextWriter^ stream, const MappingStruct* mapping, const std::string& val)
{
	stream->Write("{0} = {1}\n\n", gcnew String(mapping->Name), gcnew String(val.c_str()));
}

void SaveSetting(System::IO::TextWriter^ stream, const MappingStruct* mapping, const parity::utils::Path& val)
{
	if(gcnew String(mapping->Name) == "DefaultOutput")
		stream->Write("{0} = {1}\n\n", gcnew String(mapping->Name), gcnew String(val.file().c_str()));
	else
		stream->Write("{0} = {1}\n\n", gcnew String(mapping->Name), gcnew String(val.get().c_str()));
}

void SaveSetting(System::IO::TextWriter^ stream, const MappingStruct* mapping, const parity::utils::DefineMap& val)
{
	for(DefineMap::const_iterator it = val.begin(); it != val.end(); ++it)
	{
		stream->Write("{0} = {1}", gcnew String(mapping->Name), gcnew String(it->first.c_str()));

		if(!it->second.empty())
			stream->Write("={0}", gcnew String(it->second.c_str()));

		stream->Write("\n");
	}
}

void SaveSetting(System::IO::TextWriter^ stream, const MappingStruct* mapping, const parity::utils::PathVector& val)
{
	bool fnOnly = false;

	if(gcnew String(mapping->Name) == "ObjectsLibraries")
		fnOnly = true;

	for(PathVector::const_iterator it = val.begin(); it != val.end(); ++it)
	{
		stream->Write("{0} = {1}\n", gcnew String(mapping->Name), fnOnly ? gcnew String(it->file().c_str()) : gcnew String(it->get().c_str()));
	}

	stream->Write("\n");
}

void SaveSetting(System::IO::TextWriter^ stream, const MappingStruct* mapping, const parity::utils::LanguageType& val)
{
	stream->Write("{0} = {1}\n", gcnew String(mapping->Name), gcnew String(Context::getContext().printable(val).c_str()));
}

void SaveSetting(System::IO::TextWriter^ stream, const MappingStruct* mapping, const parity::utils::SubsystemType& val)
{
	stream->Write("{0} = {1}\n", gcnew String(mapping->Name), gcnew String(Context::getContext().printable(val).c_str()));
}

void SaveSetting(System::IO::TextWriter^ stream, const MappingStruct* mapping, const parity::utils::ToolchainType& val)
{
	stream->Write("{0} = {1}\n", gcnew String(mapping->Name), gcnew String(Context::getContext().printable(val).c_str()));
}

void SaveSetting(System::IO::TextWriter^ stream, const MappingStruct* mapping, const parity::utils::RuntimeType& val)
{
	stream->Write("{0} = {1}\n", gcnew String(mapping->Name), gcnew String(Context::getContext().printable(val).c_str()));
}

void SaveSetting(System::IO::TextWriter^ stream, const MappingStruct* mapping, const parity::utils::Color::ColorMode& val)
{
	stream->Write("{0} = {1}\n", gcnew String(mapping->Name), gcnew String(Context::getContext().printable(val).c_str()));
}

