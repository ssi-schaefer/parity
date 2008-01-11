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
	/// Zusammenfassung für Configurator
	///
	/// Warnung: Wenn Sie den Namen dieser Klasse ändern, müssen Sie auch
	///          die Ressourcendateiname-Eigenschaft für das Tool zur Kompilierung verwalteter Ressourcen ändern,
	///          das allen RESX-Dateien zugewiesen ist, von denen diese Klasse abhängt.
	///          Anderenfalls können die Designer nicht korrekt mit den lokalisierten Ressourcen
	///          arbeiten, die diesem Formular zugewiesen sind.
	/// </summary>
	public ref class Configurator : public System::Windows::Forms::Form
	{
	public:
		Configurator(void)
		{
			InitializeComponent();
			context_ = NULL;
			initial_ = NULL;
		}

	protected:
		/// <summary>
		/// Verwendete Ressourcen bereinigen.
		/// </summary>
		~Configurator()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::ToolStrip^  tsToolbar;
	private: System::Windows::Forms::ToolStripButton^  tbtnNew;
	private: System::Windows::Forms::ToolStripButton^  tbtnOpen;
	private: System::Windows::Forms::ToolStripButton^  tbtnSave;
	private: System::Windows::Forms::ListView^  lvSettings;
	private: System::Windows::Forms::ImageList^  ilImages;
	private: System::Windows::Forms::ColumnHeader^  colName;
	private: System::Windows::Forms::ColumnHeader^  colType;
	private: System::Windows::Forms::ColumnHeader^  colValue;
	private: System::Windows::Forms::ColumnHeader^  colDefaultValue;
	private: System::Windows::Forms::OpenFileDialog^  dlgOpenFile;
	private: System::Windows::Forms::SaveFileDialog^  dlgSaveFile;
	private: System::ComponentModel::IContainer^  components;
	protected: 

	protected: 




	private:
		/// <summary>
		/// Erforderliche Designervariable.
		/// </summary>


#pragma region Windows Form Designer generated code
		/// <summary>
		/// Erforderliche Methode für die Designerunterstützung.
		/// Der Inhalt der Methode darf nicht mit dem Code-Editor geändert werden.
		/// </summary>
		void InitializeComponent(void)
		{
			this->components = (gcnew System::ComponentModel::Container());
			System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(Configurator::typeid));
			System::Windows::Forms::ListViewGroup^  listViewGroup1 = (gcnew System::Windows::Forms::ListViewGroup(L"Changed Settings", System::Windows::Forms::HorizontalAlignment::Left));
			System::Windows::Forms::ListViewGroup^  listViewGroup2 = (gcnew System::Windows::Forms::ListViewGroup(L"Unchanged Settings", System::Windows::Forms::HorizontalAlignment::Left));
			this->tsToolbar = (gcnew System::Windows::Forms::ToolStrip());
			this->tbtnNew = (gcnew System::Windows::Forms::ToolStripButton());
			this->tbtnOpen = (gcnew System::Windows::Forms::ToolStripButton());
			this->tbtnSave = (gcnew System::Windows::Forms::ToolStripButton());
			this->lvSettings = (gcnew System::Windows::Forms::ListView());
			this->colName = (gcnew System::Windows::Forms::ColumnHeader(0));
			this->colType = (gcnew System::Windows::Forms::ColumnHeader());
			this->colValue = (gcnew System::Windows::Forms::ColumnHeader());
			this->colDefaultValue = (gcnew System::Windows::Forms::ColumnHeader());
			this->ilImages = (gcnew System::Windows::Forms::ImageList(this->components));
			this->dlgOpenFile = (gcnew System::Windows::Forms::OpenFileDialog());
			this->dlgSaveFile = (gcnew System::Windows::Forms::SaveFileDialog());
			this->tsToolbar->SuspendLayout();
			this->SuspendLayout();
			// 
			// tsToolbar
			// 
			this->tsToolbar->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(3) {this->tbtnNew, this->tbtnOpen, 
				this->tbtnSave});
			this->tsToolbar->Location = System::Drawing::Point(0, 0);
			this->tsToolbar->Name = L"tsToolbar";
			this->tsToolbar->Size = System::Drawing::Size(815, 25);
			this->tsToolbar->TabIndex = 0;
			this->tsToolbar->Text = L"Toolbar";
			// 
			// tbtnNew
			// 
			this->tbtnNew->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Image;
			this->tbtnNew->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"tbtnNew.Image")));
			this->tbtnNew->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->tbtnNew->Name = L"tbtnNew";
			this->tbtnNew->Size = System::Drawing::Size(23, 22);
			this->tbtnNew->Text = L"Create a new Configuration";
			this->tbtnNew->Click += gcnew System::EventHandler(this, &Configurator::NewConfiguration);
			// 
			// tbtnOpen
			// 
			this->tbtnOpen->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Image;
			this->tbtnOpen->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"tbtnOpen.Image")));
			this->tbtnOpen->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->tbtnOpen->Name = L"tbtnOpen";
			this->tbtnOpen->Size = System::Drawing::Size(23, 22);
			this->tbtnOpen->Text = L"Open an existing Configuration";
			this->tbtnOpen->Click += gcnew System::EventHandler(this, &Configurator::OpenConfiguration);
			// 
			// tbtnSave
			// 
			this->tbtnSave->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Image;
			this->tbtnSave->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"tbtnSave.Image")));
			this->tbtnSave->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->tbtnSave->Name = L"tbtnSave";
			this->tbtnSave->Size = System::Drawing::Size(23, 22);
			this->tbtnSave->Text = L"Save current configuration";
			this->tbtnSave->Click += gcnew System::EventHandler(this, &Configurator::SaveConfiguration);
			// 
			// lvSettings
			// 
			this->lvSettings->AllowColumnReorder = true;
			this->lvSettings->Columns->AddRange(gcnew cli::array< System::Windows::Forms::ColumnHeader^  >(4) {this->colName, this->colValue, 
				this->colType, this->colDefaultValue});
			this->lvSettings->Dock = System::Windows::Forms::DockStyle::Fill;
			this->lvSettings->FullRowSelect = true;
			this->lvSettings->GridLines = true;
			listViewGroup1->Header = L"Changed Settings";
			listViewGroup1->Name = L"lvgChanged";
			listViewGroup2->Header = L"Unchanged Settings";
			listViewGroup2->Name = L"lvgUnchanged";
			this->lvSettings->Groups->AddRange(gcnew cli::array< System::Windows::Forms::ListViewGroup^  >(2) {listViewGroup1, listViewGroup2});
			this->lvSettings->HideSelection = false;
			this->lvSettings->Location = System::Drawing::Point(0, 25);
			this->lvSettings->MultiSelect = false;
			this->lvSettings->Name = L"lvSettings";
			this->lvSettings->Size = System::Drawing::Size(815, 377);
			this->lvSettings->SmallImageList = this->ilImages;
			this->lvSettings->TabIndex = 1;
			this->lvSettings->UseCompatibleStateImageBehavior = false;
			this->lvSettings->View = System::Windows::Forms::View::Details;
			this->lvSettings->ItemActivate += gcnew System::EventHandler(this, &Configurator::EditItemDetails);
			// 
			// colName
			// 
			this->colName->Text = L"Name";
			this->colName->Width = 200;
			// 
			// colType
			// 
			this->colType->Text = L"Type";
			this->colType->Width = 100;
			// 
			// colValue
			// 
			this->colValue->Text = L"Value";
			this->colValue->Width = 350;
			// 
			// colDefaultValue
			// 
			this->colDefaultValue->Text = L"Default Value";
			this->colDefaultValue->Width = 150;
			// 
			// ilImages
			// 
			this->ilImages->ImageStream = (cli::safe_cast<System::Windows::Forms::ImageListStreamer^  >(resources->GetObject(L"ilImages.ImageStream")));
			this->ilImages->TransparentColor = System::Drawing::Color::Magenta;
			this->ilImages->Images->SetKeyName(0, L"Property");
			// 
			// dlgSaveFile
			// 
			this->dlgSaveFile->AddExtension = false;
			// 
			// Configurator
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(815, 402);
			this->Controls->Add(this->lvSettings);
			this->Controls->Add(this->tsToolbar);
			this->Name = L"Configurator";
			this->Text = L"Configurator";
			this->tsToolbar->ResumeLayout(false);
			this->tsToolbar->PerformLayout();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
private:
	System::Void EditItemDetails(System::Object^  sender, System::EventArgs^  e);
	System::Void NewConfiguration(System::Object^  sender, System::EventArgs^  e);
	System::Void OpenConfiguration(System::Object^  sender, System::EventArgs^  e);
	System::Void SaveConfiguration(System::Object^  sender, System::EventArgs^  e);

	System::Void CreateConfigurationView(parity::utils::Context& ctx);
	System::Void UpdateConfigurationView(parity::utils::Context& ctx);

	bool InitNewContext();

	parity::utils::Context* context_;
	parity::utils::Context* initial_;

public:
	static std::string MarshalSimpleStringToNative(String^ str);
	static String^ MarshalSimpleNativeToString(std::string& str);
};
}
