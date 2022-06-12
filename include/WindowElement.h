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

	bool hasFocus = false;

	WindowElement();
	WindowElement(const std::string& name, const SDL_Rect& rect);

	WindowElement(WindowElement&&) = delete;
	WindowElement(const WindowElement&) = delete;

	virtual ~WindowElement();

	WindowElement& operator=(const WindowElement& other) = delete;
	WindowElement& operator=(WindowElement&& other) = delete;

	virtual void Update(SDL_Event& ev) {};
	virtual void RenderElement(SDL_Renderer* const ghrenderer) = 0;

	const int Id() const;
	const std::string& Name() const;
	const SDL_Rect& Bounds() const;

	const int GetNextId() const;

};


class Button : public WindowElement {
private:
	TTF_Font* _font;
	std::string _text = "";
	SDL_Color _fontColor, _backgroundColor;

	Button(TTF_Font* font, const SDL_Color& fontColor, const SDL_Color& backColor, const std::string& name, const SDL_Rect& bounds);

public:
	Button() = delete;

	Button(Button&&) = delete;
	Button(const Button&) = delete;

	~Button();

	Button& operator=(const Button& other) = delete;
	Button& operator=(Button && other) = delete;

	Button(int x, int y, const std::string& name, const std::string& text);

	void RenderElement(SDL_Renderer* const renderer) override;
};

class TextBox : public WindowElement {
private:
	TTF_Font* _font;
	std::string _text = "";
	SDL_Rect _cursorBarRect;
	bool displayBar = false;
	const Ushort skipFrames = 15;
	Ushort curFrame = 0;

	TextBox(TTF_Font* font, const std::string& name, const std::string& text, const SDL_Rect& bounds);

public:

	TextBox() = delete;

	TextBox(const TextBox&) = delete;
	TextBox(TextBox&&) = delete;

	~TextBox();

	TextBox& operator=(const TextBox&) = delete;
	TextBox& operator=(TextBox&&) = delete;

	
	TextBox(int x, int y, const std::string& name, const std::string& text);

	void RenderElement(SDL_Renderer* const renderer) override;

	void Update(SDL_Event& ev) override;

	const std::string& Text() const;
};