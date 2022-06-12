#include "WindowElement.h"

const InputValidator NUMERIC_INPUT_VALIDATOR = [](const char input) {
	return (input > '0' && input < '9');
};

const InputValidator ALPHABETIC_INPUT_VALIDATOR = [](const char input) {
	return ((input > 'a' && input < 'z') || (input > 'A' && input < 'Z'));
};

const InputValidator ALPHANUMERIC_INPUT_VALIDATOR = [](const char input) {
	return NUMERIC_INPUT_VALIDATOR(input) || ALPHABETIC_INPUT_VALIDATOR(input);
};

// Window Element

int WindowElement::idGen = 0;

WindowElement::WindowElement(const std::string& name, const SDL_Rect& rect) :
	_elementId(new int{ GetNextId() }), _name(name), _bounds(rect) {};

WindowElement::WindowElement() : WindowElement("ERROR", SDL_Rect()) {}

WindowElement::~WindowElement() {
	delete _elementId;
}

const int WindowElement::Id() const { return *_elementId; }
const std::string& WindowElement::Name() const { return _name; }
const SDL_Rect& WindowElement::Bounds() const { return _bounds; }

const int WindowElement::GetNextId() const {
	return idGen++;
}


// Button

Button::Button(TTF_Font* font, const SDL_Color& fontColor, const SDL_Color& backColor, const std::string& name, const SDL_Rect& bounds) :
	WindowElement(name, bounds) {
	_font = font;
	_fontColor = fontColor;
	_backgroundColor = backColor;
	_text = name;
}

Button::Button(int x, int y, const std::string& name, const std::string& text) : Button(TTF_OpenFont("tahoma.ttf", 54), green, pink,
	name, SDL_Rect{ x, y, 300, 150 }) { 
	_text = text;
};

void Button::RenderElement(SDL_Renderer* const renderer) {

	SDL_Surface* surface = TTF_RenderText_Shaded(_font, _text.c_str(), _fontColor, _backgroundColor);
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

	SDL_RenderCopy(renderer, texture, NULL, &_bounds);

	SDL_FreeSurface(surface);
	SDL_DestroyTexture(texture);

}

Button::~Button() {

}


// Textbox

void TextBox::RenderElement(SDL_Renderer* const renderer) {

	SDL_Surface* surface = TTF_RenderText_Shaded(_font, _text.c_str(), black, white);
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

	_bounds.w = _text.size() * 20;
	_cursorBarRect.x = _bounds.x + _bounds.w;

	SDL_RenderCopy(renderer, texture, NULL, &_bounds);

	SDL_FreeSurface(surface);
	SDL_DestroyTexture(texture);

	if (displayBar) {
		SDL_SetRenderDrawColor(renderer, 0, 255, 0, 100);
		SDL_RenderFillRect(renderer, &_cursorBarRect);
	}

	if (hasFocus && curFrame++ == skipFrames) { displayBar = !displayBar; curFrame = 0; }
	else if (!hasFocus) { displayBar = false; }
}

TextBox::TextBox(TTF_Font* font, const std::string& name, const std::string& text, const SDL_Rect& bounds) :
	WindowElement(name, bounds) {
	_font = font;

	_text = text;
	_cursorBarRect = _bounds;
	_cursorBarRect.w = 10;
	_cursorBarRect.x += ((_bounds.w / 2.0) - (_cursorBarRect.w / 2.0));

	inputValidator = ALPHANUMERIC_INPUT_VALIDATOR;
}

TextBox::TextBox(int x, int y, const std::string& name, const std::string& text) :
	TextBox(TTF_OpenFont("tahoma.ttf", 54), name, text, SDL_Rect{x, y, 300, 100}) {}

TextBox::TextBox(int x, int y, const std::string& name, const std::string& text, InputValidator validator) :
	TextBox(TTF_OpenFont("tahoma.ttf", 54), name, text, SDL_Rect{ x, y, 300, 100 }) {
	
	inputValidator = validator;
}

void TextBox::Update(SDL_Event& ev) {

	if (ev.type == SDL_KEYDOWN) {

		if (ev.key.keysym.sym == SDLK_BACKSPACE) {
			if (_text.size() > 0) {
				_text.pop_back();
			}
		}
		else {
			if (inputValidator(ev.key.keysym.sym)) {
				_text += ev.key.keysym.sym;
			}
		}
	}
}

const std::string& TextBox::Text() const { return _text; }

TextBox::~TextBox() {
	//delete _font;
}

