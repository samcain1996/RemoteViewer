#include "Application.h"

wxIMPLEMENT_APP(Application);

Application::Application() {

	
}

Application::~Application() {
	
}

bool Application::OnInit() {
	_window = new StartUpWindow();
	_window->Show();
	return true;
}

