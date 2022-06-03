#include "Window.h"

#if defined(_WIN32)
#pragma warning(suppress: 26495)	// Warning for uninitialized SDL_Event can be silenced, it is init before use.
#endif
GenericWindow::GenericWindow(const std::string& title, const EventHandler& eventHandler) : _eventHandler(eventHandler) {
	
	TTF_Init();

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

	_renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);

	_targetFPS = 10;
}

void GenericWindow::Update() {

	Uint32 ticks;  

	bool keepAlive = true;
	while (keepAlive) {
		
		// Get events
		while (SDL_PollEvent(&_event)) {

			if (_event.type == SDL_QUIT) {
				keepAlive = false;
				break;
			}

			keepAlive = _eventHandler(_event, ElementView(_elements));
		}

		ticks = SDL_GetTicks();

		Draw();

		CapFPS(ticks);

	}

}

void GenericWindow::CapFPS(const Uint32 prevTicks) {
	if ((1000 / _targetFPS) > SDL_GetTicks() - prevTicks) { 
		SDL_Delay(1000 / _targetFPS - (SDL_GetTicks() - prevTicks)); 
	}

	std::cout << "FPS: " << std::to_string((SDL_GetTicks() - prevTicks) / 1000.0) << "\n";
}

GenericWindow::~GenericWindow() {

	//for (int i = _elements.size() - 1; i >= 0; i--) {
	//	delete _elements[i];
	//	_elements.pop_back();
	//}

	TTF_CloseFont(_font);
	TTF_Quit();

	SDL_DestroyWindow(_window);
}

InitWindow::InitWindow(const std::string& title, const EventHandler& eventHandler) : GenericWindow(title, eventHandler) {

	_font = TTF_OpenFont("tahoma.ttf", 54);

	SDL_Color fontColor{ 255, 0, 255 };
	SDL_Color backgroundColor{ 0, 255, 0 };

	SDL_Rect clientRect;

	// Position buttons
	clientRect.w = 300;
	clientRect.h = 100;

	clientRect.x = clientRect.w * 2;
	clientRect.y = (_height - clientRect.h) / 2;

	SDL_Rect serverRect;

	serverRect.w = 300;
	serverRect.h = 100;

	serverRect.x = (_width - serverRect.w) - serverRect.w;
	serverRect.y = (_height - serverRect.h) / 2;

	_clientButton = new Button(_font, fontColor, backgroundColor, "Client", clientRect);
	_serverButton = new Button(_font, fontColor, backgroundColor, "Server", serverRect);

	SDL_Rect tboxRect;
	tboxRect.w = 300;
	tboxRect.h = 100;
	tboxRect.x = serverRect.x;
	tboxRect.y = serverRect.y - 300;
	_tBox = new TextBox(_font, "TEST", tboxRect);

	_elements.push_back(*_clientButton);
	_elements.push_back(*_serverButton);
	_elements.push_back(*_tBox);

}

const bool InitWindow::Draw() {

	SDL_SetRenderDrawColor(_renderer, 0, 0, 0, 255);

	_surface = SDL_CreateRGBSurface(0, _width, _height, 32, 0, 0, 0, 0);
	_texture = SDL_CreateTextureFromSurface(_renderer, _surface);

	SDL_RenderCopy(_renderer, _texture, NULL, NULL);

	SDL_FreeSurface(_surface);
	SDL_DestroyTexture(_texture);

	for (WindowElement& windowElement : _elements) {
		windowElement.RenderElement(_renderer);
	}

	SDL_RenderPresent(_renderer);

	return true;
}

void InitWindow::Update() {
	Uint32 ticks = 0;
	bool keepAlive = true;

	while (keepAlive) {

		while (SDL_PollEvent(&_event)) {
			keepAlive = _eventHandler(_event, ElementView(_elements));
		}
		ticks = SDL_GetTicks();

		Draw();

		CapFPS(ticks);
	}
}

InitWindow::~InitWindow() {}

RenderWindow::RenderWindow(const std::string& title, const EventHandler& eventHandler) :
	GenericWindow(title, eventHandler) {

	// Allocate memory for bitmap
	_bitmapSize = 54 + CalculateTheoreticalBMPSize(_width, _height);
	_bitmap = new Byte[_bitmapSize];

	_bmpDataStream = SDL_RWFromMem(_bitmap, _bitmapSize);

}

const bool RenderWindow::Draw() {

	if (completeGroups->Empty()) { return false; } // No image is ready, skip frame

	// Assemble image buffer
	AssembleImage(completeGroups->ReadMessage());

	// Create image to render
	_bmpDataStream = SDL_RWFromMem(_bitmap, _bitmapSize);
	_surface = SDL_LoadBMP_RW(_bmpDataStream, SDL_TRUE);
	_texture = SDL_CreateTextureFromSurface(_renderer, _surface);

	// Render image
	SDL_RenderCopy(_renderer, _texture, NULL, NULL);
	SDL_RenderPresent(_renderer);

	// Free resources
	SDL_FreeSurface(_surface);
	SDL_DestroyTexture(_texture);

	return true;
}

void RenderWindow::AssembleImage(PacketPriorityQueue* const queue) {

	// Build image from packets
	for (size_t packetNo = 0; !queue->empty(); packetNo++) {

		// Offset in full image to location that this fragment belongs
		const Uint32 offset = packetNo * MAX_PACKET_PAYLOAD_SIZE;

		// Retrieve payload from top packet_bitmapData
		const Packet& packet = queue->top();

		// Append payload to vector
		std::memcpy(&_bitmap[offset], packet.Payload().data(), packet.Header().size - PACKET_HEADER_SIZE);

		queue->pop();  // Remove packet from queue
	}

	delete queue;
}

RenderWindow::~RenderWindow() {

	SDL_DestroyRenderer(_renderer);

	delete completeGroups;
	delete[] _bitmap;
}


WindowElement& ElementView::GetElementByName(const std::string& elementName) const {
	for (int i = 0; i < elements.size(); i++) {
		if (elements[i].get().Name() == elementName) { return elements[i]; }
	}
}

WindowElement& ElementView::GetElementById(const Uint32 id) const {
	if (elements.size() > id) { return elements[id]; }
}