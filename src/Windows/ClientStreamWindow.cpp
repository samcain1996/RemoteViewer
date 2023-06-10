#include "Windows/RemoteViewerWindows.h"

/*----------------------Client Streaming Window-----------------------*/

wxBEGIN_EVENT_TABLE(ClientStreamWindow, BaseWindow)
EVT_PAINT(ClientStreamWindow::OnPaint)
EVT_IDLE(ClientStreamWindow::BackgroundTask)
EVT_KEY_UP(ClientStreamWindow::HandleInput)
EVT_LEFT_DOWN(ClientStreamWindow::OnClick)
wxEND_EVENT_TABLE()

ClientStreamWindow::ClientStreamWindow(const string& ip, const wxPoint& pos, const wxSize& size) :
	BaseWindow("Remote Viewer - Master", pos, size), _imageData(CalculateBMPFileSize()) {

	// Create client to receive data from other computer
	_client = make_unique<Client>(ip);

	_popup->SetText("Connecting to " + ip);
	_popup->Popup();

	thread(&ClientStreamWindow::Connect, this).detach();
	
	pTimer = make_unique<Timer>( (1000 / Configs::TARGET_FPS), bind(&ClientStreamWindow::PaintNow, this));
}

void ClientStreamWindow::OnClick(wxMouseEvent& mouseEvent) {
	
	/*int windowX = 0;
	int windowY = 0;
	POINT cursorPos;
	GetPosition(&windowX, &windowY);
	GetCursorPos(&cursorPos);
	auto clientSize = GetClientRect();
	auto topBorder = 45;
	auto leftMargin = 10;


	if (cursorPos.x > windowX + leftMargin && cursorPos.x < windowX + leftMargin + clientSize.width) {
		if (cursorPos.y > windowY + topBorder && cursorPos.y < windowY + topBorder + clientSize.height) {

			double relX = ((double)cursorPos.x - ( windowX + leftMargin )) / clientSize.width;
			double relY = ((double)cursorPos.y - ( windowY + topBorder + 5)) / clientSize.height;

			int mappedX = (double)relX * ScreenCapture::NativeResolution().width;
			int mappedY = (double)relY * ScreenCapture::NativeResolution().height;;

			static MousePacketHeader m(1, PACKET_HEADER_SIZE + 8);
			static PacketPayload payload(8);
			EncodeAsByte(&payload.data()[0], mappedX);
			EncodeAsByte(&payload.data()[4], mappedY);
			static PacketList packets;
			packets.emplace_back(m, payload);
			_client->Send(packets, _client->dataCon);
			_client->dataCon->pIO_cont->run();
			_client->dataCon->pIO_cont->restart();
		}
	}*/


}

bool ClientStreamWindow::Connect() {

	if (!_client->TryConnect()) { return false; }
	// Allows this window and the client to communicate across threads
	ConnectMessageables(*this, *_client);

	// Initialize image header
	const BmpFileHeader header = ConstructBMPHeader();
	std::copy(header.begin(), header.end(), _imageData.begin());


	return true;
}

ClientStreamWindow::~ClientStreamWindow() {}

void ClientStreamWindow::CleanUp() {

	pTimer->stop();

	// Disconnect each connection
	if (_client->Connected()) { _client->Disconnect(); }

	BaseWindow::CleanUp();
}

void ClientStreamWindow::Resize(const Resolution& resolution) {
	_resolution = resolution;
	_imageData.reserve(CalculateBMPFileSize(_resolution));

	// Initialize image header
	const BmpFileHeader header = ConstructBMPHeader(_resolution);
	std::copy(header.begin(), header.end(), _imageData.begin());
}

void ClientStreamWindow::ImageBuilder() {

	const PixelData::iterator pixelData = _imageData.begin() + BMP_HEADER_SIZE;

	// Check  if there is a complete image
	while (!packetReader->Empty() && _client->Connected()) {

		const PacketPtr packet = packetReader->ReadMessage();
		const ImagePacketHeader& header = packet->Header();
		const PacketPayload& imageFragment = packet->Payload();

		const int offset = header.Position() * MAX_PACKET_PAYLOAD_SIZE;

		std::copy(imageFragment.begin(), imageFragment.end(), pixelData + offset);
	}

}

void ClientStreamWindow::PaintNow() {
	//long long delta = pTimer->timeSinceLastInterval();
	//Logger::LogLine("global.log", std::to_string(1000 / delta));
	if (!_initialized || !_client->Connected()) { return; }

	ImageBuilder();

	wxClientDC dc(this);

	wxMemoryInputStream istream(_imageData.data(), _imageData.size());
	wxImage image(istream);
	wxBitmap bitmap(image);

	dc.DrawBitmap(bitmap, 0, 0);

}

void ClientStreamWindow::BackgroundTask(wxIdleEvent& evt) {

	if (_client->Connected()) { _initialized = true; }

	if (!pTimer->isRunning() && _initialized) {
		_popup->Dismiss();  
		pTimer->start();
	}

	else if (!_client->Connected() && pTimer->isRunning()) {
		 pTimer->stop();
		_popup = make_unique<PopUp>(this, "Disconnected from server!", [this] { GoBack(); });
		_popup->Popup();
		_initialized = false;
	}

}