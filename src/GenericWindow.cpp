#include "GenericWindow.h"

#if defined(_WIN32)
#pragma warning(suppress: 26495)	// Warning for uninitialized SDL_Event can be silenced, it is init before use.
#endif
GenericWindow::GenericWindow(const std::string& title, std::atomic<bool>& killSignal) : _keepAlive(killSignal) {

	SDL_GetDesktopDisplayMode(0, &_displayData);

	// Set width and height to be 75% monitor values
	_width = _displayData.w * 0.75f;
	_height = _displayData.h * 0.75f;

	// Center window
	_posX = (_displayData.w - _width) / 2;
	_posY = (_displayData.h - _height) / 2;

	// Create window
	_window = SDL_CreateWindow(title.c_str(), _posX, _posY, _width, _height, NULL);

	_surface = nullptr;
	_texture = nullptr;

	_targetFPS = 60;
}

GenericWindow::~GenericWindow() {
	
	SDL_DestroyWindow(_window);
}

void GenericWindow::Update() {

	Uint32 ticks;  

	while (!_keepAlive) {
		
		// Get events
		while (SDL_PollEvent(&_event)) {

			if (_event.type == SDL_MOUSEBUTTONDOWN) {
				eventWriter->WriteMessage(_event);
				eventWriter2->WriteMessage(_event);
			}

		}

		ticks = SDL_GetTicks();

		if (Draw()) {
			SDL_FreeSurface(_surface);
		}  // Draw content to window

		CapFPS(ticks);

	}

}

void GenericWindow::CapFPS(const Uint32 prevTicks) {
	if ((1000 / _targetFPS) > SDL_GetTicks() - prevTicks) { 
		SDL_Delay(1000 / _targetFPS - (SDL_GetTicks() - prevTicks)); 
	}
}
