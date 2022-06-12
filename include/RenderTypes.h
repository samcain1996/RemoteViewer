#pragma once
#include "Packet.h"
#include "Messages.h"
#include <map>

using FontRef = std::reference_wrapper<TTF_Font* const>;
using FontPtr = std::unique_ptr<TTF_Font*>;

struct FontPool {
	
private:
	std::map<TTF_Font*, std::string> _fonts;
	FontPtr _defaultFont;
	
public:

	FontPool() { 

		if (!SDL_WasInit(NULL)) {
			SDL_Init(SDL_INIT_EVERYTHING);

			if (!TTF_WasInit() && TTF_Init() == -1) {
				printf("TTF_Init: %s\n", TTF_GetError());
				exit(1);
			}
		}
		

		_defaultFont = std::make_unique<TTF_Font*>(TTF_OpenFont("tahoma.ttf", 54));

	}

	~FontPool() {
		for (auto& font : _fonts) {
			TTF_CloseFont(font.first);
		}
	}

	const FontRef FindFont(const std::string& fontName)  {
		
		const FontRef fallbackFont = *_defaultFont;

		// Good one copilot
		std::string fontNameLower(fontName);
		std::transform(fontNameLower.begin(), fontNameLower.end(), fontNameLower.begin(), ::tolower);

		if (fontNameLower == "default") { return fallbackFont; }

		for (auto& [font, name] : _fonts) {

			if (name == fontNameLower) {
				return std::ref(font);
			}
		}
		
		
		// WOW, suprised copilot got this
		std::string pureName = fontNameLower.substr(0, fontNameLower.find('.'));

		TTF_Font* font = TTF_OpenFont(std::string(pureName + ".ttf").c_str(), 54);
		
		if (font == nullptr) { return fallbackFont; }
		
		_fonts.insert(std::make_pair(font, pureName));

		return std::ref(font);
		
	}
	

};
