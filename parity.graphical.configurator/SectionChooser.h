#pragma once

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;


namespace paritygraphicalconfigurator {

	/// <summary>
	/// Summary for SectionChooser
	///
	/// WARNING: If you change the name of this class, you will need to change the
	///          'Resource File Name' property for the managed resource compiler tool
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public ref class SectionChooser : public System::Windows::Forms::Form
	{
	public:
		SectionChooser(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~SectionChooser()
		{
			if (components)
			{
				delete components;
			}
		}
	public: System::Windows::Forms::ListBox^  lstSections;
	protected: 

	protected: 
	private: System::Windows::Forms::Label^  lblChoose;
	private: System::Windows::Forms::Button^  btnOk;

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->lstSections = (gcnew System::Windows::Forms::ListBox());
			this->lblChoose = (gcnew System::Windows::Forms::Label());
			this->btnOk = (gcnew System::Windows::Forms::Button());
			this->SuspendLayout();
			// 
			// lstSections
			// 
			this->lstSections->FormattingEnabled = true;
			this->lstSections->Location = System::Drawing::Point(12, 28);
			this->lstSections->Name = L"lstSections";
			this->lstSections->SelectionMode = System::Windows::Forms::SelectionMode::MultiExtended;
			this->lstSections->Size = System::Drawing::Size(379, 95);
			this->lstSections->TabIndex = 0;
			// 
			// lblChoose
			// 
			this->lblChoose->AutoSize = true;
			this->lblChoose->Location = System::Drawing::Point(12, 9);
			this->lblChoose->Name = L"lblChoose";
			this->lblChoose->Size = System::Drawing::Size(177, 13);
			this->lblChoose->TabIndex = 1;
			this->lblChoose->Text = L"Please choose the Sections to load:";
			// 
			// btnOk
			// 
			this->btnOk->DialogResult = System::Windows::Forms::DialogResult::OK;
			this->btnOk->Location = System::Drawing::Point(316, 132);
			this->btnOk->Name = L"btnOk";
			this->btnOk->Size = System::Drawing::Size(75, 23);
			this->btnOk->TabIndex = 2;
			this->btnOk->Text = L"OK";
			this->btnOk->UseVisualStyleBackColor = true;
			// 
			// SectionChooser
			// 
			this->AcceptButton = this->btnOk;
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(403, 167);
			this->ControlBox = false;
			this->Controls->Add(this->btnOk);
			this->Controls->Add(this->lblChoose);
			this->Controls->Add(this->lstSections);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
			this->Name = L"SectionChooser";
			this->Text = L"SectionChooser";
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
	};
}
