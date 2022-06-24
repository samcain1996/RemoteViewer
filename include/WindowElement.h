#pragma once
#include "Packet.h"
#include "Messages.h"

using FontRef = std::reference_wrapper<TTF_Font* const>;
using FontPtr = std::unique_ptr<TTF_Font*>;

class FontPool {

private:
	std::unordered_map<TTF_Font*, std::string> _fonts;
	FontPtr _defaultFont;

public:

	FontPool();
	
	FontPool(const FontPool&) = delete;
	FontPool(FontPool&&) = delete;

	FontPool& operator=(const FontPool&) = delete;
	FontPool& operator=(FontPool&&) = delete;

	~FontPool();

	const FontRef FindFont(const std::string& fontName);

};

class WindowElement;
using ElementList = std::vector<std::reference_wrapper<WindowElement>>;

class WindowElement {

	friend bool operator==(const WindowElement& we1, const WindowElement& we2) {
		return we1._elementId == we2._elementId;
	}

private:
	static int idGen;
	const int _elementId;
	
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
	
	// Ratio for how often this element should ne updated
	// relative to the window it is on.
	float _updateRatioToWindow = 1 / 15.0f;

	static FontPool _fontPool;

	FontRef _font;

public:

	WindowElement();
	WindowElement(const std::string& name, const SDL_Rect& rect, const std::string& fontName = "default", 
		const std::string& label = "PLACEHOLDER", const SDL_Color& backColor = PINK, const SDL_Color& textColor = GREEN);

	WindowElement(WindowElement&&) = delete;
	WindowElement(const WindowElement&) = delete;

	virtual ~WindowElement();

	WindowElement& operator=(const WindowElement& other) = delete;
	WindowElement& operator=(WindowElement&& other) = delete;

	virtual const bool ReadyToUpdateFrame();
	virtual void DrawUpdate();
	virtual void Update(SDL_Event& ev);  // Called every frame
	virtual void RenderElement(SDL_Renderer* const ghrenderer) = 0;

	virtual void UpdateOnFrameFunction();
	virtual void Unfocus();

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

	ValidatorList<const char> _inputValidators;

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

	void AddValidator(const Validator<const char>& validator);

	void DrawUpdate() override;
	const std::string& Text() const;
};