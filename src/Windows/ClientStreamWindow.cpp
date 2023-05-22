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
}

bool ClientStreamWindow::Connect() {

	if (!_client->Connect()) { return false; }
	// Allows this window and the client to communicate across threads
	ConnectMessageables(*this, *_client);

	// Initialize image header
	const BmpFileHeader header = ConstructBMPHeader();
	std::copy(header.begin(), header.end(), _imageData.begin());

	return true;
}

ClientStreamWindow::~ClientStreamWindow() {}

void ClientStreamWindow::CleanUp() {

	// Disconnect each connection
	if (_client->Connected()) { _client->Disconnect(); }

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

		const int offset = header.Position() * MAX_PACKET_PAYLOAD_SIZE;

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
		
		_initialized = Connect();
	}

	else if (!_timer.IsRunning()) { 
		_popup->Dismiss();  
		_timer.Start(TARGET_FRAME_TIME); 
	}

	else if (!_client->Connected()) {
		_popup = make_unique<PopUp>(this, "Disconnected from server!", [this] { GoBack(); });
		_popup->Popup();

		
	}

}