#include "Window.h"

// Window Element

WindowElement::WindowElement(const std::string& name, const SDL_Rect& rect) :
	_name(name), _bounds(rect) {};

WindowElement::WindowElement() : WindowElement("ERROR", SDL_Rect()) {}

WindowElement& WindowElement::operator=(WindowElement&& other) noexcept {
	_name = std::move(other._name);
	_bounds = std::move(other._bounds);

	return *this;
}

const std::string& WindowElement::Name() const { return _name; }
const SDL_Rect& WindowElement::Bounds() const { return _bounds; }


// Element List


const std::pair<bool, Uint32> ElementList::ElementNameExists(const std::string& name) const {
	for (const auto& [id, elem] : elements) {
		if (elem._name == name) { return std::make_pair(true, id); }
	}
	return std::make_pair(false, 0);
}

void ElementList::Add(std::string& name, const SDL_Rect& rect) {
	if (ElementNameExists(name).first) {
		name = std::to_string(_idCounter);
	}

	elements[_idCounter++] = std::move(WindowElement(name, rect));
}

const WindowElement& ElementList::GetElementByName(const std::string& elementName) const {
	auto [exists, id] = ElementNameExists(elementName);
	if (exists) {
		return elements.at(id);
	}
}

const WindowElement& ElementList::GetElementById(const Uint32 id) const {
	if (id < _idCounter) {
		return WindowElement();
	}
	return elements.at(id);
}

#if defined(_WIN32)
#pragma warning(suppress: 26495)	// Warning for uninitialized SDL_Event can be silenced, it is init before use.
#endif
GenericWindow::GenericWindow(const std::string& title, const EventHandler& eh) : _eventHandler(eh) {

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

			keepAlive = _eventHandler(_event, _elements);
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

	SDL_DestroyWindow(_window);
}

InitWindow::InitWindow(const std::string& title, const EventHandler& wev) : GenericWindow(title, wev) {
	// Init font
	TTF_Init();

	font = TTF_OpenFont("tahoma.ttf", 24);

	// Position buttons
	_clientButton.w = 300;
	_clientButton.h = 100;

	_clientButton.x = _clientButton.w * 2;
	_clientButton.y = (_height - _clientButton.h) / 2;

	_serverButton.w = 300;
	_serverButton.h = 100;

	_serverButton.x = (_width - _serverButton.w) - _serverButton.w;
	_serverButton.y = (_height - _serverButton.h) / 2;

	std::string elementName = "Client Button";
	_elements.Add(elementName, _clientButton);

	elementName = "Server Button";
	_elements.Add(elementName, _serverButton);
}

const bool InitWindow::Draw() {

	// Create button backgrounds

	_surface = SDL_CreateRGBSurface(0, _width, _height, 32, 0, 0, 0, 0);
	SDL_FillRect(_surface, &_clientButton, 0xff0000ff);
	SDL_FillRect(_surface, &_serverButton, 0xff0000ff);
	_texture = SDL_CreateTextureFromSurface(_renderer, _surface);
	SDL_RenderCopy(_renderer, _texture, NULL, NULL);

	SDL_DestroyTexture(_texture);
	SDL_FreeSurface(_surface);

	// Create button text
	_surface = TTF_RenderText_Solid(font, "Client", fontColor);
	_texture = SDL_CreateTextureFromSurface(_renderer, _surface);
	SDL_SetTextureBlendMode(_texture, SDL_BLENDMODE_NONE);
	SDL_RenderCopy(_renderer, _texture, NULL, &_clientButton);

	SDL_DestroyTexture(_texture);
	SDL_FreeSurface(_surface);

	_surface = TTF_RenderText_Solid(font, "Server", fontColor);
	_texture = SDL_CreateTextureFromSurface(_renderer, _surface);
	SDL_SetTextureBlendMode(_texture, SDL_BLENDMODE_NONE);
	SDL_RenderCopy(_renderer, _texture, NULL, &_serverButton);

	SDL_DestroyTexture(_texture);
	SDL_FreeSurface(_surface);

	SDL_RenderPresent(_renderer);

	return true;
}

InitWindow::~InitWindow() {
	TTF_CloseFont(font);
	TTF_Quit();
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

