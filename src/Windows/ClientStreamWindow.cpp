#include "Windows/RemoteViewerWindows.h"

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

	std::string message("Connecting to " + ip);
	_popup = std::make_unique<PopUp>(this, message);
	_popup->Popup();

	_client = std::make_shared<Client>(ip);
	Ushort port = remotePort == 0 ? _client->portToTry : remotePort;
	std::thread([this, port]() mutable {
			while (!_client->Connected()) {
				_client->Connect(port++, std::bind(&ClientStreamWindow::OnConnect, this));
			}
		}).detach();

}

void ClientStreamWindow::OnConnect() {
	_popup->Dismiss();

	ConnectMessageables(*this, *_client);

	_clientThr = std::thread(&Client::Start, _client);

	const BmpFileHeader header = ConstructBMPHeader();
	std::copy(header.begin(), header.end(), _imageData.begin());

	_init = true;
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

	const PixelData::iterator pixelData = _imageData.begin() + BMP_HEADER_SIZE;
	const Uint32 expectedSize = CalculateBMPFileSize(_resolution, 32, false);

	// Check  if there is a complete image
	while (!packetReader->Empty() && _client->Connected()) {

		const PacketPtr packet = packetReader->ReadMessage();
		const ImagePacketHeader& header = packet->Header();
		const PacketPayload& imageFragment = packet->Payload();

		const auto size = header.Size() - PACKET_HEADER_SIZE;

		if (header.Type() == PacketType::Image) {

			// New image
			//if (group != header.Group()) {
			//	group = header.Group();
			//	offset = 0;
			//}
			offset = header.Position() * MAX_PACKET_PAYLOAD_SIZE;
			
		}
		if (!Packet::VerifyPacket(*packet) || size + offset > expectedSize) {
			//Logger::LogLine("global.log", (char*)packet->RawData().data());
			continue; 
		}
		std::copy(imageFragment.begin(), imageFragment.begin() + size, pixelData + offset);
		if (offset + size == expectedSize) { return; }
	}

}

void ClientStreamWindow::OnTick(wxTimerEvent& timerEvent) {
	_render = true;
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
	else if (!_timer.IsRunning()) { _timer.Start(1000 / _targetFPS); }

	else if (_client->Connected()) {
		
		if (_render) {
			PaintNow();
			_render = false;
		}

	}

	else if (_clientThr.joinable()) {
		_clientThr.join();
		_popup = std::make_unique<PopUp>(this, "Disconnected from server!", [this] { GoBack(); });
		_popup->Popup();
		
	}

}