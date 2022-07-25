#include "RemoteViewerWindows.h"

WindowStack BaseWindow::_prevWindows;

BaseWindow::BaseWindow(const std::string& name, const wxPoint& pos, const wxSize& size) : 
	wxFrame(nullptr, wxID_ANY, name, pos, size), _windowId(-1) {
	IP_VALIDATOR.SetCharIncludes("0123456789.");
}


BaseWindow::BaseWindow(const std::string& name) : BaseWindow(name, wxPoint(50, 50), wxSize(1270, 720)) {}

BaseWindow::~BaseWindow() {

	_windowElements.clear();

	std::for_each(_windowElements.begin(), _windowElements.end(), [](wxControl* element) {
		delete element;
		});

	if (_killProgramOnClose) { wxExit(); }

}

void BaseWindow::HandleInput(wxKeyEvent& keyEvent) {

	int keycode = keyEvent.GetKeyCode();
	
	//if (keycode == WXK_TAB) {
	//	
	//	for (int index = 0; index < _windowElements.size(); ++index) {
	//		if (_windowElements[index]->HasFocus()) {
	//			_windowElements[(index + 1) % _windowElements.size()]->SetFocus();  // Really good one copilot
	//			break;
	//		}
	//	}
	//	
	//}

	if (keycode == WXK_BACK) {

		if (_prevWindows.empty()) { wxExit(); return; }

		BaseWindow* previousWindow;

		switch (_prevWindows.top()) {

			case WindowNames::StartUp:
				previousWindow = new StartUpWindow();
				break;
			case WindowNames::ClientInit:
				previousWindow = new ClientInitWindow(GetPosition(), GetSize());
				break;
			case WindowNames::ServerInit:
				previousWindow = new ServerInitWindow(GetPosition(), GetSize());
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



wxBEGIN_EVENT_TABLE(StartUpWindow, BaseWindow)
	EVT_BUTTON(10001, StartUpWindow::ClientButtonClick)
	EVT_BUTTON(10002, StartUpWindow::ServerButtonClick)
wxEND_EVENT_TABLE()

StartUpWindow::StartUpWindow() : BaseWindow("Remote Viewer") {
	
	_windowElements.clear();

	_clientButton = new wxButton(this, 10001, "Client", wxPoint(200, 200), wxSize(150, 50));
	_serverButton = new wxButton(this, 10002, "Server", wxPoint(400, 200), wxSize(150, 50));
	
	_windowElements.emplace_back(_clientButton);
	_windowElements.emplace_back(_serverButton);

}

StartUpWindow::~StartUpWindow() {}

void StartUpWindow::ClientButtonClick(wxCommandEvent& evt) {

	_prevWindows.emplace(WindowName());

	ClientInitWindow* clientWindow = new ClientInitWindow(GetPosition(), GetSize());
	clientWindow->Show();

	_killProgramOnClose = false;
	Close(true);
}

void StartUpWindow::ServerButtonClick(wxCommandEvent& evt) {
	_prevWindows.emplace(WindowName());

	ServerInitWindow* serverWindow = new ServerInitWindow(GetPosition(), GetSize());
	serverWindow->Show();

	_killProgramOnClose = false;
	Close(true);
}



wxBEGIN_EVENT_TABLE(ClientInitWindow, BaseWindow)
	EVT_BUTTON(20004, ClientInitWindow::ConnectButtonClick)
	EVT_KEY_UP(ClientInitWindow::HandleInput)
wxEND_EVENT_TABLE()


ClientInitWindow::ClientInitWindow(const wxPoint& pos, const wxSize& size) : BaseWindow("Client Initialization", pos, size) {

	_ipInput = new wxTextCtrl(this, IP_TB_ID, "192.168.50.160", wxPoint(100, 200), wxSize(500, 50), 0L, IP_VALIDATOR);
	_remotePortInput = new wxTextCtrl(this, PORT_TB_ID, "20009", wxPoint(100, 400), wxSize(200, 50), 0L, PORT_VALIDATOR);
	_localPortInput = new wxTextCtrl(this, PORT_TB_ID+1, "10009", wxPoint(100, 550), wxSize(200, 50), 0L, PORT_VALIDATOR);
	
	_connectButton = new wxButton(this, 20004, "Connect", wxPoint(400, 400), wxSize(200, 50));

	_windowElements.emplace_back(_remotePortInput);
	_windowElements.emplace_back(_localPortInput);
	_windowElements.emplace_back(_ipInput);
	_windowElements.emplace_back(_connectButton);
}

ClientInitWindow::~ClientInitWindow() {}

void ClientInitWindow::ConnectButtonClick(wxCommandEvent& evt) {
	const std::string ipAddress = _ipInput->GetValue().ToStdString();
	const int remotePort = std::stoi(_remotePortInput->GetValue().ToStdString());
	const int localPort = std::stoi(_localPortInput->GetValue().ToStdString());

	_killProgramOnClose = false;
	Close(true);

	ClientStreamWindow* streamWindow = new ClientStreamWindow(ipAddress, localPort, remotePort);
	streamWindow->Show();
}

wxBEGIN_EVENT_TABLE(ClientStreamWindow, BaseWindow)
	EVT_PAINT(ClientStreamWindow::OnPaint)
	EVT_IDLE(ClientStreamWindow::OnIdle)
wxEND_EVENT_TABLE()

ClientStreamWindow::ClientStreamWindow(const std::string& ip, int localPort, int remotePort) : BaseWindow("Remote Viewer - Master") {

	client = new Client(localPort, ip);
	client->Connect(std::to_string(remotePort));

	while (!_connected) {
		client->Handshake(_connected);
		client->_io_context.run();
	}
	ConnectMessageables(*this, *client);
	clientThr = std::thread(&Client::Receive, client);
	clientThr.detach();

}

ClientStreamWindow::~ClientStreamWindow() {
	//if (_connected) {
	//	clientThr.join();
	//}
}

bool ClientStreamWindow::AssembleImage() {
	if (!groupReader->Empty()) {
		PacketPriorityQueue* queue = groupReader->ReadMessage();
		
		int size = MAX_PACKET_PAYLOAD_SIZE * queue->size();
		ByteArray imgData = new Byte[MAX_PACKET_PAYLOAD_SIZE * queue->size()];
		
		for (int packetNo = 0; !queue->empty(); ++packetNo) {
			Packet packet = queue->top();
			queue->pop();
			
			std::memcpy(&imgData[packetNo * MAX_PACKET_PAYLOAD_SIZE], packet.Payload().data(), 
				packet.Payload().size());
		}

		wxMemoryInputStream stream(imgData, size);
		_image = wxImage(stream);

		_skip = false;
		
		delete[] imgData;

		delete queue;
		return true;
	}
	return false;
}

void ClientStreamWindow::PaintNow() {
	wxClientDC dc(this);

	wxBitmap bitmap(_image);
	dc.DrawBitmap(bitmap, 0, 0);
}

void ClientStreamWindow::OnPaint(wxPaintEvent& evt) {
	if (!_skip) {
		PaintNow();
	}
}

void ClientStreamWindow::OnIdle(wxIdleEvent& evt) {
	
	if (AssembleImage()) {
		PaintNow();
	}
	evt.RequestMore();
}

wxBEGIN_EVENT_TABLE(ServerInitWindow, BaseWindow)
	EVT_BUTTON(30001, ServerInitWindow::ListenButtonClick)
wxEND_EVENT_TABLE()

ServerInitWindow::ServerInitWindow(const wxPoint& pos, const wxSize& size) : BaseWindow("Server Initialization", pos, size) {
	_portTb = new wxTextCtrl(this, 30002, "20009", wxPoint(100, 200), wxSize(500, 50), 0L, IP_VALIDATOR);

	_listenButton = new Button(this, 30001, "Listen", wxPoint(400, 400), wxSize(200, 50));
}

ServerInitWindow::~ServerInitWindow() {
}

void ServerInitWindow::ListenButtonClick(wxCommandEvent& evt) {
	const int listenPort = std::stoi(_portTb->GetValue().ToStdString());
	
	bool tmpDummy = false;

	Server server(listenPort);
	server.Handshake(tmpDummy);
	
	broadcastThr = std::thread(&Server::Serve, &server);
	broadcastThr.detach();

	_killProgramOnClose = false;
	Close(true);
}


