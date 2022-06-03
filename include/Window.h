#pragma once

#include "WindowElement.h"
#include <iostream>
#include <functional>

class GenericWindow;
class InitWindow;

struct ElementView {
	std::vector<std::reference_wrapper<WindowElement>>& elements;

	ElementView() = delete;
	ElementView(std::vector<std::reference_wrapper<WindowElement>>& elements) : elements(elements) {}
	WindowElement& GetElementByName(const std::string& elementName) const;
	WindowElement& GetElementById(const Uint32 id) const;
};

using EventHandler = std::function<bool(const SDL_Event&, const ElementView&)>;

class GenericWindow {

protected:

	TTF_Font* _font;

	GenericWindow(const std::string& title, const EventHandler& eventHandler);

	GenericWindow() = delete;

	GenericWindow(const GenericWindow&) = delete;
	GenericWindow(GenericWindow&&) = delete;

	GenericWindow& operator=(const GenericWindow&) = delete;
	GenericWindow& operator=(GenericWindow&&) = delete;

	std::vector<std::reference_wrapper<WindowElement>> _elements;
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

	virtual const bool Draw();  // Draw to window

	void CapFPS(const Uint32 prevTicks);  // Limit FPS

public:
	GenericWindow(const std::string& title, const EventHandler& eventHandler, std::vector< std::reference_wrapper<WindowElement>>& els);
	virtual void Update();
	virtual ~GenericWindow();
};

class InitWindow : public GenericWindow {
private:
	TTF_Font* _font;
	Button* _clientButton;
	Button* _serverButton;
	TextBox* _tBox;

public:
	InitWindow(const std::string& title, const EventHandler& eventHandler);

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

	RenderWindow(const std::string& title, const EventHandler& eventHandler);

	RenderWindow(const RenderWindow&) = delete;
	RenderWindow(RenderWindow&&) = delete;

	~RenderWindow();

	RenderWindow& operator=(const RenderWindow&) = delete;
	RenderWindow& operator=(RenderWindow&&) = delete;

	MsgReaderPtr<PacketPriorityQueue*> completeGroups = nullptr;
};