#pragma once


#include "Types.h"
#include "Packet.h"

class Window {

protected:

	Window() = delete;

	Window(const std::string& title);
	Window(const std::string& title, bool* killSignal);

	Window(const Window&) = delete;
	Window(Window&&) = delete;

	~Window();

	SDL_Window* _window;	// Window to render to
	SDL_Surface* _surface;	// Pixel data to render to window
	SDL_Texture* _texture;	// Driver-specific, pixel data used to render

	SDL_DisplayMode _displayData;  // Data about connected monitors

	SDL_Event _event;  // Used to get input from user

	int _width, _height;  // Width and height of window
	int _posX, _posY;	  // X and Y position of window

	Uint32 _targetFPS;	  // FPS to target

	bool* const _keepAlive;

	virtual void Draw() = 0;  // Draw to window
	virtual void Update();    // Update window

	void CapFPS(const Uint32 prevTicks);  // Limit FPS
};