#pragma once
#include "GenericWindow.h"

class RenderWindow : public GenericWindow {

private:
	SDL_RWops* _bmpDataStream;  // Current image to render

	ByteArray _bitmap;  // Buffer to hold image to render

	Uint32 _bitmapSize; // Buffer size

	const bool Draw() override;  // Draws _bitmap to the window
	void AssembleImage(PacketPriorityQueue* const queue);  // Assembles _bitmap from image fragments

public:
	RenderWindow() = delete;

	RenderWindow(const std::string& title, const EventHandler& ev);

	RenderWindow(const RenderWindow&) = delete;
	RenderWindow(RenderWindow&&) = delete;

	~RenderWindow();

	void Update() override {};

	RenderWindow& operator=(const RenderWindow&) = delete;
	RenderWindow& operator=(RenderWindow&&) = delete;

	MsgReaderPtr<PacketPriorityQueue*> completeGroups = nullptr;
};