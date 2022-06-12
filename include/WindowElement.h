#pragma once
#include "RenderTypes.h"

class WindowElement {

	friend bool operator==(const WindowElement& we1, const WindowElement& we2) {
		return we1._elementId == we2._elementId;
	}

private:
	static int idGen;
	const int* const _elementId;
	
	int _skippedFrames = 0;
	int _currentFrame = 0;

	mutable SDL_Rect _bounds = { 0, 0, 0, 0 };

protected:
	int _xPos = 0;
	int _yPos = 0;
	int _width = 0;
	int _height = 0;
	
	std::string _name = "";
	std::string _label = "";

	SDL_Color _backColor = PINK;
	SDL_Color _textColor = GREEN;
	
	float _updateRatioToWindow = 1 / 15.0f;

	static FontPool _fontPool;

	FontRef _font;
	
public:

	WindowElement();
	WindowElement(const std::string& name, const SDL_Rect& rect, const std::string& fontName = "default");

	WindowElement(WindowElement&&) = delete;
	WindowElement(const WindowElement&) = delete;

	virtual ~WindowElement();

	WindowElement& operator=(const WindowElement& other) = delete;
	WindowElement& operator=(WindowElement&& other) = delete;

	virtual void Update(SDL_Event& ev);
	virtual bool UpdateDraw();
	virtual void Unfocus();
	virtual void RenderElement(SDL_Renderer* const ghrenderer) = 0;

	const int Id() const;
	const std::string& Name() const;
	const SDL_Rect& Bounds() const;

	const int GetNextId() const;

};


class Button : public WindowElement {
private:

	Button(const std::string& fontName, const SDL_Color& fontColor, const SDL_Color& backColor, const std::string& name, const SDL_Rect& bounds);

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
	SDL_Rect _cursorBarRect;
	bool displayCursorBar = false;

	Validator<const char> _validator;

	TextBox(const std::string& fontName, const std::string& name, const std::string& text, const SDL_Rect& bounds);

public:

	TextBox() = delete;

	TextBox(const TextBox&) = delete;
	TextBox(TextBox&&) = delete;

	~TextBox();

	TextBox& operator=(const TextBox&) = delete;
	TextBox& operator=(TextBox&&) = delete;

	TextBox(int x, int y, const std::string& name, const std::string& text);
	TextBox(int x, int y, const std::string& name, const std::string& text, const Validator<const char>& validator);

	void RenderElement(SDL_Renderer* const renderer) override;

	void Update(SDL_Event& ev) override;
	void Unfocus() override;

	const std::string& Text() const;
};