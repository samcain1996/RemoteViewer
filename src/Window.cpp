#include "Window.h"

wxBEGIN_EVENT_TABLE(StartUpWindow, wxFrame)
	EVT_BUTTON(10001, ClientButtonClick)
	EVT_BUTTON(10002, ServerButtonClick)
	EVT_CHAR_HOOK(BackSpace)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(ClientInitWindow, wxFrame)
	EVT_CHAR_HOOK(BackSpace)
wxEND_EVENT_TABLE()


GenericWindow::GenericWindow(const std::string& name) : wxFrame(nullptr, wxID_ANY, name, wxPoint(50, 50), wxSize(1270, 720)),
_windowId(-1) {

}

GenericWindow::~GenericWindow() {

	_windowElements.clear();

	std::for_each(_windowElements.begin(), _windowElements.end(), [](wxControl* element) {
		delete element;
		});

}

std::stack<WindowNames> GenericWindow::_prevWindows;

void GenericWindow::BackSpace(wxKeyEvent& keyEvent) {
	
	if (keyEvent.GetKeyCode() == WXK_BACK) {

		if (_prevWindows.empty()) { wxExit(); return; }

		GenericWindow* previousWindow;

		switch (_prevWindows.top()) {

			case WindowNames::StartUp:
				previousWindow = new StartUpWindow();
				break;
			case WindowNames::ClientInit:
				previousWindow = new ClientInitWindow();
				break;
			default:
				previousWindow = new StartUpWindow();
		}

		_prevWindows.pop();
		previousWindow->Show();
		Close(true);

	}
}


StartUpWindow::StartUpWindow() : GenericWindow("Remote Viewer") {
	
	_windowElements.clear();
	
	_windowElements.emplace_back(new wxButton(this, 10001, "Client", wxPoint(200, 200), wxSize(150, 50)));
	_windowElements.emplace_back(new wxButton(this, 10002, "Server", wxPoint(400, 200), wxSize(150, 50)));

}

StartUpWindow::~StartUpWindow() {
}

void StartUpWindow::ClientButtonClick(wxCommandEvent& evt) {

	_prevWindows.emplace(WindowName());

	ClientInitWindow* clientWindow = new ClientInitWindow();
	clientWindow->Show();

	Close(true);
}

void StartUpWindow::ServerButtonClick(wxCommandEvent& evt) {
	_windowElements.at(1)->SetLabelText("I've been clicked!");
}

constexpr const WindowNames StartUpWindow::WindowName() {
	return WindowNames::StartUp;
}

ClientInitWindow::ClientInitWindow() : GenericWindow("Client Initialization") {

	_windowElements.emplace_back(new wxButton(this, 10003, "YOU DID IT NICE!", wxPoint(200, 600), wxSize(500, 500)));

}

ClientInitWindow::~ClientInitWindow() {
}

constexpr const WindowNames ClientInitWindow::WindowName() {
	return WindowNames::ClientInit;
}
