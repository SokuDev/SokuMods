//
// Created by PinkySmile on 23/02/2021.
//

#ifndef SWRSTOYS_STATE_HPP
#define SWRSTOYS_STATE_HPP


#include <windows.h>
#include <SokuLib.hpp>
#include <SFML/Graphics.hpp>

namespace Practice
{
	struct Settings {
		bool controlDummy = false;
	};

	extern sf::RenderWindow *sfmlWindow;
	extern char profilePath[1024 + MAX_PATH];
	extern char profileParent[1024 + MAX_PATH];
	extern SokuLib::KeyInput dummy;
	extern Settings settings;

	void activate();
	void deactivate();
}


#endif //SWRSTOYS_STATE_HPP
