#include "Window.h"

Window::Window(const std::string& title) {

	SDL_GetDesktopDisplayMode(0, &_displayData);

	_width = _displayData.w * 0.75f;
	_height = _displayData.h * 0.75f;

	_posX = (_displayData.w - _width) / 2;
	_posY = (_displayData.h - _height) / 2;

	_window = SDL_CreateWindow(title.c_str(), _posX, _posY, _width, _height, NULL);

	_surface = nullptr;
	_texture = nullptr;

	_targetFPS = 60;
}

Window::~Window() {
	SDL_DestroyWindow(_window);
	SDL_Quit();
}

void Window::Draw() {}

void Window::Update() {

	uint32 ticks;
	bool exit = false;

	while (!exit) {
		
		ticks = SDL_GetTicks();

		Draw();
		
		while (SDL_PollEvent(&_event)) {

			if (_event.type == SDL_QUIT) {
				exit = true;
				break;
			}

		}

		CapFPS(ticks);

	}

}

void Window::CapFPS(const uint32 prevTicks) {
	if ((1000 / _targetFPS) > SDL_GetTicks() - prevTicks) { 
		SDL_Delay(1000 / _targetFPS - (SDL_GetTicks() - prevTicks)); 
	}
}