#include "Windows/RemoteViewerWindows.h"
#include "GetInput.h"

/*----------------------Client Streaming Window-----------------------*/

wxBEGIN_EVENT_TABLE(ClientStreamWindow, BaseWindow)
EVT_PAINT(ClientStreamWindow::OnPaint)
EVT_IDLE(ClientStreamWindow::BackgroundTask)
EVT_KEY_UP(ClientStreamWindow::HandleInput)
EVT_TIMER(1234, ClientStreamWindow::OnTick)
wxEND_EVENT_TABLE()

ClientStreamWindow::ClientStreamWindow(const std::string& ip,
	const Ushort remotePort, const wxPoint& pos, const wxSize& size) :
	BaseWindow("Remote Viewer - Master", pos, size), _imageData(CalculateBMPFileSize()), _timer(this, 1234) {

	// Create client to receive data from other computer
	_client = make_shared<Client>(ip);

	_popup->SetText("Connecting to " + ip);
	_popup->Popup();

	std::for_each(_client->connections.begin(), _client->connections.end(), 
		[this, counter = 0] (ConnectionPtr& pConnection) mutable {

		// Attempt to connect to other computer on separate thread so
		// program is still responsive
		std::jthread([this, &pConnection, counter](const int MAX_ATTEMPTS = 5) {

			Ushort portToConnectTo = Connection::SERVER_BASE_PORT + counter;

			// Repeatedly try to connect until maximum number of MAX_ATTEMPTS have been reached
			// or the system has successfully connected
			for (int attempt = 0; attempt < MAX_ATTEMPTS && !pConnection->connected; attempt++, portToConnectTo++) {

				if (counter == 0) {
					_client->Connect(portToConnectTo, std::bind(&ClientStreamWindow::OnConnect, this));
				}
			}

			doneConnecting = true;

		}).detach();

		counter++;

	});
}

// What happens when connection to server is made
void ClientStreamWindow::OnConnect() {

	_popup->Dismiss();

	ConnectMessageables(*this, *_client);

	// Start receiving data on separate thread
	_clientThr = std::jthread(&Client::Start, _client, std::ref(_client->connections[0]));

	const BmpFileHeader header = ConstructBMPHeader();
	std::copy(header.begin(), header.end(), _imageData.begin());

	_init = true;
}

void ClientStreamWindow::CleanUp() {

	// Disconnect each connection
	std::for_each(_client->connections.begin(), _client->connections.end(),
		[this](ConnectionPtr& pConnection) {

			if (pConnection->connected) {
				_client->Disconnect(pConnection);
			}
		});

	if (_clientThr.joinable()) { _clientThr.join(); }
	if (HAS_BEEN_CONNECTED) { packetReader->Clear(); }
}

ClientStreamWindow::~ClientStreamWindow() {}

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
	if (!_client->Connected()) { evt.Skip(); }
	else { PaintNow(); }
}

void ClientStreamWindow::BackgroundTask(wxIdleEvent& evt) {

	if (!_init) {
		if (doneConnecting) { GoBack(); }
		return;
	}
	else if (!_timer.IsRunning()) { _timer.Start(1000 / _targetFPS); }

	else if (!_client->Connected() && _clientThr.joinable()) {
		_clientThr.join();
		_popup = make_unique<PopUp>(this, "Disconnected from server!", [this] { GoBack(); });
		_popup->Popup();
	}

}