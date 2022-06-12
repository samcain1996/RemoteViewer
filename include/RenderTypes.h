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
	~FontPool();

	const FontRef FindFont(const std::string& fontName);

};
