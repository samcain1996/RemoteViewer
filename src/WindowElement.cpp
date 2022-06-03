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
}

void Button::RenderElement(SDL_Renderer* renderer) {

	SDL_Surface* surface = TTF_RenderText_Shaded(_font, _name.c_str(), _fontColor, _backgroundColor);
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

	SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_NONE);
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
	cursorBarRect.x = _bounds.x + _bounds.w;

	SDL_RenderCopy(renderer, texture, NULL, &_bounds);

	SDL_FreeSurface(surface);
	SDL_DestroyTexture(texture);

	if (displayBar) {
		SDL_SetRenderDrawColor(renderer, 0, 255, 0, 100);
		SDL_RenderFillRect(renderer, &cursorBarRect);
	}

	if (hasFocus) { displayBar = !displayBar; }
}

TextBox::TextBox(TTF_Font* font, const std::string& name, const SDL_Rect& bounds) :
	WindowElement(name, bounds) {
	_font = font;

	_text = "TextBox";
	cursorBarRect = _bounds;
	cursorBarRect.w = 10;
	cursorBarRect.x += ((_bounds.w / 2.0) - (cursorBarRect.w / 2.0));
}
