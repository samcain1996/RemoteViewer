#pragma once
#include "Window.h"
#include "Messages.h"

class ClientWindow : Window {
	
	using GroupReadyReader = MessageReader<std::pair<ByteArray, uint32> >;

	using ScreenFragmentsRef = PacketGroupPriorityQueueMap&;

private:

	SDL_Renderer* _renderer;
	SDL_RWops* _bmpDataStream;

	ScreenFragmentsRef _bmpPiecesPtr;

	GroupReadyReader _messages;

	ByteArray _bitmap;

	uint32 _bitmapSize;

	void Draw();
	void AssembleMessage(const PacketGroup&);

public:
	ClientWindow() = delete;
	ClientWindow(const std::string& title, PacketGroupPriorityQueueMap* const messages,
		MessageWriter<std::pair<ByteArray, uint32> >& messageWriter);

	ClientWindow(const ClientWindow&) = delete;
	ClientWindow(ClientWindow&&) = delete;

	void Run();
};