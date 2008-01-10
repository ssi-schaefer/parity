#pragma once

#include <Context.h>

struct MappingStruct;

typedef System::String^ (*DisplayFormatFunction)(parity::utils::Context& ctx);
typedef void (*EditFunction)(const MappingStruct* mapping, parity::utils::Context& ctx);
typedef void (*SaveFunction)(System::IO::TextWriter^ stream, const MappingStruct* mapping, parity::utils::Context& ctx);
typedef bool (*IsDefaultFunction)(parity::utils::Context& ctx);

struct MappingStruct
{
	char* Name;
	char* Type;
	char* Default;
	DisplayFormatFunction Formatter;
	IsDefaultFunction IsDefault;
	EditFunction Edit;
	SaveFunction Save;
};

extern MappingStruct* SettingMapping;

System::String^ FormatForDisplay(const bool& val);
System::String^ FormatForDisplay(const long& val);
System::String^ FormatForDisplay(const std::string& val);
System::String^ FormatForDisplay(const parity::utils::Path& val);
System::String^ FormatForDisplay(const parity::utils::DefineMap& val);
System::String^ FormatForDisplay(const parity::utils::PathVector& val);
System::String^ FormatForDisplay(const parity::utils::LanguageType& val);
System::String^ FormatForDisplay(const parity::utils::SubsystemType& val);
System::String^ FormatForDisplay(const parity::utils::ToolchainType& val);
System::String^ FormatForDisplay(const parity::utils::RuntimeType& val);
System::String^ FormatForDisplay(const parity::utils::Color::ColorMode& val);

void DisplayEditDialog(const MappingStruct* mapping, const bool& val);
void DisplayEditDialog(const MappingStruct* mapping, const long& val);
void DisplayEditDialog(const MappingStruct* mapping, const std::string& val);
void DisplayEditDialog(const MappingStruct* mapping, const parity::utils::Path& val);
void DisplayEditDialog(const MappingStruct* mapping, const parity::utils::DefineMap& val);
void DisplayEditDialog(const MappingStruct* mapping, const parity::utils::PathVector& val);
void DisplayEditDialog(const MappingStruct* mapping, const parity::utils::LanguageType& val);
void DisplayEditDialog(const MappingStruct* mapping, const parity::utils::SubsystemType& val);
void DisplayEditDialog(const MappingStruct* mapping, const parity::utils::ToolchainType& val);
void DisplayEditDialog(const MappingStruct* mapping, const parity::utils::RuntimeType& val);
void DisplayEditDialog(const MappingStruct* mapping, const parity::utils::Color::ColorMode& val);

void SaveSetting(System::IO::TextWriter^ stream, const MappingStruct* mapping, const bool& val);
void SaveSetting(System::IO::TextWriter^ stream, const MappingStruct* mapping, const long& val);
void SaveSetting(System::IO::TextWriter^ stream, const MappingStruct* mapping, const std::string& val);
void SaveSetting(System::IO::TextWriter^ stream, const MappingStruct* mapping, const parity::utils::Path& val);
void SaveSetting(System::IO::TextWriter^ stream, const MappingStruct* mapping, const parity::utils::DefineMap& val);
void SaveSetting(System::IO::TextWriter^ stream, const MappingStruct* mapping, const parity::utils::PathVector& val);
void SaveSetting(System::IO::TextWriter^ stream, const MappingStruct* mapping, const parity::utils::LanguageType& val);
void SaveSetting(System::IO::TextWriter^ stream, const MappingStruct* mapping, const parity::utils::SubsystemType& val);
void SaveSetting(System::IO::TextWriter^ stream, const MappingStruct* mapping, const parity::utils::ToolchainType& val);
void SaveSetting(System::IO::TextWriter^ stream, const MappingStruct* mapping, const parity::utils::RuntimeType& val);
void SaveSetting(System::IO::TextWriter^ stream, const MappingStruct* mapping, const parity::utils::Color::ColorMode& val);
