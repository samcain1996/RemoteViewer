#include "Windows/RemoteViewerWindows.h"

/*----------------------Client Streaming Window-----------------------*/

wxBEGIN_EVENT_TABLE(ClientStreamWindow, BaseWindow)
EVT_PAINT(ClientStreamWindow::OnPaint)
EVT_IDLE(ClientStreamWindow::BackgroundTask)
EVT_KEY_UP(ClientStreamWindow::HandleInput)
EVT_TIMER(1234, ClientStreamWindow::OnTick)
wxEND_EVENT_TABLE()

ClientStreamWindow::ClientStreamWindow(const string& ip, const wxPoint& pos, const wxSize& size) :
	BaseWindow("Remote Viewer - Master", pos, size), _imageData(CalculateBMPFileSize()), _timer(this, 1234) {

	// Create client to receive data from other computer
	_client = make_shared<Client>(ip);

	_popup->SetText("Connecting to " + ip);
	_popup->Popup();

	int counter = 0;

	// Set up data streams
	for_each(_client->connections.begin(), _client->connections.end(),
		[this, &counter](ConnectionPtr& pConnection) {

			// Attempt to connect to other computer on separate thread so
			// program is still responsive
			connectionResults.push_back(async(std::launch::async, &ClientStreamWindow::Connect, this, ref(pConnection), counter++));
		});

}

bool ClientStreamWindow::Connect(ConnectionPtr& pConnection, int counter) {
	// Repeatedly try to connect until maximum number of MAX_ATTEMPTS have been reached
// or the system has successfully connected
	for (int attempt = 0; attempt < 5 && !pConnection->connected; attempt++) {

		const Ushort portToConnectTo = Connection::SERVER_BASE_PORT + counter + attempt;

		_client->Connect(portToConnectTo, bind(&ClientStreamWindow::OnConnect, this, ref(pConnection)));
	}

	
	return pConnection->connected;
}

ClientStreamWindow::~ClientStreamWindow() {}

// What happens when connection to server is made
void ClientStreamWindow::OnConnect(ConnectionPtr& pConnection) {


	// Start receiving data on separate thread
	_clientThrs.push_back(thread([this, &pConnection] {

		_client->Receive(pConnection);
		pConnection->pIO_cont->run();
	}));
}

void ClientStreamWindow::CleanUp() {

	// Disconnect each connection
	if (_client->Connected()) { _client->Disconnect(); }

	for_each(_clientThrs.begin(), _clientThrs.end(), [](thread& _clientThr) {	if (_clientThr.joinable()) { _clientThr.join(); }});

	if (HAS_BEEN_CONNECTED) { packetReader->Clear(); }

	_client->connections.clear();

	BaseWindow::CleanUp();
}

void ClientStreamWindow::Resize(const Resolution& resolution) {
	_resolution = resolution;
	_imageData.reserve(CalculateBMPFileSize(_resolution));
}

void ClientStreamWindow::ImageBuilder() {

	const PixelData::iterator pixelData = _imageData.begin() + BMP_HEADER_SIZE;
	const Uint32 expectedSize = CalculateBMPFileSize(_resolution, 32, false);

	// Check  if there is a complete image
	while (!packetReader->Empty() && _client->Connected()) {

		const PacketPtr packet = packetReader->ReadMessage();
		const ImagePacketHeader& header = packet->Header();
		const PacketPayload& imageFragment = packet->Payload();

		int offset = header.Position() * MAX_PACKET_PAYLOAD_SIZE;

		std::copy(imageFragment.begin(), imageFragment.end(), pixelData + offset);
	}

}

void ClientStreamWindow::OnTick(wxTimerEvent& timerEvent) {
	PaintNow();
}

void ClientStreamWindow::PaintNow() {

	if (!_initialized || !_client->Connected()) { return; }

	ImageBuilder();

	wxClientDC dc(this);

	wxMemoryInputStream istream(_imageData.data(), _imageData.size());
	wxImage image(istream);
	wxBitmap bitmap(image);

	dc.DrawBitmap(bitmap, 0, 0);
}

void ClientStreamWindow::OnPaint(wxPaintEvent& evt) {

	// Do not paint until connected because no data is sent until then.
	if (!_client->Connected()) { evt.Skip(); }
	else { PaintNow(); }
}

void ClientStreamWindow::BackgroundTask(wxIdleEvent& evt) {

	if (!_initialized) {
		if (connectionResults.size() == VIDEO_THREADS) { 
			
			// Allows this window and the client to communicate across threads
			ConnectMessageables(*this, *_client);

			// Initialize image header
			const BmpFileHeader header = ConstructBMPHeader();
			std::copy(header.begin(), header.end(), _imageData.begin());
			_initialized = true; 
		}
		return;
	}

	else if (!_timer.IsRunning()) { 
		_popup->Dismiss();  
		_timer.Start(TARGET_FRAME_TIME); 
	}

	else if (!_client->Connected() && _clientThrs.size() > 0) {
		for_each(_clientThrs.begin(), _clientThrs.end(), [](thread& _clientThr) {	if (_clientThr.joinable()) { _clientThr.join(); }});
		_popup = make_unique<PopUp>(this, "Disconnected from server!", [this] { GoBack(); });
		_popup->Popup();
	}

}