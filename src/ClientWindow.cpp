#include "ClientWindow.h"

ClientWindow::ClientWindow(const std::string& title, PacketGroupPriorityQueueMap* const messages,
	MessageWriter< std::pair<ByteArray, uint32> >& messageWriter) : Window(title), _bmpPiecesPtr(*messages),
	_messages(&messageWriter), _bitmap(nullptr), _bitmapSize(0) {
	_renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);

	_bmpDataStream = nullptr;
}

void ClientWindow::Draw() {

	while (_messages.Empty()) {}

	PacketGroup group = decode256(_messages.ReadMessage().first);

	AssembleMessage(group);

	_bmpDataStream = SDL_RWFromMem(_bitmap,_bitmapSize);

	_surface = SDL_LoadBMP_RW(_bmpDataStream, SDL_TRUE);

	_texture = SDL_CreateTextureFromSurface(_renderer, _surface);

	SDL_RenderCopy(_renderer, _texture, NULL, NULL);
	SDL_RenderPresent(_renderer);

	SDL_DestroyTexture(_texture);
	SDL_FreeSurface(_surface);

}

void ClientWindow::Run() { Update(); }

void ClientWindow::AssembleMessage(const PacketGroup& group) {

	PacketPrioQueue& queue = _bmpPiecesPtr[group];

	size_t size = 0;
	if (_bitmap == nullptr) {
		_bitmapSize = queue.size() * MAX_PACKET_PAYLOAD_SIZE;
		_bitmap = new Byte[_bitmapSize];
	}

	for (size_t packetNo = 0; !queue.empty(); packetNo++) {

		// Retrieve payload from top packet_bitmapData
		const Packet& packet = queue.top();

		// Append payload to vector
		size += packet.Header().size - PACKET_HEADER_SIZE;
		std::memcpy(&_bitmap[(packet.Header().sequence - 1) * MAX_PACKET_PAYLOAD_SIZE], packet.Payload().data(), packet.Header().size - PACKET_HEADER_SIZE);

		queue.pop();  // Remove packet from queue
	}

	_bmpPiecesPtr.erase(group);
}

//void ClientWindow::ParseHeader(const ByteArray header, const uint32 headerSize) {
//	//_bmpHeaderSize = headerSize;
//	//_shid = new Byte[headerSize];
//	//std::memcpy(_shid, header, headerSize);
//
//
//}
