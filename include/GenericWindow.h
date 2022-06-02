#pragma once

#include "Types.h"
#include "Packet.h"
#include "Messages.h"
#include <functional>

class GenericWindow;
class InitWindow;
class RenderWindow;

class ElementList;

class WindowElement {
	friend ElementList;
private:
	std::string name;
	SDL_Rect bounds;

	WindowElement(const std::string& name, const SDL_Rect& rect) :
		name(name), bounds(rect) {}

public:

	WindowElement() {
		name = "ERROR";
		bounds = SDL_Rect();
	}
	
	WindowElement(WindowElement&&) = delete;
	WindowElement(const WindowElement&) = delete;

	WindowElement& operator=(const WindowElement& other) = delete;
	WindowElement& operator=(WindowElement&& other) noexcept {
		name = std::move(other.name);
		bounds = std::move(other.bounds);
		return *this;
	}

	const std::string& Name() const { return name; }
	const SDL_Rect& Bounds() const { return bounds; }

};


class ElementList {
	friend GenericWindow;
	friend InitWindow;
	friend RenderWindow;
private:
	Uint32 _idCounter = 0;
	std::unordered_map<Uint32, WindowElement> elements;

	const std::pair<bool, Uint32> ElementNameExists(const std::string& name) const {

		for (const auto& [id, elem] : elements) {
			if (elem.name == name) { return std::make_pair(true, id); }
		}
		return std::make_pair(false, 0);
	}

	void Add(std::string& name, const SDL_Rect& rect) {
		if (ElementNameExists(name).first) {
			name = std::to_string(_idCounter);
		}

		elements[_idCounter++] = std::move(WindowElement(name, rect));
	}

public:

	const WindowElement& GetElement(const std::string& name) const {
		auto [exists, id] = ElementNameExists(name);
		if (exists) {
			return elements.at(id);
		}
	}

};

using EventHandler = std::function<bool(const SDL_Event&, const ElementList&)>;

class GenericWindow {

protected:

	ElementList _elements;
	EventHandler _eventHandler;

	GenericWindow() = delete;

	GenericWindow(const GenericWindow&) = delete;
	GenericWindow(GenericWindow&&) = delete;

	GenericWindow& operator=(const GenericWindow&) = delete;
	GenericWindow& operator=(GenericWindow&&) = delete;

	SDL_Window* _window;	// GenericWindow to render to
	SDL_Surface* _surface;	// Pixel data to render to window
	SDL_Renderer* _renderer;	// Render remote screen to window
	SDL_Texture* _texture;	// Driver-specific, pixel data used to render

	SDL_DisplayMode _displayData;  // Data about connected monitors

	SDL_Event _event;  // Used to get input from user

	SDL_Rect _mouseRect;

	int _width, _height;  // Width and height of window
	int _posX, _posY;	  // X and Y position of window

	Uint32 _targetFPS;	  // FPS to target

	virtual const bool Draw() = 0;  // Draw to window

	void CapFPS(const Uint32 prevTicks);  // Limit FPS

public:
	GenericWindow(const std::string& title, const EventHandler& eh);
	virtual void Update();
	virtual ~GenericWindow();
};

class InitWindow : public GenericWindow {
private:
	TTF_Font* font;
	SDL_Color fontColor = { 255,255,0,100 };
	SDL_Rect _clientButton;
	SDL_Rect _serverButton;

public:
	InitWindow(const std::string& title, const EventHandler& wev) : GenericWindow(title, wev) {
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

		std::string temp = "Client Button";

		_elements.Add(temp, _clientButton);

		temp = "Server Button";
		_elements.Add(temp, _serverButton);
	}

	const bool Draw() override {
		_surface = TTF_RenderText_Solid(font, "Client", fontColor);
		_texture = SDL_CreateTextureFromSurface(_renderer, _surface);
		SDL_RenderCopy(_renderer, _texture, NULL, &_clientButton);

		SDL_DestroyTexture(_texture);
		SDL_FreeSurface(_surface);

		_surface = TTF_RenderText_Solid(font, "Server", fontColor);
		_texture = SDL_CreateTextureFromSurface(_renderer, _surface);
		SDL_RenderCopy(_renderer, _texture, NULL, &_serverButton);

		SDL_DestroyTexture(_texture);
		SDL_FreeSurface(_surface);

		SDL_RenderPresent(_renderer);

		return true;
	};

	~InitWindow() {
		TTF_CloseFont(font);
		TTF_Quit();
	}
};