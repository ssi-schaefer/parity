#include "DefineMapEditDialog.h"
#include "SimpleStringEditDialog.h"
#include "Configurator.h"

namespace paritygraphicalconfigurator {

	private ref class DefineListItem : System::Object
	{
	private:
		String^ key_;
		String^ value_;
		
	public:
		DefineListItem(String^ key, String^ value)
			: key_(key), value_(value)
		{
		}

		property String^ Key { String^ get() { return key_; } }

		virtual System::String^ ToString() override
		{
			String^ val = gcnew String(key_);

			if(value_ != "")
			{
				val += " => " + value_;
			}

			return val;
		}
	};

	System::Void DefineMapEditDialog::btnDelete_Click(System::Object^  sender, System::EventArgs^  e)
	{
		DefineListItem^ key = (DefineListItem^)lstDefines->SelectedItem;

		if(key == nullptr)
			return;

		defines_.erase(Configurator::MarshalSimpleStringToNative(key->Key));

		CreateDefineList();
	}

	System::Void DefineMapEditDialog::btnNew_Click(System::Object^  sender, System::EventArgs^  e)
	{
		SimpleStringEditDialog^ dlg = gcnew SimpleStringEditDialog();

		if(dlg->ShowDialog() == System::Windows::Forms::DialogResult::OK)
		{
			parity::utils::Context::getContext().convert(defines_, Configurator::MarshalSimpleStringToNative(dlg->Value));
			CreateDefineList();
		}
	}

	System::Void DefineMapEditDialog::CreateDefineList()
	{
		lstDefines->Items->Clear();

		for(parity::utils::DefineMap::iterator it = defines_.begin(); it != defines_.end(); ++it)
		{
			DefineListItem^ item = gcnew DefineListItem(gcnew String(it->first.c_str()), gcnew String(it->second.c_str()));
			lstDefines->Items->Add(item);
		}
	}

}
