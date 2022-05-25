#pragma once
#include "Client.h"
#include "Messages.h"

struct Window {
	SDL_Window* _window;		// Window to render to
	SDL_Surface* _surface;		// Pixel data to render to window
	SDL_Texture* _texture;		// Driver-specific, pixel data used to render
	SDL_Renderer* _renderer;	// Render remote screen to window
	SDL_RWops* _bmpDataStream;  // Current image to render
};

class Application {
	
	using GroupReadyReader = MessageReader<PacketGroup>;
	using ScreenFragmentsPtr = PacketGroupPriorityQueueMap*;

private:

	Window* _window = nullptr;

	std::thread _networkThr;
	NetAgent* _networker = nullptr;

	ScreenFragmentsPtr _bmpPiecesPtr = nullptr;  // Fragments of images to render

	// Message reader receives messages when image is ready to render
	GroupReadyReader _msgReader;	   

	ByteArray _bitmap = nullptr;  // Buffer to hold image to render

	Uint32 _bitmapSize; // Buffer sizE

	void Draw();  // Draws _bitmap to the window
	void Update();
	void AssembleImage(const PacketGroup group);  // Assembles _bitmap from image fragments

public:
	Application(const Ushort port);

	~Application();
};