#include "Windows/BaseWindow.h"

/*---------------Abstract Base Window------------------*/

BaseWindow::BaseWindow(const std::string& name, const wxPoint& pos, const wxSize& size, const bool show) :
	wxFrame(nullptr, wxID_ANY, name, pos, size) {

	SetIcon(FetchIcon());
	IP_VALIDATOR.SetCharIncludes("0123456789.");

	_windowElements.clear();

	Show(show);
}

BaseWindow::~BaseWindow() {
	std::for_each(_windowElements.begin(), _windowElements.end(), [](wxControl* element) {
		delete element;
		});

	_windowElements.clear();

}

void BaseWindow::GoBack() {

	// Return to the previous window
	// MEMORY LEAK?? I don't see how this wouldn't cause one...

	if (_prevWindows.empty()) { wxExit(); return; }

	BaseWindow* previousWindow = nullptr;

	switch (_prevWindows.top()) {

	case WindowNames::StartUp:
		previousWindow = new StartUpWindow(GetPosition(), GetSize());
		break;
	case WindowNames::ClientInit:
		previousWindow = new ClientInitWindow(GetPosition(), GetSize());
		break;
	case WindowNames::ServerInit:
		previousWindow = new ServerInitWindow(GetPosition(), GetSize());
		break;
	case WindowNames::UNDEFINED:
	default:
		previousWindow = new StartUpWindow(GetPosition(), GetSize());
	}

	_prevWindows.pop();

	Close(true);
}

void BaseWindow::HandleInput(wxKeyEvent& keyEvent) {

	int keycode = keyEvent.GetKeyCode();

	// Move between controls on form 

	//if (keycode == WXK_TAB) {
	//	
	//	for (int index = 0; index < _windowElements.size(); ++index) {
	//		if (_windowElements[index]->HasFocus()) {
	//			_windowElements[(index + 1) % _windowElements.size()]->SetFocus();
	//			break;
	//		}
	//	}
	//	
	//}

	if (keycode == WXK_BACK) { GoBack(); }
}