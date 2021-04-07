//
// Created by PinkySmile on 23/02/2021.
//

#ifndef SWRSTOYS_STATE_HPP
#define SWRSTOYS_STATE_HPP


#include <windows.h>
#include <SokuLib.hpp>
#include <SFML/Graphics.hpp>
#include <list>

namespace Practice
{
#define MAGIC_NUMBER                       0x01020307
#define MAGIC_NUMBER_CHR                   0x01020304
#define MAGIC_NUMBER_MACRO                 0x01020304
#define MAGIC_NUMBER_MACRO_SINGLE_MACRO    0x11121314
#define MAGIC_NUMBER_MACRO_WHOLE_CHARACTER 0x21222324

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
		bool showPosition : 1;
		bool showHitboxes : 1;
		bool showHurtboxes : 1;
		bool showCollisionBox : 1;
		bool showHitProperties : 1;
		bool showBuffProperties : 1;
		bool showSubObjectPosition : 1;
		bool showSubObjectHitboxes : 1;
		bool showSubObjectHurtboxes : 1;
		bool showSubObjectProperties : 1;

		HitBoxParams() {
			this->showPosition = true;
			this->showHitboxes = true;
			this->showHurtboxes = true;
			this->showCollisionBox = true;
			this->showHitProperties = true;
			this->showBuffProperties = true;
			this->showSubObjectPosition = true;
			this->showSubObjectHitboxes = true;
			this->showSubObjectHurtboxes = true;
			this->showSubObjectProperties = true;
		}
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
		bool save(const std::string &path) const;
		bool save(std::ofstream &stream, const std::string &path) const;
		void load();
		bool load(const std::string &path);
		bool load(std::ifstream &stream, const std::string &path = "", bool needMagic = true);
		void import(const MacroManager &other);
	};

	struct Settings {
		unsigned magicNumber = MAGIC_NUMBER;
		int weatherTime = 999;
		int requestedFrameRate = 60;
		Counter counter = COUNTER_OFF;
		BlockingSide block = NO_BLOCKING;
		TechDirection tech = NEUTRAL_TECH;
		AirTechDirection airtech = NO_AIRTECH;
		float posX = 0;
		float posY = 0;
		bool showRawInputs : 1;
		bool showEmptyInputs : 1;
		bool showLeftInputBox : 1;
		bool showRightInputBox : 1;
		bool showLeftJoypad : 1;
		bool showRightJoypad : 1;
		bool realisticInputs : 1;
		bool forceWeather : 1;
		bool weatherResetRequest : 1;
		bool showUntech : 1;
		bool showBlockstun : 1;
		bool showHitstun : 1;
		bool showGaps : 1;
		bool showFrameAdvantage : 1;
		HitBoxParams leftHitboxSettings;
		HitBoxParams rightHitboxSettings;
		struct NonSavedElements{
			bool activated : 1;
			bool recordingStarted : 1;
			bool recordingMacro : 1;
			bool recordForDummy : 1;
			bool loopMacros : 1;
			bool playingMacro : 1;
			bool controlDummy : 1;
			std::list<Macro> playList;
			Macro playingMacroBuffer;
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
