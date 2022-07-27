#include "RemoteViewerWindows.h"

/*---------------Abstract Base Window------------------*/

WindowStack BaseWindow::_prevWindows;

BaseWindow::BaseWindow(const std::string& name, const wxPoint& pos, const wxSize& size, const bool show) : 
	wxFrame(nullptr, wxID_ANY, name, pos, size) {
	IP_VALIDATOR.SetCharIncludes("0123456789.");

	_windowElements.clear();

	Show(show);
}

BaseWindow::~BaseWindow() {

	_windowElements.clear();

	std::for_each(_windowElements.begin(), _windowElements.end(), [](wxControl* element) {
		delete element;
		});

	if (_killProgramOnClose) { wxExit(); }

}

void BaseWindow::HandleInput(wxKeyEvent& keyEvent) {

	int keycode = keyEvent.GetKeyCode();

	// Move between controls on form 
	
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

	// Return to the previous window if backspace is pressed
	// MEMORY LEAK?? I don't see how this wouldn't cause one...

	if (keycode == WXK_BACK) {

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
		previousWindow->Show();

		_killProgramOnClose = false;
		Close(true);

	}
}

//-----------------------Start Up Window-----------------------//

wxBEGIN_EVENT_TABLE(StartUpWindow, BaseWindow)
	EVT_BUTTON(10001, StartUpWindow::ClientButtonClick)
	EVT_BUTTON(10002, StartUpWindow::ServerButtonClick)
wxEND_EVENT_TABLE()

StartUpWindow::StartUpWindow(const wxPoint& pos, const wxSize& size) : BaseWindow("Remote Viewer", pos, size) {

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

	_killProgramOnClose = false;
	Close(true);
}

void StartUpWindow::ServerButtonClick(wxCommandEvent& evt) {
	_prevWindows.emplace(WindowName());

	ServerInitWindow* serverWindow = new ServerInitWindow(GetPosition(), GetSize());

	_killProgramOnClose = false;
	Close(true);
}


/*------------------Client Initialization Window-----------------------*/

wxBEGIN_EVENT_TABLE(ClientInitWindow, BaseWindow)
	EVT_BUTTON(20004, ClientInitWindow::ConnectButtonClick)
	EVT_KEY_UP(ClientInitWindow::HandleInput)
wxEND_EVENT_TABLE()


ClientInitWindow::ClientInitWindow(const wxPoint& pos, const wxSize& size) : BaseWindow("Client Initialization", pos, size) {

	_ipInput = new wxTextCtrl(this, 20001, "192.168.50.160", wxPoint(100, 200), wxSize(500, 50), 0L, IP_VALIDATOR);
	_remotePortInput = new wxTextCtrl(this, 20002, "20009", wxPoint(100, 400), wxSize(200, 50), 0L, PORT_VALIDATOR);
	_localPortInput = new wxTextCtrl(this, 20003, "10009", wxPoint(100, 550), wxSize(200, 50), 0L, PORT_VALIDATOR);
	
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
	const int localPort  = std::stoi(_localPortInput->GetValue().ToStdString());

	ClientStreamWindow* clientStreamWindow = new ClientStreamWindow(ipAddress, localPort, remotePort);
	
	_killProgramOnClose = false;
	Close(true);
}

/*----------------------Client Streaming Window-----------------------*/

wxBEGIN_EVENT_TABLE(ClientStreamWindow, BaseWindow)
	EVT_PAINT(ClientStreamWindow::OnPaint)
	EVT_IDLE(ClientStreamWindow::BackgroundTask)
wxEND_EVENT_TABLE()

ClientStreamWindow::ClientStreamWindow(const std::string& ip, const Ushort localPort, const Ushort remotePort,
	const wxPoint& pos, const wxSize& size) : BaseWindow("Remote Viewer - Master", pos, size) {

	_client = new Client(localPort, ip);
	_client->Connect(std::to_string(remotePort));

	std::string message("Connecting to " + ip + ":" + std::to_string(remotePort));
	_popup = new PopUp(this, message);
	_popup->Popup();

}

ClientStreamWindow::~ClientStreamWindow() {
	_client->Send((ByteArray)Client::DISCONNECT_MESSAGE, Client::DISCONNECT_SIZE);
	_clientThr.join();
}

const bool ClientStreamWindow::AssembleImage() {
	
	// Check  if there is a complete image
	if (!groupReader->Empty()) {
		
		// Get the packets to construct the image
		PacketPriorityQueue* queue = groupReader->ReadMessage();
		
		// Allocate memory for the image if needed
		const int imgSize = MAX_PACKET_PAYLOAD_SIZE * queue->size();
		if (imgSize != _imgSize) {
			if (_imgData != nullptr) { delete[] _imgData; }
			_imgData = new Byte[imgSize];
			_imgSize = imgSize;
		}

		// Assemble image from packets
		int offset = 0;
		for (int packetNo = 0; !queue->empty(); ++packetNo) {
			Packet packet = queue->top();
			queue->pop();
			
			std::memcpy(&_imgData[offset], packet.Payload().data(), 
				packet.Payload().size());
			offset += packet.Payload().size();
		}
		
		delete queue;
		return true;
	}
	
	// Nothing available to render
	return false;
}

void ClientStreamWindow::PaintNow() {
	wxClientDC dc(this);

	if (_imgData == nullptr) { return; }

	wxMemoryInputStream istream(_imgData, _imgSize);
	wxImage image(istream);
	wxBitmap bitmap(image);
	dc.DrawBitmap(bitmap, 0, 0);
}

void ClientStreamWindow::OnPaint(wxPaintEvent& evt) {
	
	// Do not paint until connected because no data is sent until then.
	if (!_connected) { return; }
	PaintNow();
}

void ClientStreamWindow::BackgroundTask(wxIdleEvent& evt) {

	// TODO: Limit connect attempts

	// Try to connect if not connected
	if (!_connected) {
	
		_client->Handshake(_connected);
		
		_client->_io_context.run();
		_client->_io_context.restart();
		
	}

	else {

		// Delete popup on connect
		if (_popup != nullptr) {
			
			delete _popup;
			_popup = nullptr;

			ConnectMessageables(*this, *_client);
			_clientThr = std::thread(&Client::AsyncReceive, _client);
			
		}

		// If there is a complete image, render it
		else if (AssembleImage()) { PaintNow(); }

	}

	evt.RequestMore();
	
}

/*----------------Server Initialization Window---------------------*/

wxBEGIN_EVENT_TABLE(ServerInitWindow, BaseWindow)
	EVT_BUTTON(30001, ServerInitWindow::StartServer)
	EVT_IDLE(ServerInitWindow::BackgroundTask)
	EVT_KEY_UP(BaseWindow::HandleInput)
wxEND_EVENT_TABLE()

ServerInitWindow::ServerInitWindow(const wxPoint& pos, const wxSize& size) : BaseWindow("Server Initialization", pos, size) {
	_portTb = new wxTextCtrl(this, 30002, "20009", wxPoint(100, 200), wxSize(500, 50), 0L, IP_VALIDATOR);

	_startServerButton = new wxButton(this, 30001, "Listen for connections", wxPoint(400, 400), wxSize(200, 50));
}

ServerInitWindow::~ServerInitWindow() {}

void ServerInitWindow::StartServer(wxCommandEvent& evt) {
	const int listenPort = std::stoi(_portTb->GetValue().ToStdString());
	
	_server = new Server(listenPort);

	_killProgramOnClose = false;
}

void ServerInitWindow::BackgroundTask(wxIdleEvent& evt) {

	if (_server == nullptr) { return; }

	bool dummy = false;

	// TODO: Make Server non-blocking	
	_server->Handshake(dummy);

	_server->Serve();
	
}

/*------------Pop Up------------*/

wxIMPLEMENT_CLASS(PopUp, wxPopupTransientWindow);

wxBEGIN_EVENT_TABLE(PopUp, wxPopupTransientWindow)
	EVT_BUTTON(90, PopUp::OnButton)
wxEND_EVENT_TABLE()

PopUp::PopUp(BaseWindow* parent, const std::string& message) : 
	wxPopupTransientWindow(parent, wxBORDER_NONE | wxPU_CONTAINS_CONTROLS) {

	_text = new wxStaticText(this, wxID_ANY, message, wxPoint(100, 0));
	_dismissButton = new wxButton(this, 90, "Dismiss", wxPoint(100, 100));

	SetClientSize(POPUP_SIZE);
	SetPosition(wxPoint(parent->GetPosition().x / 2 + POPUP_SIZE.x / 2, parent->GetPosition().y / 2 + POPUP_SIZE.y / 2));
}

PopUp::~PopUp() {
	
	delete _text;
	delete _dismissButton;
}

void PopUp::OnButton(wxCommandEvent& evt) {
	Dismiss();
}




