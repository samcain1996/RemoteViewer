#pragma once

#include "RemoteViewerWindows.h"

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

	std::string message("Connecting to " + ip + ":" + std::to_string(remotePort));
	_popup = new PopUp(this, message);
	_popup->Popup();

	_client = new Client(ip);

	_client->Connect(remotePort, [this]() {

		_popup->Destroy();

		ConnectMessageables(*this, *_client);

		_ioThread = std::thread(&Client::Start, _client);
		_timer.Start(1000 / _targetFPS);

		const BmpFileHeader header = ConstructBMPHeader();

		std::copy(header.begin(), header.end(), _imageData.begin());

		_init = true;

		});

}

ClientStreamWindow::~ClientStreamWindow() {

	if (_client->Connected()) {
		_client->Disconnect();
		if (_ioThread.joinable()) { _ioThread.join(); }
	}

	delete _client;
}

void ClientStreamWindow::Resize(const Resolution& resolution) { _imageData = PixelData(CalculateBMPFileSize(resolution)); }

int ClientStreamWindow::ValidImagePacket(const ImagePacketHeader& header) {

	if (header.PacketType() != PacketTypes::Image) { return -1; }
	
	if (group != header.Group()) { group = header.Group(); return 0; }
	return header.Position();
}

void ClientStreamWindow::ImageBuilder() {

	static MessageReader<Packet*>*& packetReader = msgReader;
	static int offset = 0;

	const auto const pixeldata = &_imageData.data()[BMP_HEADER_SIZE];
	const Uint32 expectedSize = _imageData.size() - BMP_HEADER_SIZE;

	// Check  if there is a complete image
	while (!packetReader->Empty()) {

		const Packet* const p = packetReader->ReadMessage();
		const ImagePacketHeader& header = p->Header();
		const PacketPayload& imageFragment = p->Payload();

		PacketTypes type = p->Header().PacketType();
		if (type == PacketTypes::Image) {

			if (group != header.Group()) {
				group = header.Group();
				offset = 0;
			}
			else { offset = header.Position() * MAX_PACKET_PAYLOAD_SIZE; }
		}
		else if (imageFragment.size() + offset > expectedSize) { continue; }
		
		std::memcpy(&pixeldata[offset], imageFragment.data(), imageFragment.size());
		offset += imageFragment.size();

		delete p;
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

	else if (_ioThread.joinable()) {
		_ioThread.join();
	}

}