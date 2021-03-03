//
// Created by PinkySmile on 23/02/2021.
//

#include "State.hpp"
#include "Gui.hpp"
#include "Logic.hpp"
#include "Inputs.hpp"

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

	void placeHooks()
	{
		DWORD old;
		int newOffset;

		if (settings.activated)
			return;
		puts("Placing hooks");
		settings.activated = true;
		//Bypass the basic practice features by skipping most of the function.
		VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
		for (unsigned i = 0; i < sizeof(patchCode); i++) {
			originalCode[i] = ((unsigned char *)0x42A331)[i];
			((unsigned char *)0x42A331)[i] = patchCode[i];
		}

		newOffset = (int)KeymapManagerSetInputs - PAYLOAD_NEXT_INSTR_GET_INPUTS;
		s_origKeymapManager_SetInputs = SokuLib::union_cast<void (SokuLib::KeymapManager::*)()>(*(int *)PAYLOAD_ADDRESS_GET_INPUTS + PAYLOAD_NEXT_INSTR_GET_INPUTS);
		*(int *)PAYLOAD_ADDRESS_GET_INPUTS = newOffset;

		((unsigned *)0x00898680)[1] = 0x008986A8;
	}

	void activate()
	{
		if (sfmlWindow)
			return;

		puts("Opening window");
		sfmlWindow = new sf::RenderWindow{{640, 480}, "Advanced Practice Mode", sf::Style::Titlebar};
		puts("Window opened");
		Practice::initInputDisplay(profileParent);
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

		if (!settings.activated)
			return;
		puts("Removing hooks");
		settings.activated = false;
		VirtualProtect((PVOID)0x42A331, sizeof(patchCode), PAGE_EXECUTE_WRITECOPY, &old);
		for (unsigned i = 0; i < sizeof(patchCode); i++)
			((unsigned char *)0x42A331)[i] = originalCode[i];
		VirtualProtect((PVOID)0x42A331, sizeof(patchCode), old, &old);

		VirtualProtect((PVOID)PAYLOAD_ADDRESS_GET_INPUTS, 4, PAGE_EXECUTE_WRITECOPY, &old);
		*(int *)PAYLOAD_ADDRESS_GET_INPUTS = SokuLib::union_cast<int>(s_origKeymapManager_SetInputs) - PAYLOAD_NEXT_INSTR_GET_INPUTS;
		VirtualProtect((PVOID)PAYLOAD_ADDRESS_GET_INPUTS, 4, old, &old);
	}

	void deactivate()
	{
		if (!sfmlWindow)
			return;

		delete sfmlWindow;
		sfmlWindow = nullptr;
		removeHooks();
	}
}