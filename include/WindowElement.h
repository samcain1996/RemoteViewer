#pragma once
#include "Packet.h"
#include "Messages.h"

class ElementManager;

class WindowElement {
	friend ElementManager;

	friend bool operator==(const WindowElement& we1, const WindowElement& we2) {
		return we1._elementId == we2._elementId;
	}

private:
	int _elementId;

protected:
	std::string _name;
	SDL_Rect _bounds;

	virtual void RenderElement(SDL_Surface* surface, SDL_Texture* texture, SDL_Renderer* renderer) = 0;

public:

	WindowElement();
	WindowElement(const std::string& name, const SDL_Rect& rect);

	WindowElement(WindowElement&&) = delete;
	WindowElement(const WindowElement&) = delete;

	WindowElement& operator=(const WindowElement& other) = delete;
	WindowElement& operator=(WindowElement&& other) noexcept;

	const std::string& Name() const;
	const SDL_Rect& Bounds() const;

};


class Button : public WindowElement {
	friend class ElementManager;
	friend class InitWindow;
private:
	TTF_Font* _font;
	SDL_Color _fontColor;
	Uint32 _backgroundColor;

	void RenderElement(SDL_Surface* surface, SDL_Texture* texture, SDL_Renderer* renderer) override;
public:
	Button(TTF_Font* font, const SDL_Color& fontColor, const std::string& name, SDL_Rect& bounds);
};