#include "Windows/RemoteViewerWindows.h"

/*---------------Abstract Base Window------------------*/

BaseWindow::BaseWindow(const string& name, const wxPoint& pos, const wxSize& size, const bool show) : 
	wxFrame(nullptr, wxID_ANY, name, pos, size) {

	SetIcon(FetchIcon());
	IP_VALIDATOR.SetCharIncludes("0123456789.");

	_popup = make_unique<PopUp>(this);

	Show(show);
}

BaseWindow::~BaseWindow() {
	CleanUp();
}

void BaseWindow::CleanUp() {

	_windowElements.clear();

}

void BaseWindow::OpenWindow(const WindowNames windowName, const string& ip,
	const bool close) {

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
	case WindowNames::ClientStream:
		newWindow = new ClientStreamWindow(ip, GetPosition(), GetSize());
		break;
	case WindowNames::UNDEFINED:
	default:
		newWindow = new StartUpWindow(GetPosition(), GetSize());
	}

	if (close) { Close(true); }

}

void BaseWindow::GoBack() {

	CleanUp();
	
	// Return to the previous window
	if (_prevWindows.empty()) { wxExit(); return; }
	 
	auto window = _prevWindows.top();
	_prevWindows.pop();
	OpenWindow(window);

}

void BaseWindow::HandleInput(wxKeyEvent& keyEvent) {

	switch (keyEvent.GetKeyCode()) {
		case WXK_BACK: 
			GoBack();
			break;
		case WXK_ESCAPE:
			wxExit();
			break;
	}
}

//-----------------------Start Up Window-----------------------//

wxBEGIN_EVENT_TABLE(StartUpWindow, BaseWindow)
	EVT_BUTTON(10001, StartUpWindow::ClientButtonClick)
	EVT_BUTTON(10002, StartUpWindow::ServerButtonClick)
	EVT_KEY_UP(StartUpWindow::HandleInput)
wxEND_EVENT_TABLE()

StartUpWindow::StartUpWindow(const wxPoint& pos, const wxSize& size) : BaseWindow("Remote Viewer", pos, size),
	_clientButton(this, 10001, "Client", wxPoint(200, 200), wxSize(150, 50)),
	_serverButton(this, 10002, "Server", wxPoint(400, 200), wxSize(150, 50)) {

	_windowElements.emplace_back(&_clientButton);
	_windowElements.emplace_back(&_serverButton);

	CenterElements(_windowElements);
}

StartUpWindow::~StartUpWindow() {}

void StartUpWindow::ClientButtonClick(wxCommandEvent& evt) {
	_prevWindows.push(WindowName());
	OpenWindow(WindowNames::ClientInit);
}

void StartUpWindow::ServerButtonClick(wxCommandEvent& evt) {
	_prevWindows.push(WindowName());
	OpenWindow(WindowNames::ServerInit);
}


/*------------------Client Initialization Window-----------------------*/

wxBEGIN_EVENT_TABLE(ClientInitWindow, BaseWindow)
	EVT_BUTTON(20004, ClientInitWindow::ConnectButtonClick)
	EVT_KEY_UP(ClientInitWindow::HandleInput)
wxEND_EVENT_TABLE()


ClientInitWindow::ClientInitWindow(const wxPoint& pos, const wxSize& size) : BaseWindow("Client Initialization", pos, size),
	_ipInput(this, 20001, "127.0.0.1", wxPoint(100, 200), wxSize(500, 50), 0L, IP_VALIDATOR),
	_connectButton(this, 20004, "Connect", wxPoint(400, 400), wxSize(200, 50)) {

	_windowElements.emplace_back(&_ipInput);
	_windowElements.emplace_back(&_connectButton);
}

ClientInitWindow::~ClientInitWindow() {}

void ClientInitWindow::ConnectButtonClick(wxCommandEvent& evt) {
	_prevWindows.push(WindowName());
	OpenWindow(WindowNames::ClientStream, _ipInput.GetValue().ToStdString());
}
/*------------Pop Up------------*/

wxIMPLEMENT_CLASS(PopUp, wxPopupTransientWindow);

wxBEGIN_EVENT_TABLE(PopUp, wxPopupTransientWindow)
	EVT_BUTTON(90, PopUp::OnButton)
wxEND_EVENT_TABLE()

PopUp::PopUp(BaseWindow* parent, const string& message, const Action& OnClose) : 
	wxPopupTransientWindow(parent, wxBORDER_NONE | wxPU_CONTAINS_CONTROLS), onClose(OnClose) {

	_text = new wxTextCtrl(this, wxID_ANY, message, wxPoint(0, 0), wxSize(300, 50));
	_text->SetEditable(false);

	_dismissButton = new wxButton(this, 90, "Dismiss", wxPoint(0, 100));

	const wxPoint centerOfParent = parent->GetPosition() + wxPoint(parent->GetSize().GetWidth() / 2, parent->GetSize().GetHeight() / 2);

	SetClientSize(DEF_POPUP_SIZE);
	SetPosition(centerOfParent - wxPoint(GetSize().GetWidth() / 2, GetSize().GetHeight() / 2));
	_text->SetPosition(_text->GetPosition() + wxPoint(GetSize().GetWidth() / 2 - _text->GetSize().GetWidth() / 2, GetSize().GetHeight() / 2 - _text->GetSize().GetHeight() / 2));
	_dismissButton->SetPosition(_dismissButton->GetPosition() + wxPoint(GetSize().GetWidth() / 2 - _dismissButton->GetSize().GetWidth() / 2, GetSize().GetHeight() / 2 - _dismissButton->GetSize().GetHeight() / 2));
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