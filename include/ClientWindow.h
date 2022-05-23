#pragma once
#include "Window.h"
#include "Messages.h"

class ClientWindow : Window {
private:

	SDL_Renderer* _renderer;
	SDL_RWops* _bmpDataStream;

	PacketGroupPriorityQueueMap* _bmpPiecesPtr;

	ApplicationMessageReader _messages;

	ByteArray _bitmap = nullptr;

	uint32 _bitmapSize = 0;

	void Draw();
	void AssembleMessage(const PacketGroup&);

public:
	ClientWindow() = delete;
	ClientWindow(const std::string& title, PacketGroupPriorityQueueMap* const messages,
		ApplicationMessageWriter& messageWriter);

	ClientWindow(const ClientWindow&) = delete;
	ClientWindow(ClientWindow&&) = delete;

	void Run();
	//void ParseHeader(const ByteArray header, const uint32 headerSize);
};