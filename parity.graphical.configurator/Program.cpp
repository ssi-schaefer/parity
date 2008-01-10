// parity.graphical.configurator.cpp: Hauptprojektdatei.

#include "Configurator.h"

using namespace paritygraphicalconfigurator;

[STAThreadAttribute]
int main(array<System::String ^> ^args)
{
	// Aktivieren visueller Effekte von Windows XP, bevor Steuerelemente erstellt werden
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false); 

	// Hauptfenster erstellen und ausführen
	Application::Run(gcnew Configurator());
	return 0;
}
