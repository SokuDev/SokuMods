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
#define PAYLOAD_ADDRESS_DECK_INFOS 0x437D24
#define PAYLOAD_NEXT_INSTR_DECK_INFOS (PAYLOAD_ADDRESS_DECK_INFOS + 4)

	enum TechDirection {
		NEUTRAL_TECH,
		FORWARD_TECH,
		BACKWARD_TECH,
		RANDOM_TECH
	};

	enum AirTechDirection {
		NO_AIRTECH,
		FORWARD_AIRTECH,
		BACKWARD_AIRTECH,
		RANDOM_AIRTECH
	};

	enum BlockingSide {
		NO_BLOCKING,
		BLOCK,
		HIGH_BLOCKING,
		LOW_BLOCKING,
		RANDOM_HEIGHT_BLOCKING
	};

	struct Settings {
		bool realisticInputs = true;
		bool activated;
		bool controlDummy = false;
		bool forceWeather = true;
		bool weatherResetRequest = false;
		BlockingSide block = NO_BLOCKING;
		TechDirection tech = NEUTRAL_TECH;
		AirTechDirection airtech = NO_AIRTECH;
		float posX = 0;
		float posY = 0;
		bool showLeftInputBox = true;
		bool showRightInputBox = true;

		Settings(bool activated = false) : activated(activated) {};
	};

	extern std::map<std::string, std::vector<unsigned short>> characterSpellCards;
	extern sf::RenderWindow *sfmlWindow;
	extern char profilePath[1024 + MAX_PATH];
	extern char profileParent[1024 + MAX_PATH];
	extern SokuLib::KeyInput lastPlayerInputs;
	extern Settings settings;
	extern void (__stdcall *s_origLoadDeckData)(char *, void *, SokuLib::deckInfo &, int, SokuLib::mVC9Dequeue<short> &);

	void __stdcall loadDeckData(char *charName, void *csvFile, SokuLib::deckInfo &deck, int param4, SokuLib::mVC9Dequeue<short> &newDeck);
	void activate();
	void deactivate();
	void placeHooks();
	void removeHooks();
}


#endif //SWRSTOYS_STATE_HPP
