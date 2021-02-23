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
	enum TechDirection {
		NEUTRAL_TECH,
		FORWARD_TECH,
		BACKWARD_TECH,
		RANDOM_TECH,
	};

	enum AirTechDirection {
		NO_AIRTECH,
		FORWARD_AIRTECH,
		BACKWARD_AIRTECH,
		RANDOM_AIRTECH,
	};

	struct Settings {
		bool activated;
		bool controlDummy = false;
		TechDirection tech = NEUTRAL_TECH;
		AirTechDirection airtech = NO_AIRTECH;
		float posX = 0;
		float posY = 0;

		Settings(bool activated = false) : activated(activated) {};
	};

	extern sf::RenderWindow *sfmlWindow;
	extern char profilePath[1024 + MAX_PATH];
	extern char profileParent[1024 + MAX_PATH];
	extern SokuLib::KeyInput lastPlayerInputs;
	extern Settings settings;

	void activate();
	void deactivate();
	void placeHooks();
	void removeHooks();
}


#endif //SWRSTOYS_STATE_HPP
