#include "Configurator.h"
#include "Mapping.h"

#include <Context.h>
#include <Configuration.h>
#include <MappedFile.h>

#include <string>

using namespace parity::utils;

namespace paritygraphicalconfigurator {
	System::Void Configurator::EditItemDetails(System::Object^  sender, System::EventArgs^  e)
	{
		MappingStruct* ptr = SettingMapping;
		ListViewItem^ lvi = lvSettings->SelectedItems[0];

		if(lvi)
		{
			while(ptr->Name)
			{
				if(lvi->SubItems[0]->Text == gcnew String(ptr->Name))
				{
					ptr->Edit(ptr, *context_);
				}

				++ptr;
			}
		}
	}

	System::Void Configurator::NewConfiguration(System::Object^  sender, System::EventArgs^  e)
	{
		InitNewContext();
		UpdateConfigurationView(*context_);
	}

	bool Configurator::InitNewContext()
	{
		if(context_ && initial_ && *context_ != *initial_)
		{
			if(MessageBox::Show("Discard changes to current configuration?", "Discard changes?", MessageBoxButtons::OKCancel, MessageBoxIcon::Question) == System::Windows::Forms::DialogResult::Cancel)
				return false;
		}

		context_ = new Context();
		initial_ = new Context();

		return true;
	}

	System::Void Configurator::OpenConfiguration(System::Object^  sender, System::EventArgs^  e)
	{
		if(dlgOpenFile->ShowDialog() == System::Windows::Forms::DialogResult::OK)
		{
			if(!InitNewContext())
				return;
			
			std::string native = MarshalSimpleStringToNative(dlgOpenFile->FileName);
			MappedFile file(Path(native), ModeRead);

			Config::parseFile(*context_, file);
			*initial_ = *context_;

			UpdateConfigurationView(*context_);
		}
	}

	System::Void Configurator::SaveConfiguration(System::Object^  sender, System::EventArgs^  e)
	{
	}

	System::Void Configurator::UpdateConfigurationView(parity::utils::Context& ctx)
	{
		MappingStruct * ptr = SettingMapping;

		lvSettings->Items->Clear();

		while(ptr->Name && ptr->Formatter)
		{
			ListViewItem^ lvi = 
				gcnew ListViewItem(
					gcnew array<String^>
						{
							gcnew String(ptr->Name), 
							gcnew String(ptr->Type), 
							ptr->Formatter(ctx), 
							gcnew String(ptr->Default)
						}
					, 1
				);

			if(ptr->IsDefault(ctx))
			{
				lvi->Group = lvSettings->Groups[1];
			} else {
				lvi->Group = lvSettings->Groups[0];
			}

			lvSettings->Items->Add(lvi);

			++ptr;
		}
	}

	//
	// Helper functions
	//

	std::string Configurator::MarshalSimpleStringToNative(String^ str)
	{
		IntPtr native = System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(str);
		std::string result((char*)native.ToPointer());
		System::Runtime::InteropServices::Marshal::FreeHGlobal(native);

		return result;
	}

	String^ Configurator::MarshalSimpleNativeToString(std::string& str)
	{
		return gcnew String(str.c_str());
	}
}
