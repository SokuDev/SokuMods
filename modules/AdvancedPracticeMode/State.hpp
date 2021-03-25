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
#define MAGIC_NUMBER       0x01020305
#define MAGIC_NUMBER_CHR   0x01020304
#define MAGIC_NUMBER_MACRO 0x01020304
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
		RANDOM_HEIGHT_BLOCKING,
		BLOCK_AFTER_FIRST_HIT,
		BLOCK_FIRST_HIT,
	};

	enum Counter {
		COUNTER_OFF,
		COUNTER_ON,
		COUNTER_RANDOM,
	};

	struct CharacterState {
	private:
		SokuLib::Character _chr;

	public:
		unsigned magicNumber = MAGIC_NUMBER_CHR;
		unsigned hp = 10000;
		unsigned maxCurrentSpirit = 1000;
		bool HPInstantRegen = true;
		bool SPInstantRegen = true;
		unsigned brokenOrbs = 0;
		SokuLib::Skill skillMap[15];
		unsigned char dropLevel = 0;
		unsigned char dollLevel = 0;
		unsigned char rodLevel = 0;
		unsigned char grimoireLevel = 0;
		unsigned char fanLevel = 0;
		unsigned char specialValue = 0;

		void save(bool isLeft) const;
		void load(SokuLib::Character chr, bool isLeft);
	};

	struct HitBoxParams {
		bool showPosition = true;
		bool showHitboxes = true;
		bool showHurtboxes = true;
		bool showCollisionBox = true;
		bool showHitProperties = true;
		bool showBuffProperties = true;
		bool showSubObjectPosition = true;
		bool showSubObjectHitboxes = true;
		bool showSubObjectHurtboxes = true;
		bool showSubObjectProperties = true;
	};

	typedef std::vector<std::pair<SokuLib::KeyInput, unsigned>> MacroData;

	struct Macro {
		std::string name;
		MacroData macroElems;
	};

	std::ostream &operator<<(std::ostream &stream, const Macro &macro);
	std::istream &operator>>(std::istream &stream, Macro &macro);

	struct MacroManager {
		std::array<std::vector<Macro>, 20> macros;

		void save() const;
		void load();
		bool load(const std::string &path);
		bool import(const std::string &path);
	};

	struct Settings {
		unsigned magicNumber = MAGIC_NUMBER;
		bool realisticInputs = true;
		bool controlDummy = false;
		bool forceWeather = true;
		bool weatherResetRequest = false;
		int weatherTime = 999;
		int requestedFrameRate = 60;
		Counter counter = COUNTER_OFF;
		BlockingSide block = NO_BLOCKING;
		TechDirection tech = NEUTRAL_TECH;
		AirTechDirection airtech = NO_AIRTECH;
		float posX = 0;
		float posY = 0;
		bool showRawInputs = false;
		bool showEmptyInputs = false;
		bool showLeftInputBox = true;
		bool showRightInputBox = true;
		bool showLeftJoypad = true;
		bool showRightJoypad = true;
		HitBoxParams leftHitboxSettings;
		HitBoxParams rightHitboxSettings;
		struct NonSavedElements{
			bool activated;
			bool recordingStarted : 1;
			bool recordingMacro : 1;
			bool recordForDummy : 1;
			MacroData *recordBuffer = nullptr;
			CharacterState leftState;
			CharacterState rightState;
			MacroManager macros;
		} nonSaved;

		Settings(bool activated = false);
		void load();
		void save() const;
		~Settings();
	};

	extern int idleCounter;
	extern int dummyHitCounter;
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
