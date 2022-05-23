#include "ClientWindow.h"

ClientWindow::ClientWindow(const std::string& title, PacketGroupPriorityQueueMap* const messages,
	MessageWriter<PacketGroup>& messageWriter) : Window(title), _bmpPiecesPtr(*messages),
	_msgReader(&messageWriter), _bitmap(nullptr), _bitmapSize(0) {
	_renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);

	_bitmapSize = 54 + CalculateTheoreticalBMPSize(_width, _height);
	_bitmap = new Byte[_bitmapSize];

	_bmpDataStream = SDL_RWFromMem(_bitmap, _bitmapSize);
}

void ClientWindow::Draw() {

	while (_msgReader.Empty()) {} // No image is ready, skip frame

	// Assemble image buffer
	const PacketGroup group = _msgReader.ReadMessage();
	AssembleImage(group);

	// Create image to render
	_surface = SDL_LoadBMP_RW(_bmpDataStream, SDL_TRUE);
	_texture = SDL_CreateTextureFromSurface(_renderer, _surface);

	// Render image
	SDL_RenderCopy(_renderer, _texture, NULL, NULL);
	SDL_RenderPresent(_renderer);

	// Free resources
	SDL_DestroyTexture(_texture);
	SDL_FreeSurface(_surface);
}

void ClientWindow::Run() { Update(); }

void ClientWindow::AssembleImage(const PacketGroup group) {

	PacketPriorityQueue& queue = _bmpPiecesPtr[group];

	// Build image from packets
	for (size_t packetNo = 0; !queue.empty(); packetNo++) {

		// Offset in full image to location that this fragment belongs
		const Uint32 offset = packetNo * MAX_PACKET_PAYLOAD_SIZE;

		// Retrieve payload from top packet_bitmapData
		const Packet& packet = queue.top();

		// Append payload to vector
		std::memcpy(&_bitmap[offset], packet.Payload().data(), packet.Header().size - PACKET_HEADER_SIZE);

		queue.pop();  // Remove packet from queue
	}

	_bmpPiecesPtr.erase(group); // Erase queue
}

ClientWindow::~ClientWindow() {
	SDL_DestroyRenderer(_renderer);

	delete[] _bitmap;
}