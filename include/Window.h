#pragma once

#include "WindowElement.h"
#include <forward_list>
#include <iostream>

class EventData;
using EventHandler = std::function<bool(EventData&)>;
using WindowCore = std::pair<ElementList, EventHandler>;
using WindowList = std::forward_list<WindowCore>;

struct WindowData {
	SDL_Event& windowEvent;
	SDL_Rect& mouseRect;
	int& windowWidth;
	int& windowHeight;

	WindowData(SDL_Event& evnt, SDL_Rect& mouseRect, int width, int height);
};

class EventData {
private:
	
	WindowList& _windowList;
	ElementList& _elemList;
	int _focusIdx;
	
public:
	
	const WindowData& _windowData;


	EventData(const WindowData& windowData, const int focusIdx, WindowList& windowList, ElementList& elementList);

	EventData(const EventData&) = delete;
	EventData(EventData&&) = delete;

	EventData& operator=(const EventData&) = delete;
	EventData& operator=(EventData&&) = delete;

	const WindowElement& GetElementByName(const std::string& elementName) const;
	const WindowElement& GetElementById(const Uint32 id) const;
	const WindowElement& GetFocussedElement() const;

	void ChangeWindow(ElementList& elements, EventHandler& newEventHandler) const;

};



class GenericWindow {

private:

	bool GetTopWindow();
	bool LocalUpdate();

protected:
	
	GenericWindow(const std::string& title, const EventHandler& eventHandler);

	GenericWindow() = delete;

	GenericWindow(const GenericWindow&) = delete;
	GenericWindow(GenericWindow&&) = delete;

	GenericWindow& operator=(const GenericWindow&) = delete;
	GenericWindow& operator=(GenericWindow&&) = delete;

	bool _keepAlive = true;

	ElementList _elements;
	EventHandler _eventHandler;

	WindowList _windowList;

	int focussedElementIndex = -1;

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