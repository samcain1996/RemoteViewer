#include "Window.h"

const WindowElement& EventData::GetElementByName(const std::string& elementName) const {
	for (int i = 0; i < _elemList.size(); i++) {
		if (_elemList.at(i).get().Name() == elementName) { return _elemList.at(i); }
	}
	std::terminate(); // <-- Come back to this later
}

const WindowElement& EventData::GetElementById(const Uint32 id) const {
	if (_elemList.size() > id) { return _elemList.at(id); }
	std::terminate(); // <-- Come back to this later
}

void EventData::New(ElementList& elements, EventHandler& newEventHandler) const {

	_prevWindows.push_front(std::make_pair(elements, newEventHandler));

}

GenericWindow::GenericWindow(const std::string& title, const EventHandler& eventHandler) : _eventHandler(eventHandler) {

	SDL_GetDesktopDisplayMode(0, &_displayData);

	// Set width and height to be 75% monitor values
	_width = _displayData.w * 0.75f;
	_height = _displayData.h * 0.75f;

	// Center window
	_posX = (_displayData.w - _width) / 2;
	_posY = (_displayData.h - _height) / 2;

	_mouseRect.w = _mouseRect.h = 32;

	// Create window
	_window = SDL_CreateWindow(title.c_str(), _posX, _posY, _width, _height, NULL);

	_surface = nullptr;
	_texture = nullptr;

	_renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);

	_targetFPS = 60;

}

bool GenericWindow::LocalUpdate() {

	if (_event.type == SDL_MOUSEBUTTONDOWN) {
		SDL_GetMouseState(&_mouseRect.x, &_mouseRect.y);

		for (auto& element : _elements) {

			if (SDL_HasIntersection(&_mouseRect, &element.get().Bounds())) {

				if (_focussedElement) {
					_focussedElement->hasFocus = false;
				}

				_focussedElement = &element.get();
				_focussedElement->hasFocus = true;

				break;
			}
		}
	}

	if (_event.type == SDL_KEYDOWN) {
		if (_event.key.keysym.sym == SDLK_ESCAPE) {

			_prevWindows.pop_front();

			if (_prevWindows.empty()) { return false; }
			WindowData newData = _prevWindows.front();

			_elements = newData.first;
			_eventHandler = newData.second;

		}
	}

	return true;

}

GenericWindow::GenericWindow(const std::string& title, const EventHandler& eventHandler, const ElementList& els) :
	GenericWindow(title, eventHandler) {

	_elements = els;

	Update();
}

GenericWindow::GenericWindow(const std::string& title, const EventHandler& eventHandler, ElementList&& els) :
	GenericWindow(title, eventHandler) {

	_elements = std::move(els);
}


void GenericWindow::Update() {

	_prevWindows.push_front(std::make_pair(_elements, _eventHandler));

	Uint32 ticks = SDL_GetTicks();
	SDL_GetMouseState(&_mouseRect.x, &_mouseRect.y);

	while (_keepAlive) {
		
		// Get events
		while (SDL_PollEvent(&_event)) {

			if (!LocalUpdate()) {
				return;
			}

			EventData eventData(_event, _mouseRect, _width, _height, _focussedElement, _prevWindows, std::ref(_elements));
			bool newElements = _eventHandler(eventData);

			if (newElements) {
				
				if (!_prevWindows.empty()) {

					WindowData newData = _prevWindows.front();

					_elements = newData.first;
					_eventHandler = newData.second;


				}

				continue; 
			}
			

			if (_event.type == SDL_QUIT) {
				_keepAlive = false;
				break;
			}

			if (_focussedElement != nullptr) {
				_focussedElement->Update(_event);
			}
		}

		if (CapFPS2(ticks)) {

			Draw();
			ticks = SDL_GetTicks();
		}
		continue;

		//CapFPS(ticks);

	}

}

void GenericWindow::Draw() {

	SDL_SetRenderDrawColor(_renderer, 0, 0, 0, 255);
	SDL_RenderClear(_renderer);

	for (WindowElement& windowElement : _elements) {
		windowElement.RenderElement(_renderer);
	}

	SDL_RenderPresent(_renderer);

}

void GenericWindow::CapFPS(const Uint32 prevTicks) {
	if ((MillisInSecs / _targetFPS) > SDL_GetTicks() - prevTicks) { 
		SDL_Delay(MillisInSecs / _targetFPS - (SDL_GetTicks() - prevTicks));
	}

	std::cout << "FPS: " << std::to_string((SDL_GetTicks() - prevTicks) / 1000.0) << "\n";
}

bool GenericWindow::CapFPS2(const Uint32 prevTicks) {
	return !((MillisInSecs / _targetFPS) > SDL_GetTicks() - prevTicks);
}

GenericWindow::~GenericWindow() {

	//TTF_CloseFont(_font);
	//SDL_DestroyRenderer(_renderer);
	//SDL_DestroyWindow(_window);
}

RenderWindow::RenderWindow(const std::string& title, EventHandler& eventHandler) :
	GenericWindow(title, eventHandler) {

	// Allocate memory for bitmap
	_bitmapSize = 54 + CalculateTheoreticalBMPSize(_width, _height);
	_bitmap = new Byte[_bitmapSize];

	_bmpDataStream = SDL_RWFromMem(_bitmap, _bitmapSize);

}

void RenderWindow::Draw() {


	if (msgReader->Empty()) { return; } // No image is ready, skip frame

	// Assemble image buffer
	AssembleImage(msgReader->ReadMessage());

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

	delete[] _bitmap;
}


