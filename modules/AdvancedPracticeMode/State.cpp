//
// Created by PinkySmile on 23/02/2021.
//

#include <fstream>
#include <iostream>
#include <direct.h>
#include "State.hpp"
#include "Gui.hpp"
#include "Logic.hpp"
#include "Inputs.hpp"
#include "Hitboxes.hpp"
#include "Dummy.hpp"
#include "Gap.hpp"

#define mkdir _mkdir
#define PAYLOAD_ADDRESS_GET_INPUTS 0x40A45E
#define PAYLOAD_NEXT_INSTR_GET_INPUTS (PAYLOAD_ADDRESS_GET_INPUTS + 4)

namespace Practice
{
	std::map<SokuLib::Character, std::map<unsigned short, SokuLib::DrawUtils::Sprite>> cardsTextures;
	std::map<SokuLib::Character, CharacterInfo> characterInfos{
		{ SokuLib::CHARACTER_REIMU,     { SokuLib::CHARACTER_REIMU,     "reimu",     "Reimu",     "Reimu Hakurei",          {"236", "214", "421", "623"} }},
		{ SokuLib::CHARACTER_MARISA,    { SokuLib::CHARACTER_MARISA,    "marisa",    "Marisa",    "Marisa Kirisame",        {"214", "623", "22",  "236"} }},
		{ SokuLib::CHARACTER_SAKUYA,    { SokuLib::CHARACTER_SAKUYA,    "sakuya",    "Sakuya",    "Sakuya Izayoi",          {"623", "214", "236", "22" } }},
		{ SokuLib::CHARACTER_ALICE,     { SokuLib::CHARACTER_ALICE,     "alice",     "Alice",     "Alice Margatroid",       {"236", "623", "214", "22" } }},
		{ SokuLib::CHARACTER_PATCHOULI, { SokuLib::CHARACTER_PATCHOULI, "patchouli", "Patchouli", "Patchouli Knowledge",    {"236", "22",  "623", "214", "421"} }},
		{ SokuLib::CHARACTER_YOUMU,     { SokuLib::CHARACTER_YOUMU,     "youmu",     "Youmu",     "Youmu Konpaku",          {"236", "623", "214", "22" } }},
		{ SokuLib::CHARACTER_REMILIA,   { SokuLib::CHARACTER_REMILIA,   "remilia",   "Remilia",   "Remilia Scarlet",        {"236", "214", "623", "22" } }},
		{ SokuLib::CHARACTER_YUYUKO,    { SokuLib::CHARACTER_YUYUKO,    "yuyuko",    "Yuyuko",    "Yuyuko Saigyouji",       {"214", "236", "421", "623"} }},
		{ SokuLib::CHARACTER_YUKARI,    { SokuLib::CHARACTER_YUKARI,    "yukari",    "Yukari",    "Yukari Yakumo",          {"236", "623", "214", "421"} }},
		{ SokuLib::CHARACTER_SUIKA,     { SokuLib::CHARACTER_SUIKA,     "suika",     "Suika",     "Suika Ibuki",            {"236", "623", "214", "22" } }},
		{ SokuLib::CHARACTER_REISEN,    { SokuLib::CHARACTER_REISEN,    "udonge",    "Reisen",    "Reisen Undongein Inaba", {"236", "214", "623", "22" } }},
		{ SokuLib::CHARACTER_AYA,       { SokuLib::CHARACTER_AYA,       "aya",       "Aya",       "Aya Shameimaru",         {"236", "214", "22",  "421"} }},
		{ SokuLib::CHARACTER_KOMACHI,   { SokuLib::CHARACTER_KOMACHI,   "komachi",   "Komachi",   "Komachi Onozuka",        {"236", "623", "22",  "214"} }},
		{ SokuLib::CHARACTER_IKU,       { SokuLib::CHARACTER_IKU,       "iku",       "Iku",       "Iku Nagae",              {"236", "623", "22",  "214"} }},
		{ SokuLib::CHARACTER_TENSHI,    { SokuLib::CHARACTER_TENSHI,    "tenshi",    "Tenshi",    "Tenshi Hinanawi",        {"214", "22",  "236", "623"} }},
		{ SokuLib::CHARACTER_SANAE,     { SokuLib::CHARACTER_SANAE,     "sanae",     "Sanae",     "Sanae Kochiya",          {"236", "22",  "623", "214"} }},
		{ SokuLib::CHARACTER_CIRNO,     { SokuLib::CHARACTER_CIRNO,     "chirno",    "Cirno",     "Cirno",                  {"236", "214", "22",  "623"} }},
		{ SokuLib::CHARACTER_MEILING,   { SokuLib::CHARACTER_MEILING,   "meirin",    "Meiling",   "Hong Meiling",           {"214", "623", "22",  "236"} }},
		{ SokuLib::CHARACTER_UTSUHO,    { SokuLib::CHARACTER_UTSUHO,    "utsuho",    "Utsuho",    "Utsuho Reiuji",          {"623", "236", "22",  "214"} }},
		{ SokuLib::CHARACTER_SUWAKO,    { SokuLib::CHARACTER_SUWAKO,    "suwako",    "Suwako",    "Suwako Moriya",          {"214", "623", "236", "22" } }}
	};
	static bool loaded = false;
	void (__stdcall *s_origLoadDeckData)(char *, void *, SokuLib::DeckInfo &, int, SokuLib::Dequeue<short> &);
	static void (SokuLib::KeymapManager::*s_origKeymapManager_SetInputs)();
	static const unsigned char patchCode[] = {
		0x31, 0xED,                  //xor ebp, ebp
		0xE9, 0x91, 0x01, 0x00, 0x00 //jmp pc+00000191
	};
	static unsigned char originalCode[sizeof(patchCode)];

	void __fastcall KeymapManagerSetInputs(SokuLib::KeymapManager *This)
	{
		(This->*s_origKeymapManager_SetInputs)();
		Practice::handleInput(*This);
	}

	static void loadAllExistingCards()
	{
		char buffer[] = "data/csv/000000000000/spellcard.csv";
		char bufferCards[] = "data/card/000000000000/card000.bmp";

		puts("Loading cards");
		for (auto& [id, info] : characterInfos) {
			sprintf(buffer, "data/csv/%s/spellcard.csv", info.codeName.c_str());
			printf("Loading cards from %s\n", buffer);

			SokuLib::CSVParser parser{buffer};
			int i = 0;

			do {
				auto str = parser.getNextCell();

				i++;
				try {
					auto str2 = parser.getNextCell();

					printf("New card %s: %s\n", str.c_str(), str2.c_str());

					auto cardId = std::stoul(str);
					auto &entry = info.cards[cardId];
					auto &sprite = cardsTextures[id][cardId];

					sprintf(bufferCards, "data/card/%s/card%03lu.bmp", info.codeName.c_str(), cardId);
					entry.id = cardId;
					entry.name = str2;
					sprite.texture.loadFromGame(bufferCards);
				} catch (std::exception &e) {
					MessageBoxA(
						SokuLib::window,
						(
							"Fatal error: Cannot load cards list for " + info.fullName + ":\n" +
							"In file " + buffer + ": Cannot parse cell #1 at line #" + std::to_string(i) +
							" \"" + str + "\": " + e.what()
						).c_str(),
						"Loading code infos failed",
						MB_ICONERROR
					);
					abort();
				}
			} while (parser.goToNextLine());
		}
	}

	void __stdcall loadDeckData(char *charName, void *csvFile, SokuLib::DeckInfo &deck, int param4, SokuLib::Dequeue<short> &newDeck)
	{
		unsigned short originalDeck[20];

		if (newDeck.size != 20) {
			MessageBoxA(
				SokuLib::window,
				(
					"Your deck must contain 20 cards (but has " + std::to_string(newDeck.size) + ")."
					"You won't be able to give yourself cards that are not in your deck."
				).c_str(),
				"Invalid deck size",
				MB_ICONWARNING
			);
			s_origLoadDeckData(charName, csvFile, deck, param4, newDeck);
			return;
		}
		if (characterInfos[SokuLib::CHARACTER_REIMU].cards.empty())
			loadAllExistingCards();
		printf("Save old deck of %i cards\n", newDeck.size);
		for (int i = 0; i < 20; i++)
			originalDeck[i] = newDeck[i];

		auto &entry = std::find_if(characterInfos.begin(), characterInfos.end(), [charName](const auto &m) { return m.second.codeName == charName; })->second.cards;

		printf("Loading deck for %s (%i total cards so %0.f fake decks)\n", charName, entry.size(), std::ceil(entry.size() / 20.f));
		for (int j = 0; j < entry.size(); j += 20) {
			auto size = j + 20 < entry.size() ? 20 : entry.size() - j;
			auto it = entry.begin();

			printf("Generate deck of %i cards\n", size);
			for (int i = 0; i < size; i++) {
				newDeck[i] = it->first;
				it++;
			}
			newDeck.size = size;
			puts("Fake deck generated");
			s_origLoadDeckData(charName, csvFile, deck, param4, newDeck);
		}

		puts("Placing old deck back");
		for (int i = 0; i < 20; i++)
			newDeck[i] = originalDeck[i];
		newDeck.size = 20;
		s_origLoadDeckData(charName, csvFile, deck, param4, newDeck);
		puts("Work done !");
	}

	wchar_t soku2Path[1024 + MAX_PATH];
	Settings settings;
	sf::RenderWindow *sfmlWindow;
	char profilePath[1024 + MAX_PATH];
	char profileParent[1024 + MAX_PATH];
	SokuLib::KeyInput lastPlayerInputs;
	int idleCounter = 0;
	int dummyHitCounter = 0;
	static int (SokuLib::CharacterManager::*original_onHit)(int param);

	static int __fastcall onHit(SokuLib::CharacterManager &This, int, int param)
	{
		if (&This == &SokuLib::getBattleMgr().rightCharacterManager)
			dummyBeforeHit((unsigned char)~param >> 1 & 1);

		return (This.*original_onHit)(param);
	}

	void placeHooks()
	{
		DWORD old;
		int newOffset;

		if (settings.nonSaved.activated)
			return;
		puts("Placing hooks");
		settings.nonSaved.activated = true;
		//Bypass the basic practice features by skipping most of the function.
		VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
		for (unsigned i = 0; i < sizeof(patchCode); i++) {
			originalCode[i] = ((unsigned char *)0x42A331)[i];
			((unsigned char *)0x42A331)[i] = patchCode[i];
		}

		newOffset = (int)KeymapManagerSetInputs - PAYLOAD_NEXT_INSTR_GET_INPUTS;
		s_origKeymapManager_SetInputs = SokuLib::union_cast<void (SokuLib::KeymapManager::*)()>(*(int *)PAYLOAD_ADDRESS_GET_INPUTS + PAYLOAD_NEXT_INSTR_GET_INPUTS);
		*(int *)PAYLOAD_ADDRESS_GET_INPUTS = newOffset;
		VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, old, &old);

		VirtualProtect((PVOID)0x47c5aa, 4, PAGE_EXECUTE_WRITECOPY, &old);
		original_onHit = SokuLib::union_cast<int (SokuLib::CharacterManager::*)(int)>(SokuLib::TamperNearJmpOpr(0x47c5a9, reinterpret_cast<DWORD>(onHit)));
		VirtualProtect((PVOID)0x47c5aa, 4, old, &old);

		//This forces the dummy to have an input device (probably keyboard ?)
		if (SokuLib::mainMode == SokuLib::BATTLE_MODE_PRACTICE)
			((unsigned *)0x00898680)[1] = 0x008986A8;
	}

	void activate()
	{
		if (sfmlWindow)
			return;

		if (settings.nonSaved.enabled)
			settings.reset();
		else
			settings.load();
		puts("Opening window");
		sfmlWindow = new sf::RenderWindow{{640, 480}, "Advanced Practice Mode", sf::Style::Titlebar};
		puts("Window opened");
		Practice::initInputDisplay(profileParent);
		Practice::initBoxDisplay(profileParent);
		Practice::initGap(profileParent);
		Practice::init(profileParent);
		Practice::gui.setTarget(*sfmlWindow);
		try {
			Practice::loadAllGuiElements(profileParent);
		} catch (std::exception &e) {
			puts(e.what());
			throw;
		}
		placeHooks();
	}

	void removeHooks()
	{
		DWORD old;

		if (!settings.nonSaved.activated)
			return;
		puts("Removing hooks");
		settings.nonSaved.activated = false;
		VirtualProtect((PVOID)0x42A331, sizeof(patchCode), PAGE_EXECUTE_WRITECOPY, &old);
		for (unsigned i = 0; i < sizeof(patchCode); i++)
			((unsigned char *)0x42A331)[i] = originalCode[i];
		VirtualProtect((PVOID)0x42A331, sizeof(patchCode), old, &old);

		VirtualProtect((PVOID)0x47c5aa, 4, PAGE_EXECUTE_WRITECOPY, &old);
		SokuLib::TamperNearJmpOpr(0x47c5a9, SokuLib::union_cast<DWORD>(original_onHit));
		VirtualProtect((PVOID)0x47c5aa, 4, old, &old);

		VirtualProtect((PVOID)PAYLOAD_ADDRESS_GET_INPUTS, 4, PAGE_EXECUTE_WRITECOPY, &old);
		*(int *)PAYLOAD_ADDRESS_GET_INPUTS = SokuLib::union_cast<int>(s_origKeymapManager_SetInputs) - PAYLOAD_NEXT_INSTR_GET_INPUTS;
		VirtualProtect((PVOID)PAYLOAD_ADDRESS_GET_INPUTS, 4, old, &old);
	}

	void deactivate()
	{
		if (!sfmlWindow)
			return;

		if (!settings.nonSaved.enabled)
			settings.save();
		delete sfmlWindow;
		sfmlWindow = nullptr;
		removeHooks();
	}

	Settings::Settings(bool activated, bool enabled) :
		nonSaved{activated, false, false, false, false, false, false, enabled}
	{
		this->showRawInputs = false;
		this->showEmptyInputs = false;
		this->showLeftInputBox = true;
		this->showRightInputBox = true;
		this->showLeftJoypad = true;
		this->showRightJoypad = true;
		this->realisticInputs = true;
		this->forceWeather = true;
		this->weatherResetRequest = false;
		this->showUntech = true;
		this->showBlockstun = true;
		this->showHitstun = true;
		this->showGaps = true;
		this->showFrameAdvantage = true;
	}

	void Settings::load()
	{
		std::cout << "Loading settings from APMSettings/APMLastSession.dat" << std::endl;

		std::ifstream stream{"APMSettings/APMLastSession.dat"};

		if (!stream) {
			std::cerr << "Error: Couldn't load settings from \"APMSettings/APMLastSession.dat\": " << strerror(errno) << std::endl;
			this->nonSaved.leftState.load(SokuLib::leftChar, true);
			this->nonSaved.rightState.load(SokuLib::rightChar, false);
			return;
		}
		stream.read(reinterpret_cast<char *>(this), sizeof(*this) - sizeof(NonSavedElements));
		if (MAGIC_NUMBER != this->magicNumber) {
			std::cerr << "Error: Couldn't load settings from \"APMSettings/APMLastSession.dat\": ";
			std::cerr << "Magic number 0x" << std::hex << this->magicNumber << " doesn't match expected magic number 0x" << MAGIC_NUMBER << std::endl;
			*this = Settings(this->nonSaved.activated, this->nonSaved.enabled);
		}
		this->nonSaved.leftState.load(SokuLib::leftChar, true);
		this->nonSaved.rightState.load(SokuLib::rightChar, false);
		this->nonSaved.macros.load();
	}

	void Settings::save() const
	{
		std::cout << "Saving settings to APMSettings/APMLastSession.dat" << std::endl;
		mkdir("APMSettings");

		std::ofstream stream{"APMSettings/APMLastSession.dat"};

		if (!stream) {
			std::cerr << "Error: Couldn't save settings to \"APMSettings/APMLastSession.dat\": " << strerror(errno) << std::endl;
			MessageBoxA(SokuLib::window, ("Error: Couldn't save file to \"APMSettings/APMLastSession.dat\": " + std::string(strerror(errno))).c_str(), "Cannot save settings.", MB_ICONERROR);
			return;
		}
		stream.write(reinterpret_cast<const char *>(this), sizeof(*this) - sizeof(NonSavedElements));
		this->nonSaved.leftState.save(true);
		this->nonSaved.rightState.save(false);
		this->nonSaved.macros.save();
	}

	Settings::~Settings()
	{
		if (this->nonSaved.activated && !this->nonSaved.enabled)
			this->save();
	}

	void Settings::reset()
	{
		*this = Settings(this->nonSaved.activated, this->nonSaved.enabled);
		this->nonSaved.leftState  = CharacterState(SokuLib::leftChar);
		this->nonSaved.rightState = CharacterState(SokuLib::rightChar);
	}

	void CharacterState::save(bool isLeft) const
	{
		std::string path = "APMSettings/APMLastSession" + std::string(isLeft ? ("Player") : ("Dummy")) + characterInfos[this->_chr].shortName + ".dat";

		//for (size_t pos = path.find(' '); pos != std::string::npos; pos = path.find(' '))
		//	path.erase(path.begin() + pos);
		std::cout << "Saving character state to " << path << std::endl;

		std::ofstream stream{path};

		if (!stream) {
			std::cerr << "Error: Couldn't save file from \"" << path << "\": " << strerror(errno) << std::endl;
			MessageBoxA(SokuLib::window, ("Error: Couldn't save file to \"" + path + "\": " + strerror(errno)).c_str(), "Cannot save settings.", MB_ICONERROR);
			return;
		}
		stream.write(reinterpret_cast<const char *>(this), sizeof(*this));
	}

	void CharacterState::load(SokuLib::Character chr, bool isLeft)
	{
		this->_chr = chr;
		std::string path = "APMSettings/APMLastSession" + std::string(isLeft ? ("Player") : ("Dummy")) + characterInfos[chr].shortName + ".dat";

		//for (size_t pos = path.find(' '); pos != std::string::npos; pos = path.find(' '))
		//	path.erase(path.begin() + pos);
		std::cout << "Loading character state from " << path << std::endl;

		std::ifstream stream{path};

		if (!stream) {
			std::cerr << "Error: Couldn't load file from \"" << path << "\": " << strerror(errno) << std::endl;
			*this = CharacterState(chr);
			return;
		}
		stream.read(reinterpret_cast<char *>(this), sizeof(*this));
		if (MAGIC_NUMBER_CHR != this->magicNumber) {
			std::cerr << "Error: Couldn't load file from \"" << path << "\": ";
			std::cerr << "Magic number 0x" << std::hex << this->magicNumber << " doesn't match expected magic number 0x" << MAGIC_NUMBER_CHR << std::endl;
			*this = CharacterState(chr);
		}
	}

	CharacterState::CharacterState(SokuLib::Character chr)
	{
		SokuLib::Skill buffer[15] = {
			{0x00, false},
			{0x00, false},
			{0x00, false},
			{0x00, false},
			{
			 static_cast<unsigned char>((chr != SokuLib::CHARACTER_PATCHOULI) * 0x7F),
			       chr != SokuLib::CHARACTER_PATCHOULI
			},
			{0x7F, true},
			{0x7F, true},
			{0x7F, true},
			{0x7F, true},
			{0x7F, true},
			{0x7F, true},
			{0x7F, true},
			{0x7F, true},
			{0x7F, true},
			{0x7F, true},
		};

		this->_chr = chr;
		memcpy(&this->skillMap, &buffer, sizeof(buffer));
	}

	void MacroManager::load()
	{
		this->load("APMSettings/APMMacros.dat");
	}

	bool MacroManager::load(const std::string &path)
	{
		std::ifstream stream{path};

		std::cout << "Loading macros from " << path << std::endl;
		if (!stream) {
			std::cerr << "Error: Couldn't load macro file from \"" << path << "\": " << strerror(errno) << std::endl;
			return false;
		}

		return this->load(stream, path);
	}

	bool MacroManager::load(std::ifstream &stream, const std::string &path, bool needMagic)
	{
		unsigned magic;
		int i = 0;

		if (needMagic) {
			stream.read(reinterpret_cast<char *>(&magic), sizeof(magic));
			if (magic != MAGIC_NUMBER_MACRO) {
				std::cerr << "Error: Couldn't load file from \"" << path << "\": ";
				std::cerr << "Magic number 0x" << std::hex << magic << " doesn't match expected magic number 0x" << MAGIC_NUMBER_MACRO << std::endl;
				return false;
			}
		}

		for (auto &macroArray : this->macros) {
			unsigned short length;

			stream.read(reinterpret_cast<char *>(&length), sizeof(length));
			std::cout << "Macro array for " << SokuLib::charactersName[i++] << " has " << length << " entry/entries at load time" << std::endl;
			if (length >= 4096) {
				MessageBoxA(
					SokuLib::window,
					("There are more than 4096 macros entry (" +
					 std::to_string(length) +
					 ").\nThe macro file might be corrupted. The mod might crash if you have too many macros.\n"
					 "Please remove unused macros.\nDo you want to remove the file ?").c_str(),
					"Suspicious amount of macros in file.", MB_ICONWARNING | MB_YESNO
				);
				//TODO: Actually remove the file
			}
			macroArray.clear();
			macroArray.reserve(length);
			for (int j = length; j; j--)
				stream >> (macroArray.emplace_back(), macroArray.back());
		}
		return true;
	}

	void MacroManager::save() const
	{
		this->save("APMSettings/APMMacros.dat");
	}

	bool MacroManager::save(const std::string &path) const
	{
		std::ofstream stream{path};

		std::cout << "Saving macros to \"" << path << '"' << std::endl;
		if (!stream) {
			std::cerr << "Error: Couldn't save macro file to \"" << path << "\": " << strerror(errno) << std::endl;
			return false;
		}
		return this->save(stream, path);
	}

	bool MacroManager::save(std::ofstream &stream, const std::string &path) const
	{
		unsigned magic = MAGIC_NUMBER_MACRO;
		int i = 0;

		stream.write(reinterpret_cast<char *>(&magic), sizeof(magic));
		for (auto &macroArray : this->macros) {
			unsigned short length = macroArray.size();

			std::cout << "Macro array for " << SokuLib::charactersName[i++] << " has " << length << " entry/entries at save time" << std::endl;
			stream.write(reinterpret_cast<char *>(&length), sizeof(length));
			for (const auto &macro : macroArray)
				stream << macro;
		}
		return !stream.fail();
	}

	void MacroManager::import(const MacroManager &alternate)
	{
		for (int i = 0; i < this->macros.size(); i++)
			this->macros[i].insert(this->macros[i].end(), alternate.macros[i].begin(), alternate.macros[i].end());
	}

	std::ostream &operator<<(std::ostream &stream, const Macro &macro)
	{
		unsigned short length;

		length = macro.name.size();
		stream.write(reinterpret_cast<char *>(&length), sizeof(length));
		stream.write(macro.name.c_str(), length);

		length = macro.macroElems.size();
		stream.write(reinterpret_cast<char *>(&length), sizeof(length));
		stream.write(reinterpret_cast<const char *>(macro.macroElems.data()), length * sizeof(macro.macroElems[0]));
		return stream;
	}

	std::istream &operator>>(std::istream &stream, Macro &macro)
	{
		unsigned short length;
		char *buffer;
		std::pair<SokuLib::KeyInput, unsigned> *buffer2;

		stream.read(reinterpret_cast<char *>(&length), sizeof(length));
		buffer = new char[length];
		stream.read(buffer, length);
		macro.name = std::string(buffer, buffer + length);
		delete[] buffer;

		stream.read(reinterpret_cast<char *>(&length), sizeof(length));
		buffer2 = new std::pair<SokuLib::KeyInput, unsigned>[length];
		stream.read(reinterpret_cast<char *>(buffer2), length * sizeof(*buffer2));
		macro.macroElems = std::vector<std::pair<SokuLib::KeyInput, unsigned>>(buffer2, buffer2 + length);
		delete[] buffer2;
		return stream;
	}
}