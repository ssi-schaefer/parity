#pragma once

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;


namespace paritygraphicalconfigurator {

	/// <summary>
	/// Zusammenfassung für SimpleStringEditDialog
	///
	/// Warnung: Wenn Sie den Namen dieser Klasse ändern, müssen Sie auch
	///          die Ressourcendateiname-Eigenschaft für das Tool zur Kompilierung verwalteter Ressourcen ändern,
	///          das allen RESX-Dateien zugewiesen ist, von denen diese Klasse abhängt.
	///          Anderenfalls können die Designer nicht korrekt mit den lokalisierten Ressourcen
	///          arbeiten, die diesem Formular zugewiesen sind.
	/// </summary>
	public ref class SimpleStringEditDialog : public System::Windows::Forms::Form
	{
	public:
		SimpleStringEditDialog(void)
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
		~SimpleStringEditDialog()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::Label^  label1;
	private: System::Windows::Forms::TextBox^  txtValue;
	private: System::Windows::Forms::Button^  btnCancel;
	private: System::Windows::Forms::Button^  btnOK;
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
			this->label1 = (gcnew System::Windows::Forms::Label());
			this->txtValue = (gcnew System::Windows::Forms::TextBox());
			this->btnCancel = (gcnew System::Windows::Forms::Button());
			this->btnOK = (gcnew System::Windows::Forms::Button());
			this->SuspendLayout();
			// 
			// label1
			// 
			this->label1->AutoSize = true;
			this->label1->Location = System::Drawing::Point(12, 9);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(264, 13);
			this->label1->TabIndex = 0;
			this->label1->Text = L"Change the Value below to reflect the desired change:";
			// 
			// txtValue
			// 
			this->txtValue->Location = System::Drawing::Point(15, 25);
			this->txtValue->Name = L"txtValue";
			this->txtValue->Size = System::Drawing::Size(306, 20);
			this->txtValue->TabIndex = 1;
			// 
			// btnCancel
			// 
			this->btnCancel->DialogResult = System::Windows::Forms::DialogResult::Cancel;
			this->btnCancel->Location = System::Drawing::Point(165, 52);
			this->btnCancel->Name = L"btnCancel";
			this->btnCancel->Size = System::Drawing::Size(75, 23);
			this->btnCancel->TabIndex = 2;
			this->btnCancel->Text = L"Cancel";
			this->btnCancel->UseVisualStyleBackColor = true;
			// 
			// btnOK
			// 
			this->btnOK->DialogResult = System::Windows::Forms::DialogResult::OK;
			this->btnOK->Location = System::Drawing::Point(246, 52);
			this->btnOK->Name = L"btnOK";
			this->btnOK->Size = System::Drawing::Size(75, 23);
			this->btnOK->TabIndex = 3;
			this->btnOK->Text = L"OK";
			this->btnOK->UseVisualStyleBackColor = true;
			// 
			// SimpleStringEditDialog
			// 
			this->AcceptButton = this->btnOK;
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->CancelButton = this->btnCancel;
			this->ClientSize = System::Drawing::Size(333, 87);
			this->ControlBox = false;
			this->Controls->Add(this->btnOK);
			this->Controls->Add(this->btnCancel);
			this->Controls->Add(this->txtValue);
			this->Controls->Add(this->label1);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
			this->Name = L"SimpleStringEditDialog";
			this->Text = L"Edit Setting";
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion

public:
	property String^ Value {
		String^ get() { return txtValue->Text; }
		void set(String^ value) { txtValue->Text = value; }
	}

	};
}
