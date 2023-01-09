#include "Windows/RemoteViewerWindows.h"

/*---------------Abstract Base Window------------------*/

BaseWindow::BaseWindow(const std::string& name, const wxPoint& pos, const wxSize& size, const bool show) : 
	wxFrame(nullptr, wxID_ANY, name, pos, size) {

	SetIcon(FetchIcon());
	IP_VALIDATOR.SetCharIncludes("0123456789.");

	_windowElements.clear();

	Show(show);
}

BaseWindow::~BaseWindow() {

	CleanUp();

	std::for_each(_windowElements.begin(), _windowElements.end(), [](wxControl* element) {
		delete element;
		});

	_windowElements.clear();
	
}

BaseWindow* BaseWindow::SpawnWindow(const WindowNames windowName) {
	
	BaseWindow* newWindow = nullptr;

	switch (windowName) {

	case WindowNames::StartUp:
		newWindow = new StartUpWindow(GetPosition(), GetSize());
		break;
	case WindowNames::ClientInit:
		newWindow = new ClientInitWindow(GetPosition(), GetSize());
		break;
	case WindowNames::ServerInit:
		newWindow = new ServerInitWindow(GetPosition(), GetSize());
		break;
	case WindowNames::UNDEFINED:
	default:
		newWindow = new StartUpWindow(GetPosition(), GetSize());
	}

	return newWindow;
}

void BaseWindow::OpenWindow(const WindowNames windowName) {

	_prevWindows.push(WindowName());

	SpawnWindow(windowName);

	Close(true);
}

void BaseWindow::GoBack() {

	CleanUp();
	// Return to the previous window
	if (_prevWindows.empty()) { wxExit(); return; }
	 
	SpawnWindow(_prevWindows.top());

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

//-----------------------Start Up Window-----------------------//

wxBEGIN_EVENT_TABLE(StartUpWindow, BaseWindow)
	EVT_BUTTON(10001, StartUpWindow::ClientButtonClick)
	EVT_BUTTON(10002, StartUpWindow::ServerButtonClick)
	EVT_KEY_UP(StartUpWindow::HandleInput)
wxEND_EVENT_TABLE()

StartUpWindow::StartUpWindow(const wxPoint& pos, const wxSize& size) : BaseWindow("Remote Viewer", pos, size) {

	_windowElements.clear();

	_clientButton = new wxButton(this, 10001, "Client", wxPoint(200, 200), wxSize(150, 50));
	_serverButton = new wxButton(this, 10002, "Server", wxPoint(400, 200), wxSize(150, 50));

	_windowElements.emplace_back(_clientButton);
	_windowElements.emplace_back(_serverButton);

	CenterElements(_windowElements);
}

StartUpWindow::~StartUpWindow() {}

void StartUpWindow::ClientButtonClick(wxCommandEvent& evt) {
	OpenWindow(WindowNames::ClientInit);
}

void StartUpWindow::ServerButtonClick(wxCommandEvent& evt) {
	OpenWindow(WindowNames::ServerInit);
}


/*------------------Client Initialization Window-----------------------*/

wxBEGIN_EVENT_TABLE(ClientInitWindow, BaseWindow)
	EVT_BUTTON(20004, ClientInitWindow::ConnectButtonClick)
	EVT_KEY_UP(ClientInitWindow::HandleInput)
wxEND_EVENT_TABLE()


ClientInitWindow::ClientInitWindow(const wxPoint& pos, const wxSize& size) : BaseWindow("Client Initialization", pos, size) {

	_ipInput = new wxTextCtrl(this, 20001, "192.168.50.160", wxPoint(100, 200), wxSize(500, 50), 0L, IP_VALIDATOR);
	_remotePortInput = new wxTextCtrl(this, 20002, "20009", wxPoint(100, 400), wxSize(200, 50), 0L, PORT_VALIDATOR);
	
	_connectButton = new wxButton(this, 20004, "Connect", wxPoint(400, 400), wxSize(200, 50));

	_windowElements.emplace_back(_remotePortInput);
	_windowElements.emplace_back(_ipInput);
	_windowElements.emplace_back(_connectButton);
}

ClientInitWindow::~ClientInitWindow() {}

void ClientInitWindow::ConnectButtonClick(wxCommandEvent& evt) {
	
	const std::string ipAddress = _ipInput->GetValue().ToStdString();
	
	const int remotePort = std::stoi(_remotePortInput->GetValue().ToStdString());

	ClientStreamWindow* clientStreamWindow = new ClientStreamWindow(ipAddress, remotePort, GetPosition(), GetSize());
	
	Close(true);
}
/*------------Pop Up------------*/

wxIMPLEMENT_CLASS(PopUp, wxPopupTransientWindow);

wxBEGIN_EVENT_TABLE(PopUp, wxPopupTransientWindow)
	EVT_BUTTON(90, PopUp::OnButton)
wxEND_EVENT_TABLE()

PopUp::PopUp(BaseWindow* parent, const std::string& message, Action&& OnClose) : 
	wxPopupTransientWindow(parent, wxBORDER_NONE | wxPU_CONTAINS_CONTROLS), onClose(OnClose) {

	_text = new wxStaticText(this, wxID_ANY, message, wxPoint(0, 0));
	_dismissButton = new wxButton(this, 90, "Dismiss", wxPoint(0, 100));

	SetClientSize(POPUP_SIZE);

	const wxPoint centerOfParent = parent->GetPosition() + wxPoint(parent->GetSize().GetWidth() / 2, parent->GetSize().GetHeight() / 2);
	SetPosition(centerOfParent - wxPoint(GetSize().GetWidth() / 2, GetSize().GetHeight() / 2));
	
}

PopUp::~PopUp() {
	
	delete _text;
	delete _dismissButton;
}

void PopUp::OnDismiss() {
	onClose();
}

void PopUp::OnButton(wxCommandEvent& evt) {
	Dismiss();
}