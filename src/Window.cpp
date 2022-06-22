#include "Window.h"


WindowData::WindowData(SDL_Event& evnt, SDL_Rect& mouseRect, int width, int height) :
	windowEvent(evnt), mouseRect(mouseRect), windowWidth(width), windowHeight(height) {
}


EventData::EventData(const WindowData& windowData, const int focusIdx, WindowList& windowList, ElementList& elementList) :
	_windowData(windowData), _focusIdx(focusIdx), _windowList(windowList), _elemList(elementList) {}

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

const WindowElement& EventData::GetFocussedElement() const {
	return _elemList[_focusIdx];
}

void EventData::ChangeWindow(ElementList& elements, EventHandler& newEventHandler) const {

	_windowList.push_front(std::make_pair(elements, newEventHandler));

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

	// Get focused element
	if (_event.type == SDL_MOUSEBUTTONDOWN) {

		SDL_GetMouseState(&_mouseRect.x, &_mouseRect.y);

		for (size_t index = 0; index < _elements.size(); index++) {

			WindowElement& element = _elements.at(index);

			if (SDL_HasIntersection(&_mouseRect, &element.Bounds())) {

				if (focussedElementIndex > 0 && focussedElementIndex != index) {
					_elements[focussedElementIndex].get().Unfocus();
				}

				focussedElementIndex = index;

				break;

			}
		}
	}

	// Go back a window if escape is hit
	if (_event.type == SDL_KEYDOWN) {

		const char& key = _event.key.keysym.sym;

		if (key == SDLK_ESCAPE) {

			focussedElementIndex = -1;

			_windowList.pop_front();

			if (std::empty(_windowList)) { return false; }


		}

	}

	_eventHandler = _windowList.front().second;
	_elements = _windowList.front().first;

	return true;

}

GenericWindow::GenericWindow(const std::string& title, const EventHandler& eventHandler, const ElementList& els) :
	GenericWindow(title, eventHandler) {

	_elements = els;

	_windowList.push_front(std::make_pair(_elements, _eventHandler));

	Update();
}

GenericWindow::GenericWindow(const std::string& title, const EventHandler& eventHandler, ElementList&& els) :
	GenericWindow(title, eventHandler) {

	_elements = std::move(els);

	_windowList.push_front(std::make_pair(_elements, _eventHandler));
}


void GenericWindow::Update() {

	Uint32 ticks = SDL_GetTicks();
	SDL_GetMouseState(&_mouseRect.x, &_mouseRect.y);
	
	while (_keepAlive) {

		// Get events
		while (SDL_PollEvent(&_event)) {
		
			if (!LocalUpdate()) {
				return;
			}

			WindowData windowData(_event, _mouseRect, _width, _height);
			EventData eventData(windowData, focussedElementIndex, _windowList, std::ref(_elements));
			
			_keepAlive = _eventHandler(eventData);
			if (!_keepAlive) { return; }

			if (_windowList.front().first != _elements) {
				const WindowCore& newWindow = _windowList.front();
				
				_elements = newWindow.first;
				_eventHandler = newWindow.second;

				focussedElementIndex = -1;
			}

			if (_event.type == SDL_QUIT) {
				_keepAlive = false;
				break;
			}

			if (focussedElementIndex > -1) {
				_elements[focussedElementIndex].get().Update(_event);
			}


		}

		if (CapFPS2(ticks)) {
			
			Draw();
			ticks = SDL_GetTicks();
		}

	}

}

void GenericWindow::Draw() {

	if (focussedElementIndex > -1) {
		_elements[focussedElementIndex].get().DrawUpdate();
	}
	
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

	SDL_DestroyRenderer(_renderer);
	SDL_DestroyWindow(_window);
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
