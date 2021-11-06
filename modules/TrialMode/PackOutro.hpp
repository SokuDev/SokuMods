//
// Created by Gegel85 on 05/11/2021.
//

#ifndef SWRSTOYS_PACKOUTRO_HPP
#define SWRSTOYS_PACKOUTRO_HPP


#include <string>
#include <sol/forward.hpp>
#include <SokuLib.hpp>

class PackOutro {
private:
	sol::state _lua;
	unsigned _currentSprite = 0;
	unsigned _currentCommand = 0;
	std::vector<std::string> _commands;
	SokuLib::DrawUtils::Sprite _background;
	std::vector<std::unique_ptr<SokuLib::DrawUtils::Sprite>> _sprites;

public:
	PackOutro(const std::string &packPath, const std::string &file);
	void draw() const;
	void update();
};


#endif //SWRSTOYS_PACKOUTRO_HPP
