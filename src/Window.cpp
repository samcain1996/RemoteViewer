#include "Window.h"

#if defined(_WIN32)
#pragma warning(suppress: 26495)	// Warning for uninitialized SDL_Event can be silenced, it is init before use.
#endif
GenericWindow::GenericWindow(const std::string& title, const EventHandler& eh) : _eventHandler(eh),
	_elementManager(this) {
	
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

	_targetFPS = 60;
}

void GenericWindow::Update() {

	Uint32 ticks;  

	bool keepAlive = true;
	while (keepAlive) {
		
		// Get events
		while (SDL_PollEvent(&_event)) {

			keepAlive = _eventHandler(_event, _elementManager);
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
}

GenericWindow::~GenericWindow() {

	TTF_CloseFont(_font);
	TTF_Quit();
	SDL_DestroyWindow(_window);
}

InitWindow::InitWindow(const std::string& title, const EventHandler& wev) : GenericWindow(title, wev) {

	_font = TTF_OpenFont("tahoma.ttf", 24);

	InitButtons();


}

void InitWindow::InitButtons() {
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

	_clientButton = new Button(_font, _fontColor, "Client", clientRect);
	_serverButton = new Button(_font, _fontColor, "Server", serverRect);

	_elementManager.Add(_clientButton);
	_elementManager.Add(_serverButton);
}

const bool InitWindow::Draw() {

	_elementManager.RenderElements();

	SDL_RenderPresent(_renderer);

	return true;
}

InitWindow::~InitWindow() {
}

RenderWindow::RenderWindow(const std::string& title, const EventHandler& ev) :
	GenericWindow(title, ev) {

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

// Element Manager

ElementManager::ElementManager(GenericWindow* window) : _window(window) {
}

ElementManager::~ElementManager() {

}
void ElementManager::Add(WindowElement* element) {
	elements.push_back(element);
}

void ElementManager::RenderElements() {
	for (WindowElement* element : elements) {
		element->RenderElement(_window->_surface, _window->_texture, _window->_renderer);
	}
}

const WindowElement& ElementManager::GetElementByName(const std::string& elementName) const {
	for (int i = 0; i < elements.size(); i++) {
		if (elements[i]->Name() == elementName) { return *elements.at(i); }
	}
}

const WindowElement& ElementManager::GetElementById(const Uint32 id) const {
	if (_idCounter >= id) { return *elements[id]; }
}