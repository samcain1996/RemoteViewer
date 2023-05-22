#include "Application.h"

wxIMPLEMENT_APP(Application);

Application::Application() {}

Application::~Application() {}

bool Application::OnInit() {
	ScreenCapture screen;
	screen.CaptureScreen();
	screen.SaveToFile();
	Logger::newStream("global.log", std::ios_base::binary);
	ScreenCapture::CalibrateResolution();
	wxInitAllImageHandlers();

	_window = new StartUpWindow();
	_window->Show();
	return true;
}

