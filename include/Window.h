#pragma once

#include "Packet.h"
#include "Messages.h"
#include <functional>

class ElementList;

class WindowElement {
	friend ElementList;
private:
	std::string _name;
	SDL_Rect _bounds;

	WindowElement(const std::string& name, const SDL_Rect& rect);

public:

	WindowElement();
	
	WindowElement(WindowElement&&) = delete;
	WindowElement(const WindowElement&) = delete;

	WindowElement& operator=(const WindowElement& other) = delete;
	WindowElement& operator=(WindowElement&& other) noexcept;

	const std::string& Name() const;
	const SDL_Rect& Bounds() const;

};

class GenericWindow;
class InitWindow;
class RenderWindow;

class ElementList {
	friend GenericWindow;
	friend InitWindow;
	friend RenderWindow;
private:
	Uint32 _idCounter = 0;
	std::unordered_map<Uint32, WindowElement> elements;

	const std::pair<bool, Uint32> ElementNameExists(const std::string& name) const;

	void Add(std::string& name, const SDL_Rect& rect);

public:

	const WindowElement& GetElementByName(const std::string& elementName) const;
	const WindowElement& GetElementById(const Uint32 id) const;

};

using EventHandler = std::function<bool(const SDL_Event&, const ElementList&)>;

class GenericWindow {

protected:

	GenericWindow(const std::string& title, const EventHandler& eh);

	GenericWindow() = delete;

	GenericWindow(const GenericWindow&) = delete;
	GenericWindow(GenericWindow&&) = delete;

	GenericWindow& operator=(const GenericWindow&) = delete;
	GenericWindow& operator=(GenericWindow&&) = delete;

	ElementList _elements;
	EventHandler _eventHandler;

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
	virtual void Update();
	virtual ~GenericWindow();
};

class InitWindow : public GenericWindow {
private:
	TTF_Font* font;
	SDL_Color fontColor = { 255,255,0,100 };
	SDL_Rect _clientButton, _serverButton;

public:
	InitWindow(const std::string& title, const EventHandler& wev);

	const bool Draw() override;
	void Update() override {

		Uint32 ticks = 0;
		bool keepAlive = true;

		Draw();

		while (keepAlive) {

			while (SDL_PollEvent(&_event)) {
				keepAlive = _eventHandler(_event, _elements);
			}

		}
	}

	~InitWindow();
};

class RenderWindow : public GenericWindow {

private:
	SDL_RWops* _bmpDataStream;  // Current image to render

	ByteArray _bitmap;  // Buffer to hold image to render

	Uint32 _bitmapSize; // Buffer size

	const bool Draw() override;  // Draws _bitmap to the window
	void AssembleImage(PacketPriorityQueue* const queue);  // Assembles _bitmap from image fragments

public:
	RenderWindow() = delete;

	RenderWindow(const std::string& title, const EventHandler& ev);

	RenderWindow(const RenderWindow&) = delete;
	RenderWindow(RenderWindow&&) = delete;

	~RenderWindow();

	RenderWindow& operator=(const RenderWindow&) = delete;
	RenderWindow& operator=(RenderWindow&&) = delete;

	MsgReaderPtr<PacketPriorityQueue*> completeGroups = nullptr;
};