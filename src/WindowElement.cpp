#include "WindowElement.h"


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

	_text = _name;
}

Button::Button(int x, int y, const std::string& name, const std::string& text) : 
	Button(TTF_OpenFont("tahoma.ttf", 54), SDL_Color{ 0, 255, 0 }, SDL_Color{ 255,0,255 }, std::to_string(GetNextId()),
	SDL_Rect{ x, y, 300, 100 }) {
	_name = name;
	_text = text;
}

void Button::RenderElement(SDL_Renderer* renderer) {

	SDL_Surface* surface = TTF_RenderText_Shaded(_font, _text.c_str(), _fontColor, _backgroundColor);
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

	SDL_RenderCopy(renderer, texture, NULL, &_bounds);

	SDL_FreeSurface(surface);
	SDL_DestroyTexture(texture);

}

// Textbox
void TextBox::RenderElement(SDL_Renderer* renderer) {

	SDL_Color white = { 255, 255, 255 };
	SDL_Color black = { 0,0,0 };

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
}

TextBox::TextBox(TTF_Font* font, const std::string& name, const std::string& text, const SDL_Rect& bounds) :
	WindowElement(name, bounds) {
	_font = font;

	_text = text;
	_cursorBarRect = _bounds;
	_cursorBarRect.w = 10;
	_cursorBarRect.x += ((_bounds.w / 2.0) - (_cursorBarRect.w / 2.0));
}

TextBox::TextBox(int x, int y, const std::string& name, const std::string& text) :
	TextBox(TTF_OpenFont("tahoma.ttf", 54), name, text, SDL_Rect{x, y, 300, 100}) {}

void TextBox::Update(SDL_Event& ev) {

	if (ev.type == SDL_KEYDOWN) {

		if (ev.key.keysym.sym == SDLK_BACKSPACE) {
			if (_text.size() > 0) {
				_text.pop_back();
			}
		}
		else {
			_text += ev.key.keysym.sym;
		}
	}
}

const std::string& TextBox::Text() const { return _text; }

