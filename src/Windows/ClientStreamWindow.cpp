#include "Windows/RemoteViewerWindows.h"

/*----------------------Client Streaming Window-----------------------*/

wxBEGIN_EVENT_TABLE(ClientStreamWindow, BaseWindow)
EVT_PAINT(ClientStreamWindow::OnPaint)
EVT_IDLE(ClientStreamWindow::BackgroundTask)
EVT_KEY_UP(ClientStreamWindow::HandleInput)
EVT_TIMER(1234, ClientStreamWindow::OnTick)
wxEND_EVENT_TABLE()

ClientStreamWindow::ClientStreamWindow(const std::string& ip, const Ushort localPort,
	const Ushort remotePort, const wxPoint& pos, const wxSize& size) :
	BaseWindow("Remote Viewer - Master", pos, size), _imageData(CalculateBMPFileSize()), _timer(this, 1234) {

	Logger::newStream("Receive.log");

	std::string message("Connecting to " + ip + ":" + std::to_string(remotePort));
	_popup = std::make_unique<PopUp>(this, message);
	_popup->Popup();

	_client = std::make_shared<Client>(ip);

	_client->Connect(remotePort, [this]() {

		_popup->Dismiss();

		ConnectMessageables(*this, *_client);

		_clientThr = std::thread(&Client::Start, _client);
		_timer.Start(1000 / _targetFPS);

		const BmpFileHeader header = ConstructBMPHeader();
		std::copy(header.begin(), header.end(), _imageData.begin());

		_init = true;

		});

}

ClientStreamWindow::~ClientStreamWindow() {

	if (_client->Connected()) {
		_client->Disconnect();
		if (_clientThr.joinable()) { _clientThr.join(); }
	}
	packetReader->Clear();
}

void ClientStreamWindow::Resize(const Resolution& resolution) { 
	_resolution = resolution;
	_imageData.reserve(CalculateBMPFileSize(_resolution)); 
}

void ClientStreamWindow::ImageBuilder() {

	int offset = 0;
	static Loggette log = Logger::getLog("Receive.log").value();

	const PixelData::iterator pixelData = _imageData.begin() + BMP_HEADER_SIZE;
	const Uint32 expectedSize = CalculateBMPFileSize(_resolution, 32, false);

	// Check  if there is a complete image
	while (!packetReader->Empty() && _client->Connected()) {

		std::shared_ptr<Packet> p = packetReader->ReadMessage();
		const ImagePacketHeader& header = p->Header();
		const PacketPayload& imageFragment = p->Payload();

		const auto& size = header.Size() - PACKET_HEADER_SIZE;

		if (header.Type() == PacketType::Image) {

			// New image
			if (group != header.Group()) {
				group = header.Group();
				offset = 0;
			}
			offset = header.Position() * MAX_PACKET_PAYLOAD_SIZE;
			
		}
		if (!Packet::VerifyPacket(*p) || size + offset > expectedSize) {
			offset = 0;
			std::string lineToLog = "Header Size: " + std::to_string(header.Size()) + "\n";
			lineToLog += "Actual Size: " + std::to_string(imageFragment.size() + PACKET_HEADER_SIZE) + "\n";
			lineToLog += "Dump:\n" + (std::string)header + "\n";

			log.WriteLine(lineToLog);
			continue; 
		}
		std::copy(imageFragment.begin(), imageFragment.begin() + size, pixelData + offset);
		
	}

}

void ClientStreamWindow::OnTick(wxTimerEvent& timerEvent) {
	_render = true;
	_timer.Start(1000 / _targetFPS);
	wxWakeUpIdle();
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

	if (!_init) { return; }

	if (_client->Connected()) {
		
		if (_render) {
			PaintNow();
			_render = false;
		}

	}

	else if (_clientThr.joinable()) {
		_clientThr.join();
		_popup = std::make_unique<PopUp>(this, "Disconnected from server!", [this]() { GoBack(); });
		_popup->Popup();
		
	}

}