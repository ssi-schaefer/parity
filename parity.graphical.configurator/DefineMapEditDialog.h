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
	/// Zusammenfassung für DefineMapEditDialog
	///
	/// Warnung: Wenn Sie den Namen dieser Klasse ändern, müssen Sie auch
	///          die Ressourcendateiname-Eigenschaft für das Tool zur Kompilierung verwalteter Ressourcen ändern,
	///          das allen RESX-Dateien zugewiesen ist, von denen diese Klasse abhängt.
	///          Anderenfalls können die Designer nicht korrekt mit den lokalisierten Ressourcen
	///          arbeiten, die diesem Formular zugewiesen sind.
	/// </summary>
	public ref class DefineMapEditDialog : public System::Windows::Forms::Form
	{
	public:
		DefineMapEditDialog(parity::utils::DefineMap& toEdit)
			: defines_(toEdit), original_(toEdit)
		{
			InitializeComponent();
			//
			//TODO: Konstruktorcode hier hinzufügen.
			//
		}

	protected:
		/// <summary>
		/// Verwendete Ressourcen bereinigen.
		/// </summary>
		~DefineMapEditDialog()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::Button^  btnCancel;
	private: System::Windows::Forms::Button^  btnOk;
	private: System::Windows::Forms::ListBox^  lstDefines;
	private: System::Windows::Forms::Button^  btnDelete;
	private: System::Windows::Forms::Button^  btnNew;
	protected: 


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
			this->btnCancel = (gcnew System::Windows::Forms::Button());
			this->btnOk = (gcnew System::Windows::Forms::Button());
			this->lstDefines = (gcnew System::Windows::Forms::ListBox());
			this->btnDelete = (gcnew System::Windows::Forms::Button());
			this->btnNew = (gcnew System::Windows::Forms::Button());
			this->SuspendLayout();
			// 
			// btnCancel
			// 
			this->btnCancel->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
			this->btnCancel->DialogResult = System::Windows::Forms::DialogResult::Cancel;
			this->btnCancel->Location = System::Drawing::Point(341, 272);
			this->btnCancel->Name = L"btnCancel";
			this->btnCancel->Size = System::Drawing::Size(75, 23);
			this->btnCancel->TabIndex = 0;
			this->btnCancel->Text = L"Cancel";
			this->btnCancel->UseVisualStyleBackColor = true;
			// 
			// btnOk
			// 
			this->btnOk->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
			this->btnOk->DialogResult = System::Windows::Forms::DialogResult::OK;
			this->btnOk->Location = System::Drawing::Point(422, 272);
			this->btnOk->Name = L"btnOk";
			this->btnOk->Size = System::Drawing::Size(75, 23);
			this->btnOk->TabIndex = 1;
			this->btnOk->Text = L"OK";
			this->btnOk->UseVisualStyleBackColor = true;
			// 
			// lstDefines
			// 
			this->lstDefines->FormattingEnabled = true;
			this->lstDefines->Location = System::Drawing::Point(12, 12);
			this->lstDefines->Name = L"lstDefines";
			this->lstDefines->Size = System::Drawing::Size(485, 251);
			this->lstDefines->TabIndex = 2;
			// 
			// btnDelete
			// 
			this->btnDelete->Location = System::Drawing::Point(12, 272);
			this->btnDelete->Name = L"btnDelete";
			this->btnDelete->Size = System::Drawing::Size(75, 23);
			this->btnDelete->TabIndex = 3;
			this->btnDelete->Text = L"Delete";
			this->btnDelete->UseVisualStyleBackColor = true;
			// 
			// btnNew
			// 
			this->btnNew->Location = System::Drawing::Point(93, 272);
			this->btnNew->Name = L"btnNew";
			this->btnNew->Size = System::Drawing::Size(75, 23);
			this->btnNew->TabIndex = 4;
			this->btnNew->Text = L"New";
			this->btnNew->UseVisualStyleBackColor = true;
			// 
			// DefineMapEditDialog
			// 
			this->AcceptButton = this->btnOk;
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->CancelButton = this->btnCancel;
			this->ClientSize = System::Drawing::Size(509, 307);
			this->ControlBox = false;
			this->Controls->Add(this->btnNew);
			this->Controls->Add(this->btnDelete);
			this->Controls->Add(this->lstDefines);
			this->Controls->Add(this->btnOk);
			this->Controls->Add(this->btnCancel);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
			this->Name = L"DefineMapEditDialog";
			this->Text = L"Edit Define Collection";
			this->ResumeLayout(false);

		}
#pragma endregion

private:
	parity::utils::DefineMap& defines_;
	};
}
