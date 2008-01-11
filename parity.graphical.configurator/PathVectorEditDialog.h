#pragma once

#include <Context.h>

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;


namespace paritygraphicalconfigurator {

	/// <summary>
	/// Zusammenfassung für PathVectorEditDialog
	///
	/// Warnung: Wenn Sie den Namen dieser Klasse ändern, müssen Sie auch
	///          die Ressourcendateiname-Eigenschaft für das Tool zur Kompilierung verwalteter Ressourcen ändern,
	///          das allen RESX-Dateien zugewiesen ist, von denen diese Klasse abhängt.
	///          Anderenfalls können die Designer nicht korrekt mit den lokalisierten Ressourcen
	///          arbeiten, die diesem Formular zugewiesen sind.
	/// </summary>
	public ref class PathVectorEditDialog : public System::Windows::Forms::Form
	{
	public:
		PathVectorEditDialog(parity::utils::PathVector& toEdit)
			: paths_(toEdit)
		{
			InitializeComponent();
			//
			//TODO: Konstruktorcode hier hinzufügen.
			//
			CreatePathList();
		}

	protected:
		/// <summary>
		/// Verwendete Ressourcen bereinigen.
		/// </summary>
		~PathVectorEditDialog()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::Button^  btnOK;
	protected: 
	private: System::Windows::Forms::Button^  btnCancel;
	private: System::Windows::Forms::ListBox^  lstPaths;
	private: System::Windows::Forms::Button^  btnDelete;
	private: System::Windows::Forms::Button^  btnNew;


	private:
		/// <summary>
		/// Erforderliche Designervariable.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Erforderliche Methode für die Designerunterstützung.
		/// Der Inhalt der Methode darf nicht mit dem Code-Editor geändert werden.
		/// </summary>
		void InitializeComponent(void)
		{
			this->btnOK = (gcnew System::Windows::Forms::Button());
			this->btnCancel = (gcnew System::Windows::Forms::Button());
			this->lstPaths = (gcnew System::Windows::Forms::ListBox());
			this->btnDelete = (gcnew System::Windows::Forms::Button());
			this->btnNew = (gcnew System::Windows::Forms::Button());
			this->SuspendLayout();
			// 
			// btnOK
			// 
			this->btnOK->DialogResult = System::Windows::Forms::DialogResult::OK;
			this->btnOK->Location = System::Drawing::Point(408, 321);
			this->btnOK->Name = L"btnOK";
			this->btnOK->Size = System::Drawing::Size(75, 23);
			this->btnOK->TabIndex = 0;
			this->btnOK->Text = L"OK";
			this->btnOK->UseVisualStyleBackColor = true;
			// 
			// btnCancel
			// 
			this->btnCancel->DialogResult = System::Windows::Forms::DialogResult::Cancel;
			this->btnCancel->Location = System::Drawing::Point(327, 321);
			this->btnCancel->Name = L"btnCancel";
			this->btnCancel->Size = System::Drawing::Size(75, 23);
			this->btnCancel->TabIndex = 1;
			this->btnCancel->Text = L"Cancel";
			this->btnCancel->UseVisualStyleBackColor = true;
			// 
			// lstPaths
			// 
			this->lstPaths->FormattingEnabled = true;
			this->lstPaths->Location = System::Drawing::Point(12, 12);
			this->lstPaths->Name = L"lstPaths";
			this->lstPaths->Size = System::Drawing::Size(471, 303);
			this->lstPaths->TabIndex = 2;
			// 
			// btnDelete
			// 
			this->btnDelete->Location = System::Drawing::Point(12, 321);
			this->btnDelete->Name = L"btnDelete";
			this->btnDelete->Size = System::Drawing::Size(75, 23);
			this->btnDelete->TabIndex = 3;
			this->btnDelete->Text = L"Delete";
			this->btnDelete->UseVisualStyleBackColor = true;
			this->btnDelete->Click += gcnew System::EventHandler(this, &PathVectorEditDialog::btnDelete_Click);
			// 
			// btnNew
			// 
			this->btnNew->Location = System::Drawing::Point(93, 321);
			this->btnNew->Name = L"btnNew";
			this->btnNew->Size = System::Drawing::Size(75, 23);
			this->btnNew->TabIndex = 4;
			this->btnNew->Text = L"New";
			this->btnNew->UseVisualStyleBackColor = true;
			this->btnNew->Click += gcnew System::EventHandler(this, &PathVectorEditDialog::btnNew_Click);
			// 
			// PathVectorEditDialog
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(495, 354);
			this->ControlBox = false;
			this->Controls->Add(this->btnNew);
			this->Controls->Add(this->btnDelete);
			this->Controls->Add(this->lstPaths);
			this->Controls->Add(this->btnCancel);
			this->Controls->Add(this->btnOK);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
			this->Name = L"PathVectorEditDialog";
			this->Text = L"Edit Path List";
			this->ResumeLayout(false);

		}
#pragma endregion
private:
	System::Void btnDelete_Click(System::Object^  sender, System::EventArgs^  e);
	System::Void btnNew_Click(System::Object^  sender, System::EventArgs^  e);

	System::Void CreatePathList();

	parity::utils::PathVector& paths_;
};
}
