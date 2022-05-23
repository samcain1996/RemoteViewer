#pragma once


#include "Types.h"
#include "Packet.h"

class Window {

protected:
	SDL_Window* _window;
	SDL_Surface* _surface;
	SDL_Texture* _texture;

	SDL_DisplayMode _displayData;

	SDL_Event _event;

	int _width, _height;
	int _posX, _posY;

	uint32 _targetFPS;

	virtual void Draw();
	virtual void Update();

	void CapFPS(const uint32 prevTicks);

	Window() = delete;
	
	Window(const std::string& title);

	Window(const Window&) = delete;
	Window(Window&&) = delete;

	~Window();
};