#include "RenderWindow.h"

RenderWindow::RenderWindow(const std::string& title, ScreenFragmentsRef fragments,
	MessageWriter<PacketGroup>& messageWriter, bool* killSignal) : 
	Window(title, killSignal), _bmpPiecesPtr(fragments), _msgReader(&messageWriter) {

	_renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);

	// Allocate memory for bitmap
	_bitmapSize = 54 + CalculateTheoreticalBMPSize(_width, _height);
	_bitmap = new Byte[_bitmapSize];

	_bmpDataStream = SDL_RWFromMem(_bitmap, _bitmapSize);

	Update();
}

void RenderWindow::Draw() {

	if (_msgReader.Empty()) { return; } // No image is ready, skip frame

	// Assemble image buffer
	const PacketGroup group = _msgReader.ReadMessage();
	AssembleImage(group);

	// Create image to render
	_bmpDataStream = SDL_RWFromMem(_bitmap, _bitmapSize);
	_surface = SDL_LoadBMP_RW(_bmpDataStream, SDL_TRUE);
	_texture = SDL_CreateTextureFromSurface(_renderer, _surface);

	// Render image
	SDL_RenderCopy(_renderer, _texture, NULL, NULL);
	SDL_RenderPresent(_renderer);

	SDL_UpdateWindowSurface(_window);

	// Free resources
	SDL_DestroyTexture(_texture);
	SDL_FreeSurface(_surface);
}

void RenderWindow::AssembleImage(const PacketGroup group) {

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

RenderWindow::~RenderWindow() {
	SDL_RWclose(_bmpDataStream);
	SDL_DestroyRenderer(_renderer);

	delete[] _bitmap;
}