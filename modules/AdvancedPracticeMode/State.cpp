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

#define mkdir _mkdir
#define PAYLOAD_ADDRESS_GET_INPUTS 0x40A45E
#define PAYLOAD_NEXT_INSTR_GET_INPUTS (PAYLOAD_ADDRESS_GET_INPUTS + 4)

namespace Practice
{
	std::map<std::string, std::vector<unsigned short>> characterSpellCards{
		{"alice", {200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211}},
		{"aya", {200, 201, 202, 203, 205, 206, 207, 208, 211, 212}},
		{"chirno", {200, 201, 202, 203, 204, 205, 206, 207, 208, 210, 213}},
		{"iku", {200, 201, 202, 203, 206, 207, 208, 209, 210, 211}},
		{"komachi", {200, 201, 202, 203, 204, 205, 206, 207, 211}},
		{"marisa", {200, 202, 203, 204, 205, 206, 207, 208, 209, 211, 212, 214, 215, 219}},
		{"meirin", {200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 211}},
		{"patchouli", {200, 201, 202, 203, 204, 205, 206, 207, 210, 211, 212, 213}},
		{"reimu", {200, 201, 204, 206, 207, 208, 209, 210, 214, 219}},
		{"remilia", {200, 201, 202, 203, 204, 205, 206, 207, 208, 209}},
		{"sakuya", {200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212}},
		{"sanae", {200, 201, 202, 203, 204, 205, 206, 207, 210}},
		{"suika", {200, 201, 202, 203, 204, 205, 206, 207, 208, 212}},
		{"suwako", {200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 212}},
		{"tenshi", {200, 201, 202, 203, 204, 205, 206, 207, 208, 209}},
		{"udonge", {200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211}},
		{"utsuho", {200, 201, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214}},
		{"youmu", {200, 201, 202, 203, 204, 205, 206, 207, 208, 212}},
		{"yukari", {200, 201, 202, 203, 204, 205, 206, 207, 208, 215}},
		{"yuyuko", {200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 219}}
	};
	void (__stdcall *s_origLoadDeckData)(char *, void *, SokuLib::deckInfo &, int, SokuLib::mVC9Dequeue<short> &);
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

	void __stdcall loadDeckData(char *charName, void *csvFile, SokuLib::deckInfo &deck, int param4, SokuLib::mVC9Dequeue<short> &newDeck)
	{
		bool isPatchy = strcmp(charName, "patchouli") == 0;
		int number = (4 + isPatchy) * 3;
		unsigned short originalDeck[20];

		printf("Save old deck of %i cards\n", min(newDeck.size, 20));
		for (int i = 0; i < min(newDeck.size, 20); i++) {
			originalDeck[i] = newDeck[i];
		}

		printf("Loading deck for %s (%i skill cards)\n", charName, number);
		for (int i = 0; i < number; i++)
			newDeck[i] = 100 + i;
		newDeck.size = number;
		puts("Fake deck generated");
		s_origLoadDeckData(charName, csvFile, deck, param4, newDeck);

		auto &entry = characterSpellCards[charName];

		printf("Loading deck for %s (%i spell cards)\n", charName, entry.size());
		for (int i = 0; i < entry.size(); i++)
			newDeck[i] = entry[i];
		puts("Fake deck generated");
		newDeck.size = entry.size();
		s_origLoadDeckData(charName, csvFile, deck, param4, newDeck);

		puts("Placing old deck back");
		for (int i = 0; i < 20; i++)
			newDeck[i] = originalDeck[i];
		newDeck.size = 20;
		puts("Work done !");
	}

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

		VirtualProtect((PVOID)0x47c5aa, 4, PAGE_EXECUTE_WRITECOPY, &old);
		original_onHit = SokuLib::union_cast<int (SokuLib::CharacterManager::*)(int)>(SokuLib::TamperNearJmpOpr(0x47c5a9, reinterpret_cast<DWORD>(onHit)));
		VirtualProtect((PVOID)0x47c5aa, 4, old, &old);

		//This forces the dummy to have an input device (probably keyboard ?)
		((unsigned *)0x00898680)[1] = 0x008986A8;
	}

	void activate()
	{
		if (sfmlWindow)
			return;

		settings.load();
		puts("Opening window");
		sfmlWindow = new sf::RenderWindow{{640, 480}, "Advanced Practice Mode", sf::Style::Titlebar};
		puts("Window opened");
		Practice::initInputDisplay(profileParent);
		Practice::initBoxDisplay(profileParent);
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

		settings.save();
		delete sfmlWindow;
		sfmlWindow = nullptr;
		removeHooks();
	}

	Settings::Settings(bool activated) :
		nonSaved{activated, false, false, false}
	{
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
			*this = Settings();
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
		if (this->nonSaved.activated)
			this->save();
	}

	void CharacterState::save(bool isLeft) const
	{
		std::string path = "APMSettings/APMLastSession" + std::string(isLeft ? ("Player") : ("Dummy")) + SokuLib::charactersName[this->_chr] + ".dat";

		for (size_t pos = path.find(' '); pos != std::string::npos; pos = path.find(' '))
			path.erase(path.begin() + pos);
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
		std::string path = "APMSettings/APMLastSession" + std::string(isLeft ? ("Player") : ("Dummy")) + SokuLib::charactersName[chr] + ".dat";

		for (size_t pos = path.find(' '); pos != std::string::npos; pos = path.find(' '))
			path.erase(path.begin() + pos);
		std::cout << "Loading character state from " << path << std::endl;

		std::ifstream stream{path};

		if (!stream) {
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

			std::cerr << "Error: Couldn't load file from \"" << path << "\": " << strerror(errno) << std::endl;
			*this = CharacterState();
			this->_chr = chr;
			memcpy(&this->skillMap, &buffer, sizeof(buffer));
			return;
		}
		stream.read(reinterpret_cast<char *>(this), sizeof(*this));
		if (MAGIC_NUMBER_CHR != this->magicNumber) {
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

			std::cerr << "Error: Couldn't load file from \"" << path << "\": ";
			std::cerr << "Magic number 0x" << std::hex << this->magicNumber << " doesn't match expected magic number 0x" << MAGIC_NUMBER_CHR << std::endl;
			*this = CharacterState();
			this->_chr = chr;
			memcpy(&this->skillMap, &buffer, sizeof(buffer));
		}
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

		unsigned magic;
		int i = 0;

		stream.read(reinterpret_cast<char *>(&magic), sizeof(magic));
		if (magic != MAGIC_NUMBER_MACRO) {
			std::cerr << "Error: Couldn't load file from \"" << path << "\": ";
			std::cerr << "Magic number 0x" << std::hex << magic << " doesn't match expected magic number 0x" << MAGIC_NUMBER_MACRO << std::endl;
			return false;
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
			macroArray.reserve(length);
			for (int j = length; j; j--)
				stream >> (macroArray.emplace_back(), macroArray.back());
		}
		return true;
	}

	void MacroManager::save() const
	{
		std::ofstream stream{"APMSettings/APMMacros.dat"};
		unsigned magic = MAGIC_NUMBER_MACRO;
		int i = 0;

		std::cout << "Saving macros to APMSettings/APMMacros.dat" << std::endl;
		if (!stream) {
			std::cerr << "Error: Couldn't save macro file to \"APMSettings/APMMacros.dat\": " << strerror(errno) << std::endl;
			return;
		}

		stream.write(reinterpret_cast<char *>(&magic), sizeof(magic));
		for (auto &macroArray : this->macros) {
			unsigned short length = macroArray.size();

			std::cout << "Macro array for " << SokuLib::charactersName[i++] << " has " << length << " entry/entries at save time" << std::endl;
			stream.write(reinterpret_cast<char *>(&length), sizeof(length));
			for (const auto &macro : macroArray)
				stream << macro;
		}
	}

	bool MacroManager::import(const std::string &path)
	{
		MacroManager alternate;

		if (!alternate.load(path))
			return false;
		for (int i = 0; i < this->macros.size(); i++)
			this->macros[i].insert(this->macros[i].end(), alternate.macros[i].begin(), alternate.macros[i].end());
		return true;
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
		stream.read(reinterpret_cast<char *>(buffer2), length * sizeof(*buffer));
		macro.macroElems = std::vector<std::pair<SokuLib::KeyInput, unsigned>>(buffer2, buffer2 + length);
		delete[] buffer2;
		return stream;
	}
}