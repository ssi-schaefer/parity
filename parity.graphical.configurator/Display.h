#pragma once

#include <Context.h>

System::String^ FormatForDisplay(const bool& val);
System::String^ FormatForDisplay(const long& val);
System::String^ FormatForDisplay(const std::string& val);
System::String^ FormatForDisplay(const parity::utils::Path& val);
System::String^ FormatForDisplay(const parity::utils::DefineMap& val);
//System::String^ FormatForDisplay(const parity::utils::SourceMap& val);
System::String^ FormatForDisplay(const parity::utils::PathVector& val);
System::String^ FormatForDisplay(const parity::utils::LanguageType& val);
System::String^ FormatForDisplay(const parity::utils::SubsystemType& val);
System::String^ FormatForDisplay(const parity::utils::ToolchainType& val);
System::String^ FormatForDisplay(const parity::utils::RuntimeType& val);
System::String^ FormatForDisplay(const parity::utils::Color::ColorMode& val);

typedef System::String^ (*DisplayFormatFunction)(parity::utils::Context& ctx);
typedef bool (*IsDefaultFunction)(parity::utils::Context& ctx);

struct MappingStruct
{
	char* Name;
	char* Type;
	char* Default;
	DisplayFormatFunction Formatter;
	IsDefaultFunction IsDefault;
};

extern DisplayMappingStruct* DisplayMapping;
