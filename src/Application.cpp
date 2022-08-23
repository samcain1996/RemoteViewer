#include "Application.h"

wxIMPLEMENT_APP(Application);

Application::Application() {
	
#if defined(_WIN32)
	// SetProcessDPIAware();
#endif

}

Application::~Application() {
	
}

bool Application::OnInit() {
	_window = new StartUpWindow();
	_window->Show();
	return true;
}

