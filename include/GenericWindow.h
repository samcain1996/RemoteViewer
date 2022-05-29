#pragma once


#include "Types.h"
#include "Packet.h"

class GenericWindow {

protected:

	GenericWindow() = delete;

	//GenericWindow(const std::string& title);
	GenericWindow(const std::string& title, std::atomic<bool>* killSignal);

	GenericWindow(const GenericWindow&) = delete;
	GenericWindow(GenericWindow&&) = delete;

	virtual ~GenericWindow();

	GenericWindow& operator=(const GenericWindow&) = delete;
	GenericWindow& operator=(GenericWindow&&) = delete;

	SDL_Window* _window;	// GenericWindow to render to
	SDL_Surface* _surface;	// Pixel data to render to window
	SDL_Texture* _texture;	// Driver-specific, pixel data used to render

	SDL_DisplayMode _displayData;  // Data about connected monitors

	SDL_Event _event;  // Used to get input from user

	int _width, _height;  // Width and height of window
	int _posX, _posY;	  // X and Y position of window

	int _mouseX, _mouseY;  // Mouse position

	Uint32 _targetFPS;	  // FPS to target

	std::atomic<bool>* _keepAlive;

	virtual void Draw() = 0;  // Draw to window
	virtual void Update();    // Update window

	void CapFPS(const Uint32 prevTicks);  // Limit FPS
};