#include "Window.h"

StartUpWindow::StartUpWindow() : GenericWindow() {
	
	_windowElements.clear();
	
	_windowElements.emplace_back(new wxButton(this, wxID_ANY, "Client", wxPoint(200, 200), wxSize(150, 50)));
	_windowElements.emplace_back(new wxButton(this, wxID_ANY, "Server", wxPoint(400, 200), wxSize(150, 50)));

}

StartUpWindow::~StartUpWindow() {
	
}

GenericWindow::~GenericWindow() {
	
	_windowElements.clear();

	std::for_each(_windowElements.begin(), _windowElements.end(), [](wxControl* element) {
		delete element;
		});

}
