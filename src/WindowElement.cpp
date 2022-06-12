#include "WindowElement.h"

// Window Element

int WindowElement::idGen = 0;

FontPool WindowElement::_fontPool;

WindowElement::WindowElement(const std::string& name, const SDL_Rect& rect, const std::string& fontName,
	const std::string& label, const SDL_Color& backColor, const SDL_Color& textColor) : 
	_elementId(GetNextId()), _name(name), _label(label), _font(_fontPool.FindFont(fontName)), _backColor(backColor), _textColor(textColor) {

	_xPos = rect.x;
	_yPos = rect.y;

	_width = rect.w;
	_height = rect.h;

	_skippedFrames = 1 / _updateRatioToWindow;

};

WindowElement::WindowElement() : WindowElement("ERROR", SDL_Rect()) {}

WindowElement::~WindowElement() {}

void WindowElement::Update(SDL_Event& ev) {

	if (_updateElement) { _updateElement = false; }

	if (_currentFrame++ == _skippedFrames) {
		_currentFrame = 0;
		_updateElement = true;
	}

}

void WindowElement::Unfocus() {
	_currentFrame = 0;
}

const int WindowElement::Id() const { return _elementId; }
const std::string& WindowElement::Name() const { return _name; }
const SDL_Rect& WindowElement::Bounds() const { 

	_bounds.x = _xPos;
	_bounds.y = _yPos;
	_bounds.w = _width;
	_bounds.h = _height;

	return _bounds;
}

const int WindowElement::GetNextId() const {
	return idGen++;
}


// Button

Button::Button(const std::string& fontName, const SDL_Color& fontColor, const SDL_Color& backColor, const std::string& name, const SDL_Rect& bounds) :
	WindowElement(name, bounds, fontName, name, fontColor, backColor) {
}

Button::Button(int x, int y, const std::string& name, const std::string& text) : Button("default", GREEN, PINK,
	name, SDL_Rect{ x, y, 300, 150 }) { 
	_label = text;
};

void Button::RenderElement(SDL_Renderer* const renderer) {

	SDL_Surface* surface = TTF_RenderText_Shaded(_font, _label.c_str(), _textColor, _backColor);
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

	SDL_RenderCopy(renderer, texture, NULL, &Bounds());

	SDL_FreeSurface(surface);
	SDL_DestroyTexture(texture);

}

Button::~Button() {

}


// Textbox

void TextBox::RenderElement(SDL_Renderer* const renderer) {

	SDL_Surface* surface = TTF_RenderText_Shaded(_font, _label.c_str(), BLACK, WHITE);
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

	_width = _label.size() * 20;
	_cursorBarRect.x = _xPos + _width;

	SDL_RenderCopy(renderer, texture, NULL, &Bounds());

	SDL_FreeSurface(surface);
	SDL_DestroyTexture(texture);

	const SDL_Color& drawColor = displayCursorBar ? GREEN : WHITE;

	SDL_SetRenderDrawColor(renderer, drawColor.r, drawColor.g, drawColor.b, 100);
	SDL_RenderFillRect(renderer, &_cursorBarRect);
}

TextBox::TextBox(const std::string& fontName, const std::string& name, const std::string& text, const SDL_Rect& bounds) :
	WindowElement(name, bounds, fontName, text) {

	_cursorBarRect = Bounds();
	_cursorBarRect.w = 10;
	_cursorBarRect.x += ((_width / 2.0) - (_cursorBarRect.w / 2.0));
}

TextBox::TextBox(int x, int y, const std::string& name, const std::string& text) :
	TextBox("default", name, text, SDL_Rect{x, y, 300, 100}) {}

TextBox::TextBox(int x, int y, const std::string& name, const std::string& text, const Validator<const char>& validator) :
	TextBox("default", name, text, SDL_Rect{ x, y, 300, 100 }) {
	
	_inputValidators.push_back(validator);
}

void TextBox::Update(SDL_Event& ev) {

	WindowElement::Update(ev);

	if (_updateElement) {
		displayCursorBar = !displayCursorBar;
	}

	if (ev.type == SDL_KEYDOWN) {

		const char& key = ev.key.keysym.sym;

		if (key == SDLK_BACKSPACE) {
			if (_label.size() > 0) {
				_label.pop_back();
			}
		}
		else {
			
			for (auto& validator : _inputValidators) {
				if (!validator(key)) {
					return;
				}
			}
			
			_label += key;

		}
	}
}

void TextBox::Unfocus() {
	
	WindowElement::Unfocus();
	
	displayCursorBar = false;
	
}

const std::string& TextBox::Text() const { return _label; }

void TextBox::AddValidator(const Validator<const char>& validator) {

	_inputValidators.push_back(validator);

}


TextBox::~TextBox() {
}

