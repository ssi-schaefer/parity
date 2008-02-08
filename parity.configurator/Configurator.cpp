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

#include <iostream>
#include <sstream>
#include <Context.h>
#include <Configuration.h>
#include <MappedFile.h>
#include <MemoryFile.h>
#include <Color.h>
#include <Exception.h>
#include <Log.h>

using namespace parity::utils;

typedef struct {
	const char* name;
	const char* type;
	const char* init;
	void (Context::*setter)(const std::string&);
	std::string (*getter)();
} ComplexMapping;

#undef CTX_GETSET
#undef CTX_GETSET_C
#undef CTX_GETSET_I
#undef CTX_GETSET_CI
#define CTX_GETSET_I(type, name, init)
#define CTX_GETSET_CI(type, name, init)

#define CTX_GETSET(type, name, init) \
	std::string get##name##AsString() { return Context::getContext().printable(Context::getContext().get##name()); }

#define CTX_GETSET_C(type, name, init) \
	CTX_GETSET(type, name, init)

CTX_MEMBERS

#undef CTX_GETSET
#undef CTX_GETSET_C

#define CTX_GETSET(type, name, init) { #name, #type, #init, &Context::set##name##String, get##name##AsString },
#define CTX_GETSET_C(type, name, init) { #name, #type, "", &Context::set##name##String, get##name##AsString },

ComplexMapping gMapping[] = {
	CTX_MEMBERS
	// must be last
	{0, 0, 0, 0, 0}
};

void mainMenu();
void displayContext();
void changeSetting();
void loadConfiguration();
void saveConfiguration();

using namespace parity;

//
// CONFIX:EXENAME('parity.configurator')
//

int main(void)
{

	std::cout << "Setting DebugLevel to verbose..." << std::endl;
	Log::setLevel(Log::Verbose);

	mainMenu();

	return 0;
}

void mainMenu()
{
	bool bRunning = true;
	Color col(Context::getContext().getColorMode());

	while(bRunning)
	{
		std::cout << std::endl;
		std::cout << col.green("Main Menu:") << std::endl;
		std::cout << "----------" << std::endl;
		std::cout << std::endl;
		std::cout << " 1) Display current Settings" << std::endl;
		std::cout << " 2) Change a Setting" << std::endl;
		std::cout << " 3) Load a Configuration File" << std::endl;
		std::cout << " 4) Save current Settings to a File" << std::endl;
		std::cout << " 5) Quit Configurator" << std::endl;
		std::cout << std::endl;

		std::cout << "Your Choice: ";
		long choice = 0;
		std::cin >> choice;

		std::cout << std::endl;

		switch(choice)
		{
		case 1:
			displayContext();
			break;
		case 2:
			changeSetting();
			break;
		case 3:
			loadConfiguration();
			break;
		case 4:
			saveConfiguration();
			break;
		case 5:
			return;
		}
	}
}

void displayContext()
{
	bool bState = Context::getContext().getCtxDump();
	Context::getContext().setCtxDump(true);
	Context::getContext().dump(false);
	Context::getContext().setCtxDump(bState);
}

ComplexMapping* getComplexMapping(const char* name)
{
	ComplexMapping* ptr = gMapping;

	while(ptr && ptr->name)
	{
		if(strcmp(name, ptr->name) == 0)
			return ptr;

		++ptr;
	}

	return 0;
}

void changeSetting()
{
	std::string setting;
	std::string value;
	Context& ctx = Context::getContext();
	ComplexMapping* ptr = 0;

	while(!ptr)
	{
		std::cout << "Setting Name: ";
		std::cin >> setting;

		if(setting.empty())
			return;

		ptr = getComplexMapping(setting.c_str());
	}

	if(!ptr)
		return;

	std::cout << "Found Setting " << ptr->name << ", Type: " << ptr->type << ", Initial Value: " << ptr->init << ", Current: " << ptr->getter() << std::endl;
	std::cout << std::endl;
	std::cout << "New Value: ";
	std::cin >> value;

	try {
		(ctx.*(ptr->setter))(value);
	} catch(const Exception& e)
	{
		std::cout << "error while setting" << setting << ": " << e.what() << std::endl;
	}

	std::cout << "New Value set to: " << ptr->getter() << std::endl;
}

void loadConfiguration()
{
	std::string filename;
	Context& ctx = Context::getContext();
	
	std::cout << "Filename: ";
	std::cin >> filename;
	
	if(filename.empty())
		return;

	std::cout << "loading " << filename << "..." << std::endl;

	try {
		Path path(filename);
		path.toNative();

		if(!path.exists())
			throw Exception("%s does not exist!", filename.c_str());

		MappedFile map(path, ModeRead);
		Config::parseFile(ctx, map);
	} catch(const Exception& e) {
		std::cout << "error while loading " << filename << ": " << e.what() << std::endl;
	}
}

void saveSetting(std::ostringstream& target, const std::string& name, const std::string& value)
{
	target << name << " = " << value << std::endl;
}

void saveSetting(std::ostringstream& target, const std::string& name, const bool& value)
{
	target << name << " = " << (value?"on":"off") << std::endl;
}

void saveSetting(std::ostringstream& target, const std::string& name, const Path& value)
{
	if(name == "DefaultOutput")
		target << name << " = " << value.file() << std::endl;
	else
		target << name << " = " << value.get() << std::endl;
}

void saveSetting(std::ostringstream& target, const std::string& name, const long& value)
{
	target << name << " = " << value << std::endl;
}

void saveSetting(std::ostringstream& target, const std::string& name, const PathVector& value)
{
	bool bFileOnly = false;
	if(name == "ObjectsLibraries")
		bFileOnly = true;
	for(PathVector::const_iterator it = value.begin(); it != value.end(); ++it)
	{
		target << name << " = " << (bFileOnly? it->file() : it->get()) << std::endl;
	}
}

void saveSetting(std::ostringstream& target, const std::string& name, const DefineMap& value)
{
	for(DefineMap::const_iterator it = value.begin(); it != value.end(); ++it)
	{
		target << name << " = " << it->first;
		if(!it->second.empty())
		{
			target << "=" << it->second;
		}
		target << std::endl;
	}
}

void saveSetting(std::ostringstream& target, const std::string& name, const ToolchainType& value)
{
	target << name << " = ";

	switch(value)
	{
	case ToolchainInterixGNU:
		target << "GCC";
		break;
	case ToolchainMicrosoft:
		target << "Microsoft";
		break;
	case ToolchainInvalid:
		target << "<INVALID>";
		break;
	}
	target << std::endl;
}

void saveSetting(std::ostringstream& target, const std::string& name, const Color::ColorMode& value)
{
	target << name << " = ";

	switch(value)
	{
	case Color::Bright:
		target << "bright";
		break;
	case Color::Dark:
		target << "dark";
		break;
	}
	target << std::endl;
}

void saveSetting(std::ostringstream& target, const std::string& name, const LanguageType& value)
{
	target << name << " = ";

	switch(value)
	{
	case LanguageC:
		target << "C";
		break;
	case LanguageCpp:
		target << "C++";
		break;
	case LanguageInvalid:
		target << "none";
		break;
	default:
		target << "INVALID" << std::endl;
		break;
	}
	target << std::endl;
}

void saveSetting(std::ostringstream& target, const std::string& name, const RuntimeType& value)
{
	target << name << " = " << Context::getContext().printable(value);
}

void saveSetting(std::ostringstream& target, const std::string& name, const SubsystemType& value)
{
	target << name << " = ";

	switch(value)
	{
	case SubsystemWindowsCui:
		target << "Console";
		break;
	case SubsystemUnknown:
		target << "Invalid";
		break;
	case SubsystemPosixCui:
		target << "POSIX";
		break;
	case SubsystemWindowsGui:
		target << "Windows";
		break;
	case SubsystemWindowsCeGui:
		target << "Windows CE";
		break;
	default:
		target << "INVALID";
		break;
	}
	target << std::endl;
}

void saveSetting(std::ostringstream& target, const std::string& name, const SourceMap& value)
{
	target << "# WARNING: this is a runtime only setting, and may not be set by configuration!" << std::endl;
}

void saveConfiguration()
{
	std::ostringstream target;
	Context& ctx = Context::getContext();

	target << "#\n"
		"# PARITY CONFIGURATION\n"
		"#\n"
		"#  Automatically generated by the parity configuration utility.\n"
		"#\n";

#undef CTX_GETSET
#undef CTX_GETSET_C

#define CTX_GETSET(type, name, init) \
	if(ctx.get##name() != init) { \
		target << "\n# " << #name << " (" << #type << "): Initial Value: " << #init << "\n"; \
		saveSetting(target, #name, ctx.get##name()); \
	}

#define CTX_GETSET_C(type, name, init) CTX_GETSET(type, name, init)

	CTX_MEMBERS

	std::string filename;
	std::cout << "Filename: ";
	std::cin >> filename;

	Path file(filename);
	file.toNative();

	if(file.exists())
	{
		std::cout << "Overwrite existing " << filename << "? (y/n) ";
		std::string answer;
		std::cin >> answer;

		if(answer[0] != 'y' && answer[0] != 'Y')
			return;
	}

	//
	// This memory file does not live long enough to allow any
	// modification of the buffer, so we can savely const_cast it.
	//
	try {
		std::string str = target.str();
		MemoryFile mem(const_cast<char*>(str.data()), str.size());
		mem.save(file);

		file.mode(0644);
	} catch(const Exception& e) {
		Log::warning("While writing %s: %s\n", filename.c_str(), e.what());
	}
}

