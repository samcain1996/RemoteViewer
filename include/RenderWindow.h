#pragma once
#include "GenericWindow.h"

class RenderWindow : public GenericWindow {
private:

	SDL_Renderer* _renderer;	// Render remote screen to window
	SDL_RWops* _bmpDataStream;  // Current image to render

	ByteArray _bitmap;  // Buffer to hold image to render

	Uint32 _bitmapSize; // Buffer size


	bool Draw() override;  // Draws _bitmap to the window
	void AssembleImage(PacketPriorityQueue* const queue);  // Assembles _bitmap from image fragments

public:
	RenderWindow() = delete;

	RenderWindow(const std::string& title, std::atomic<bool>& killSignal);

	RenderWindow(const RenderWindow&) = delete;
	RenderWindow(RenderWindow&&) = delete;

	~RenderWindow();

	RenderWindow& operator=(const RenderWindow&) = delete;
	RenderWindow& operator=(RenderWindow&&) = delete;

	MessageReader<PacketPriorityQueue*>* completeGroups = nullptr;


	friend void Update(RenderWindow& window);
};