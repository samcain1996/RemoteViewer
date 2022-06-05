#pragma once

#include "WindowElement.h"
#include <iostream>
#include <functional>

using ElementList = std::vector<std::reference_wrapper<WindowElement>>;

struct ElementView {
	const WindowElement& elemInFocus;
	ElementList& elements;

	ElementView() = delete;
	ElementView(ElementList& elements, WindowElement& elementInFocus) : elements(elements), elemInFocus(elementInFocus) {}

	WindowElement& GetElementByName(const std::string& elementName) const;
	WindowElement& GetElementById(const Uint32 id) const;
};

struct EventData {
	const SDL_Event& windowEvent;
	const SDL_Rect& mouseRect;
	const int& windowWidth;
	const int& windowHeight;

	EventData(const SDL_Event& evnt, const SDL_Rect& mouseRect, const int width, const int height) : 
		windowEvent(evnt), mouseRect(mouseRect), windowWidth(width), windowHeight(height) {};
};

using EventHandler = std::function<bool(const EventData&, const ElementView&)>;

class GenericWindow {

protected:
	bool keepAlive = true;

	TTF_Font* _font;

	GenericWindow(const std::string& title, const EventHandler& eventHandler);

	GenericWindow() = delete;

	GenericWindow(const GenericWindow&) = delete;
	GenericWindow(GenericWindow&&) = delete;

	GenericWindow& operator=(const GenericWindow&) = delete;
	GenericWindow& operator=(GenericWindow&&) = delete;

	ElementList _elements;
	EventHandler _eventHandler;

	void GetFocus();

	WindowElement* _focussedElement = nullptr;

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

	virtual void Draw();  // Draw to window

	void CapFPS(const Uint32 prevTicks);  // Limit FPS

public:
	GenericWindow(const std::string& title, const EventHandler& eventHandler, ElementList& els);
	GenericWindow(const std::string& title, const EventHandler& eventHandler, ElementList&& els);
	virtual void Update();
	virtual ~GenericWindow();
};

class RenderWindow : public GenericWindow, public Messageable<PacketPriorityQueue*> {

private:
	SDL_RWops* _bmpDataStream;  // Current image to render

	ByteArray _bitmap;  // Buffer to hold image to render
	Uint32 _bitmapSize; // Buffer size

	void Draw() override;  // Draws _bitmap to the window
	void AssembleImage(PacketPriorityQueue* const queue);  // Assembles _bitmap from image fragments

public:
	RenderWindow() = delete;

	RenderWindow(const std::string& title, const EventHandler& eventHandler);

	RenderWindow(const RenderWindow&) = delete;
	RenderWindow(RenderWindow&&) = delete;

	~RenderWindow();

	RenderWindow& operator=(const RenderWindow&) = delete;
	RenderWindow& operator=(RenderWindow&&) = delete;
};

template <class T>
concept IsTypeOfWindow = std::derived_from<T, GenericWindow> || std::same_as<T, GenericWindow>;

template <class Window>
concept IsSubTypeOfWindow = IsTypeOfWindow<Window> && !std::same_as<Window, GenericWindow>;

//template <class Window>