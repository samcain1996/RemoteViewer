#pragma once
#include "Window.h"
#include "Messages.h"

class RenderWindow : Window {
	
	using GroupReadyReader = MessageReader<PacketGroup>;
	using ScreenFragmentsRef = PacketGroupPriorityQueueMap&;

private:

	SDL_Renderer* _renderer;	// Render remote screen to window
	SDL_RWops* _bmpDataStream;  // Current image to render

	ScreenFragmentsRef _bmpPiecesPtr;  // Fragments of images to render

	// Message reader receives messages when image is ready to render
	GroupReadyReader _msgReader;	   

	ByteArray _bitmap;  // Buffer to hold image to render

	Uint32 _bitmapSize; // Buffer size

	//std::mutex* _mutexPtr;

	void Draw() override;  // Draws _bitmap to the window
	void AssembleImage(const PacketGroup group);  // Assembles _bitmap from image fragments

public:
	RenderWindow() = delete;

	RenderWindow(const std::string& title, ScreenFragmentsRef fragments,
		MessageWriter<PacketGroup>& messageWriter, bool* killSignal);

	RenderWindow(const RenderWindow&) = delete;
	RenderWindow(RenderWindow&&) = delete;

	~RenderWindow();
};