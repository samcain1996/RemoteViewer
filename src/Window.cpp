#include "Window.h"

wxBEGIN_EVENT_TABLE(StartUpWindow, wxFrame)
	EVT_BUTTON(10001, ClientButtonClick)
	EVT_BUTTON(10002, ServerButtonClick)
wxEND_EVENT_TABLE()

StartUpWindow::StartUpWindow() : GenericWindow() {
	
	_windowElements.clear();
	
	_windowElements.emplace_back(new wxButton(this, 10001, "Client", wxPoint(200, 200), wxSize(150, 50)));
	_windowElements.emplace_back(new wxButton(this, 10002, "Server", wxPoint(400, 200), wxSize(150, 50)));

}

StartUpWindow::~StartUpWindow() {
	wxExit();
}

void StartUpWindow::ClientButtonClick(wxCommandEvent& evt) {
	_windowElements.at(0)->SetLabelText("I've been clicked!");
}

void StartUpWindow::ServerButtonClick(wxCommandEvent& evt) {
	_windowElements.at(1)->SetLabelText("I've been clicked!");
}

GenericWindow::GenericWindow() : wxFrame(nullptr, wxID_ANY, "Remote Viewer", wxPoint(50, 50), wxSize(1270, 720)),
	_windowId(-1) {

}

GenericWindow::~GenericWindow() {
	
	_windowElements.clear();

	std::for_each(_windowElements.begin(), _windowElements.end(), [](wxControl* element) {
		delete element;
		});

}
