#pragma once

#include "WindowElement.h"
#include <forward_list>
#include <iostream>

using ElementList = std::vector<std::reference_wrapper<WindowElement>>;

class EventData;
using EventHandler = std::function<bool(EventData&)>;
using WindowCore = std::pair<ElementList, EventHandler>;

using WindowList = std::forward_list<WindowCore>;

class EventData {

public:
	const SDL_Event& windowEvent;
	const SDL_Rect& mouseRect;
	const int& windowWidth;
	const int& windowHeight;

	WindowList& _windowList;
	ElementList& _elemList;
	const WindowElement* const elemInFocus;

	EventData(const SDL_Event& evnt, const SDL_Rect& mouseRect, const int width, const int height,
		const WindowElement* const elementInFocus, WindowList& windowList,
		ElementList& elementList) :
		windowEvent(evnt), mouseRect(mouseRect), windowWidth(width), windowHeight(height), 
		elemInFocus(elementInFocus), _windowList(windowList), _elemList(elementList) {};

	const WindowElement& GetElementByName(const std::string& elementName) const;
	const WindowElement& GetElementById(const Uint32 id) const;

	void ChangeWindow(ElementList& elements, EventHandler& newEventHandler) const;

};



class GenericWindow {

private:

	bool LocalUpdate();

protected:
	GenericWindow(const std::string& title, const EventHandler& eventHandler);

	GenericWindow() = delete;

	GenericWindow(const GenericWindow&) = delete;
	GenericWindow(GenericWindow&&) = delete;

	GenericWindow& operator=(const GenericWindow&) = delete;
	GenericWindow& operator=(GenericWindow&&) = delete;

	bool _keepAlive = true;

	TTF_Font* _font;

	ElementList _elements;
	EventHandler _eventHandler;

	WindowList _windowList;

	WindowElement* _focussedElement = nullptr;

	SDL_Window* _window;	// GenericWindow to render to
	SDL_Surface* _surface;	// Pixel data to render to window
	SDL_Texture* _texture;	// Driver-specific, pixel data used to render
	SDL_Renderer* _renderer;	// Render remote screen to window

	SDL_DisplayMode _displayData;  // Data about connected monitors

	SDL_Event _event; 
	SDL_Rect _mouseRect; 

	int _width, _height;  
	int _posX, _posY;	 

	Uint32 _targetFPS;

	virtual void Draw(); 

	void CapFPS(const Uint32 prevTicks);  
	bool CapFPS2(const Uint32 prevTicks);

public:
	GenericWindow(const std::string& title, const EventHandler& eventHandler, const ElementList& els);
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

	RenderWindow(const std::string& title, EventHandler& eventHandler);

	RenderWindow(const RenderWindow&) = delete;
	RenderWindow(RenderWindow&&) = delete;

	~RenderWindow();

	RenderWindow& operator=(const RenderWindow&) = delete;
	RenderWindow& operator=(RenderWindow&&) = delete;
};

template <class T>
concept IsTypeOfWindow = std::derived_from<T, GenericWindow> || std::same_as<T, GenericWindow>;

template <class T>
concept IsSubTypeOfWindow = IsTypeOfWindow<T> && !std::same_as<T, GenericWindow>;