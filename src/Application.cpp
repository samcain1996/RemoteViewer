#include "Application.h"

Application::Application(const Ushort port) {

	_networker = new Client(port, "192.168.50.160", &_msgReader);

	_networkThr = std::thread([&]() {
		// Assume client for now
		dynamic_cast<Client*>(_networker)->Connect("10008");
		});


	_window = new Window;

}

void Application::Draw() {

	if (_msgReader.Empty()) { return; } // No image is ready, skip frame

	// Assemble image buffer
	const PacketGroup group = _msgReader.ReadMessage();
	AssembleImage(group);

	// Create image to render
	_bmpDataStream = SDL_RWFromConstMem(_bitmap, _bitmapSize);
	_surface = SDL_LoadBMP_RW(_bmpDataStream, SDL_TRUE);
	_texture = SDL_CreateTextureFromSurface(_renderer, _surface);

	// Render image
	SDL_RenderCopy(_renderer, _texture, NULL, NULL);
	SDL_RenderPresent(_renderer);

	// Free resources
	SDL_DestroyTexture(_texture);
	SDL_FreeSurface(_surface);
}

void Application::AssembleImage(const PacketGroup group) {

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

Application::~Application() {

	_networkThr.join();
	delete dynamic_cast<Client*>(_networker);

	SDL_RWclose(_bmpDataStream);
	SDL_DestroyRenderer(_renderer);

	delete[] _bitmap;
}