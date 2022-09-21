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
	case WindowNames::ClientStream:
		previousWindow = new ClientInitWindow(GetPosition(), GetSize());
		break;
	case WindowNames::ServerInit:
	case WindowNames::ServerStream:
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
	//			_windowElements[(index + 1) % _windowElements.size()]->SetFocus();  // Really good one copilot
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
}

StartUpWindow::~StartUpWindow() {}

void StartUpWindow::ClientButtonClick(wxCommandEvent& evt) {

	_prevWindows.emplace(WindowName());

	ClientInitWindow* clientWindow = new ClientInitWindow(GetPosition(), GetSize());

	Close(true);
}

void StartUpWindow::ServerButtonClick(wxCommandEvent& evt) {
	_prevWindows.emplace(WindowName());

	ServerInitWindow* serverWindow = new ServerInitWindow(GetPosition(), GetSize());

	Close(true);
}


/*------------------Client Initialization Window-----------------------*/

wxBEGIN_EVENT_TABLE(ClientInitWindow, BaseWindow)
	EVT_BUTTON(20004, ClientInitWindow::ConnectButtonClick)
	EVT_KEY_UP(ClientInitWindow::HandleInput)
wxEND_EVENT_TABLE()


ClientInitWindow::ClientInitWindow(const wxPoint& pos, const wxSize& size) : BaseWindow("Client Initialization", pos, size) {

	_ipInput = new wxTextCtrl(this, 20001, "127.0.0.1", wxPoint(100, 200), wxSize(500, 50), 0L, IP_VALIDATOR);
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

	ClientStreamWindow* clientStreamWindow = new ClientStreamWindow(ipAddress, localPort, remotePort, GetPosition(), GetSize());
	
	Close(true);
}

/*----------------------Client Streaming Window-----------------------*/

wxBEGIN_EVENT_TABLE(ClientStreamWindow, BaseWindow)
	EVT_PAINT(ClientStreamWindow::OnPaint)
	EVT_IDLE(ClientStreamWindow::BackgroundTask)
	EVT_KEY_UP(ClientStreamWindow::HandleInput)
	EVT_TIMER(1234, ClientStreamWindow::OnTick)
wxEND_EVENT_TABLE()

ClientStreamWindow::ClientStreamWindow(const std::string& ip, const Ushort localPort, 
	const Ushort remotePort, const wxPoint& pos, const wxSize& size) : BaseWindow("Remote Viewer - Master", pos, size), 
	_imageData(ScreenCapture::CalculateBMPFileSize() + BMP_HEADER_SIZE + MAX_PACKET_SIZE), _timer(this, 1234) {

	std::string message("Connecting to " + ip + ":" + std::to_string(remotePort));
	_popup = new PopUp(this, message);
	_popup->Popup();

	_client = new Client(ip);

	bool isWindows = true;
	
	_client->Connect(remotePort, [this, &isWindows]() {
		
		_popup->Destroy();
		
		ConnectMessageables(*this, *_client);

		_ioThr = std::thread(&Client::Start, _client);
		_timer.Start(1000 / _targetFPS);

		const BmpFileHeader header = ScreenCapture::ConstructBMPHeader(ScreenCapture::DefaultResolution, 32, !isWindows);

		std::copy(header.begin(), header.end(), _imageData.begin());
		
		_init = true;
		
		}, isWindows);

}

ClientStreamWindow::~ClientStreamWindow() {

	if (_client->Connected()) {
		_client->Disconnect();
		if (_ioThr.joinable()) { _ioThr.join(); }
	}

	delete _client;
}

void ClientStreamWindow::ImageBuilder() {

	MessageReader<ByteVec*>*& packetReader = msgReader;
	const int totalSize = ScreenCapture::CalculateBMPFileSize();
	const PixelData const pixeldata = &_imageData.data()[BMP_HEADER_SIZE];
	static int offset = 0;

	// Check  if there is a complete image
	while (!packetReader->Empty()) {

		ByteVec*  imageFragment = packetReader->ReadMessage();
		
		const size_t minSize = (size_t)(totalSize - offset);
		const int size = std::min(imageFragment->size(), minSize);

		std::memcpy(&pixeldata[offset], imageFragment->data(), size);
		offset += size;
		
		if (imageFragment->size() != MAX_PACKET_SIZE) {
			Logger::Log("client.log", "New Frame", true);
			offset = 0;
		}

		delete imageFragment;	
	}
	
}

void ClientStreamWindow::OnTick(wxTimerEvent& timerEvent) {
	_render = true;
	wxWakeUpIdle();
	_timer.Start(1000 / _targetFPS);
}

void ClientStreamWindow::PaintNow() {

	if (!_init || !_client->Connected()) { return; }

	ImageBuilder();
	
	wxClientDC dc(this);
	
	wxMemoryInputStream istream(_imageData.data(), _imageData.size());
	wxImage image(istream);
	wxBitmap bitmap(image);
	
	dc.DrawBitmap(bitmap, 0, 0);
}

void ClientStreamWindow::OnPaint(wxPaintEvent& evt) {
	
	// Do not paint until connected because no data is sent until then.
	if (!_init || !_client->Connected()) { evt.Skip(); }
	else { PaintNow(); }
}

void ClientStreamWindow::BackgroundTask(wxIdleEvent& evt) {

	if (!_init) { return; }

	if (_client->Connected()) {

		if (_render) {
			PaintNow();
			_render = false;
		}

	}

	else if (_ioThr.joinable()) {
		_ioThr.join();
	}
	
}

/*----------------Server Initialization Window---------------------*/

wxBEGIN_EVENT_TABLE(ServerInitWindow, BaseWindow)
	EVT_BUTTON(30001, ServerInitWindow::StartServer)
	EVT_KEY_UP(ServerInitWindow::HandleInput)
wxEND_EVENT_TABLE()

ServerInitWindow::ServerInitWindow(const wxPoint& pos, const wxSize& size) : 
	BaseWindow("Server Initialization", pos, size) {
	_portTb = new wxTextCtrl(this, 30002, "20009", wxPoint(100, 200), wxSize(500, 50), 0L, IP_VALIDATOR);

	_startServerButton = new wxButton(this, 30001, "Listen for connections", wxPoint(400, 400), wxSize(200, 50));

}

void ServerInitWindow::StartServer(wxCommandEvent& evt) {
	const int listenPort = std::stoi(_portTb->GetValue().ToStdString());

	ServerStreamWindow* streamWindow = new ServerStreamWindow(listenPort);

	Close(true);
}

ServerInitWindow::~ServerInitWindow() {}


/*----------------Server Streaming Window---------------------*/

wxBEGIN_EVENT_TABLE(ServerStreamWindow, BaseWindow)
	EVT_IDLE(ServerStreamWindow::BackgroundTask)
	EVT_TIMER(2345, ServerStreamWindow::OnTick)
wxEND_EVENT_TABLE()


ServerStreamWindow::ServerStreamWindow(const int listenPort) : 
	BaseWindow(std::string(), wxPoint(1000, 0), wxSize(10, 10)), _timer(this, 2345) {
	
	_server = new Server(listenPort, std::chrono::seconds(10));

	std::string message("Listening on port " + std::to_string(listenPort));
	_popup = new PopUp(this, message);
	_popup->Popup();

	_init = true;

	_ioThr = std::thread(&Server::Listen, _server);

}

ServerStreamWindow::~ServerStreamWindow() {
	if (_server->Connected()) {
		_server->Disconnect();
	}
	delete _server;
}

void ServerStreamWindow::BackgroundTask(wxIdleEvent& evt) {
	
	if (!_init) { return; }

	if (_server->Connected()) {

		if (_ioThr.joinable()) {

			_ioThr.join();

			delete _popup;

			_timer.Start(1000 / _targetFPS);


		}

		else if (!_server->Serve()) {
			GoBack();
		}

	}
}

void ServerStreamWindow::OnTick(wxTimerEvent& timerEvent) {
	wxWakeUpIdle();
	_timer.Start(1000 / _targetFPS);
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

	const wxPoint centerOfParent = parent->GetPosition() + wxPoint(parent->GetSize().GetWidth() / 2, parent->GetSize().GetHeight() / 2);
	SetPosition(centerOfParent - wxPoint(GetSize().GetWidth() / 2, GetSize().GetHeight() / 2));
	
}

PopUp::~PopUp() {
	
	delete _text;
	delete _dismissButton;
}

void PopUp::OnButton(wxCommandEvent& evt) {
	Dismiss();
}


