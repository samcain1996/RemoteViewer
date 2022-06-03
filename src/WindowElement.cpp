#include "WindowElement.h"


// Window Element

WindowElement::WindowElement(const std::string& name, const SDL_Rect& rect) :
	_name(name), _bounds(rect) {};

WindowElement::WindowElement() : WindowElement("ERROR", SDL_Rect()) {}

WindowElement& WindowElement::operator=(WindowElement&& other) noexcept {
	_name = std::move(other._name);
	_bounds = std::move(other._bounds);

	return *this;
}

const std::string& WindowElement::Name() const { return _name; }
const SDL_Rect& WindowElement::Bounds() const { return _bounds; }


// Button
void Button::RenderElement(SDL_Surface* surface, SDL_Texture* texture, SDL_Renderer* renderer) {

	SDL_FillRect(surface, &_bounds, _backgroundColor);
	texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_RenderCopy(renderer, texture, NULL, NULL);

	SDL_FreeSurface(surface);
	SDL_DestroyTexture(texture);

	surface = TTF_RenderText_Solid(_font, _name.c_str(), _fontColor);
	texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_NONE);
	SDL_RenderCopy(renderer, texture, NULL, &_bounds);

	SDL_FreeSurface(surface);
	SDL_DestroyTexture(texture);

}

Button::Button(TTF_Font* font, const SDL_Color& fontColor, const std::string& name, SDL_Rect& bounds) :
	WindowElement(name, bounds) {
	_font = font;
	_fontColor = fontColor;
	_backgroundColor = 0xff00ff00;
}

