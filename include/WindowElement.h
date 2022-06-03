#pragma once
#include "Packet.h"
#include "Messages.h"

class WindowElement {

	friend bool operator==(const WindowElement& we1, const WindowElement& we2) {
		return we1._elementId == we2._elementId;
	}

private:
	static int idGen;
	const int* const _elementId;

protected:
	std::string _name;
	SDL_Rect _bounds;

public:

	WindowElement();
	WindowElement(const std::string& name, const SDL_Rect& rect);

	WindowElement(WindowElement&&) = delete;
	WindowElement(const WindowElement&) = delete;

	virtual ~WindowElement();

	WindowElement& operator=(const WindowElement& other) = delete;
	WindowElement& operator=(WindowElement&& other) = delete;

	virtual void Update() {};

	virtual void RenderElement(SDL_Renderer* renderer) = 0;

	const int Id() const;
	const std::string& Name() const;
	const SDL_Rect& Bounds() const;

	const int GetNextId() const;

};


class Button : public WindowElement {
private:
	TTF_Font* _font;
	SDL_Color _fontColor, _backgroundColor;

public:
	Button() = delete;

	Button(Button&&) = delete;
	Button(const Button&) = delete;

	Button& operator=(const Button& other) = delete;
	Button& operator=(Button&& other) = delete;

	Button(TTF_Font* font, const SDL_Color& fontColor, const SDL_Color& backColor, const std::string& name, const SDL_Rect& bounds);

	void RenderElement(SDL_Renderer* renderer) override;
};

class TextBox : public WindowElement {
private:
	TTF_Font* _font;
	std::string _text;
	SDL_Rect cursorBarRect;
	bool displayBar = false;

public:

	TextBox() = delete;

	TextBox(const TextBox&) = delete;
	TextBox(TextBox&&) = delete;

	TextBox& operator=(const TextBox&) = delete;
	TextBox& operator=(TextBox&&) = delete;

	TextBox(TTF_Font* font, const std::string& name, const SDL_Rect& bounds);

	void RenderElement(SDL_Renderer* renderer) override;

	void Update() override {

	}

	void RemoveLetter() { if (!_text.empty()) { _text.erase(_text.end() - 1); } }
	void Add(const char letter) { _text += letter; }
	const std::string& Text() const { return _text; }
};