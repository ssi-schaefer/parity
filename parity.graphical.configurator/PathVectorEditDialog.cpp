#include "PathVectorEditDialog.h"
#include "SimpleStringEditDialog.h"
#include "Configurator.h"

namespace paritygraphicalconfigurator {

	System::Void PathVectorEditDialog::btnDelete_Click(System::Object^  sender, System::EventArgs^  e)
	{
		String^ str = (String^)lstPaths->SelectedItem;

		if(str == nullptr)
			return;

		parity::utils::Path cmp(Configurator::MarshalSimpleStringToNative(str));

		for(parity::utils::PathVector::iterator it = paths_.begin(); it != paths_.end(); ++it)
		{
			if(cmp == *it)
			{
				paths_.erase(it);
				break;
			}
		}

		CreatePathList();
	}

	System::Void PathVectorEditDialog::btnNew_Click(System::Object^  sender, System::EventArgs^  e)
	{
		SimpleStringEditDialog^ dlg = gcnew SimpleStringEditDialog();

		if(dlg->ShowDialog() == System::Windows::Forms::DialogResult::OK)
		{
			parity::utils::Context::getContext().convert(paths_, Configurator::MarshalSimpleStringToNative(dlg->Value));
			CreatePathList();
		}
	}

	System::Void PathVectorEditDialog::CreatePathList()
	{
		lstPaths->Items->Clear();

		for(parity::utils::PathVector::iterator it = paths_.begin(); it != paths_.end(); ++it)
		{
			String^ str = gcnew String(it->get().c_str());
			lstPaths->Items->Add(str);
		}
	}

}