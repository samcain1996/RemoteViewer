#include "RemoteViewerWindows.h"

wxBEGIN_EVENT_TABLE(StartUpWindow, wxFrame)
	EVT_BUTTON(10001, ClientButtonClick)
	EVT_BUTTON(10002, ServerButtonClick)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(ClientInitWindow, wxFrame)
	EVT_BUTTON(20001, ConnectButtonClick)
wxEND_EVENT_TABLE()

WindowStack BaseWindow::_prevWindows;

BaseWindow::BaseWindow(const std::string& name) : wxFrame(nullptr, wxID_ANY, name, wxPoint(50, 50), wxSize(1270, 720)),
_windowId(-1) {
	IP_VALIDATOR.SetCharIncludes("0123456789.");
}

BaseWindow::~BaseWindow() {

	_windowElements.clear();

	std::for_each(_windowElements.begin(), _windowElements.end(), [](wxControl* element) {
		delete element;
		});

	if (_killProgramOnClose) { wxExit(); }

}

void BaseWindow::HandleInput(wxKeyEvent& keyEvent) {
	
	if (keyEvent.GetKeyCode() == WXK_BACK) {

		if (_prevWindows.empty()) { wxExit(); return; }

		BaseWindow* previousWindow;

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

		_killProgramOnClose = false;
		Close(true);

	}
}

StartUpWindow::StartUpWindow() : BaseWindow("Remote Viewer") {
	
	_windowElements.clear();
	
	_windowElements.emplace_back(new wxButton(this, 10001, "Client", wxPoint(200, 200), wxSize(150, 50)));
	_windowElements.emplace_back(new wxButton(this, 10002, "Server", wxPoint(400, 200), wxSize(150, 50)));

}

StartUpWindow::~StartUpWindow() {}

void StartUpWindow::ClientButtonClick(wxCommandEvent& evt) {

	_prevWindows.emplace(WindowName());

	ClientInitWindow* clientWindow = new ClientInitWindow();
	clientWindow->Show();

	_killProgramOnClose = false;
	Close(true);
}

void StartUpWindow::ServerButtonClick(wxCommandEvent& evt) {
	_windowElements.at(1)->SetLabelText("I've been clicked!");
}

constexpr const WindowNames StartUpWindow::WindowName() {
	return WindowNames::StartUp;
}



ClientInitWindow::ClientInitWindow() : BaseWindow("Client Initialization") {

	_ipInput = new wxTextCtrl(this, IP_TB_IP, "", wxPoint(100, 200), wxSize(500, 50), 0L, IP_VALIDATOR);
	_portInput = new wxTextCtrl(this, PORT_TB_ID, "", wxPoint(100, 400), wxSize(200, 50), 0L, PORT_VALIDATOR);
	
	_connectButton = new wxButton(this, 20004, "Connect", wxPoint(400, 400), wxSize(200, 50));

	_windowElements.emplace_back(_portInput);
	_windowElements.emplace_back(_ipInput);
	_windowElements.emplace_back(_connectButton);
}

ClientInitWindow::~ClientInitWindow() {}

void ClientInitWindow::HandleInput(wxKeyEvent& keyEvent) {

	int keycode = keyEvent.GetKeyCode();
	
	if (_ipInput->HasFocus()) {
		_ipInput->AppendText((char)keycode);
	}
	else if (_portInput->HasFocus()) {
		_portInput->AppendText((char)keycode);
	}
	else {
		BaseWindow::HandleInput(keyEvent);
	}
}

void ClientInitWindow::ConnectButtonClick(wxCommandEvent& evt) {
	const std::string ipAddress = _ipInput->GetValue().ToStdString();
	const std::string port = _portInput->GetValue().ToStdString();

	// TODO: Connect
}

constexpr const WindowNames ClientInitWindow::WindowName() {
	return WindowNames::ClientInit;
}
