//
// Created by PinkySmile on 23/02/2021.
//

#include "State.hpp"
#include "Gui.hpp"
#include "Logic.hpp"

#define PAYLOAD_ADDRESS_PLAYER 0x40A45E
#define PAYLOAD_NEXT_INSTR_PLAYER (PAYLOAD_ADDRESS_PLAYER + 4)

namespace Practice
{
	static void (*s_origKeymapManager_SetInputs)();
	static const unsigned char patchCode[] = {
		0x31, 0xED,                  //xor ebp, ebp
		0xE9, 0x91, 0x01, 0x00, 0x00 //jmp pc+00000191
	};
	static unsigned char originalCode[sizeof(patchCode)];

	struct FakeKeyMapMgr {
		SokuLib::KeymapManager base;

		void handleInput()
		{
			Practice::handleInput(this->base);
		}
	};

	void KeymapManagerSetInputs()
	{
		__asm push ecx;
		s_origKeymapManager_SetInputs();
		__asm pop ecx;
		SokuLib::union_cast<void (*)()>(&FakeKeyMapMgr::handleInput)();
	}

	sf::RenderWindow *sfmlWindow;
	char profilePath[1024 + MAX_PATH];
	char profileParent[1024 + MAX_PATH];
	SokuLib::KeyInput dummy;

	void activate()
	{
		DWORD old;
		int newOffset;

		if (sfmlWindow)
			return;

		sfmlWindow = new sf::RenderWindow{{640, 480}, "Advanced Practice Mode", sf::Style::Titlebar};
		Practice::init(profileParent);
		Practice::gui.setTarget(*sfmlWindow);
		try {
			Practice::loadAllGuiElements(profileParent);
		} catch (std::exception &e) {
			puts(e.what());
			throw;
		}

		//Bypass the basic practice features by skipping most of the function.
		VirtualProtect((PVOID)0x42A331, sizeof(patchCode), PAGE_EXECUTE_WRITECOPY, &old);
		for (unsigned i = 0; i < sizeof(patchCode); i++) {
			originalCode[i] = ((unsigned char *)0x42A331)[i];
			((unsigned char *)0x42A331)[i] = patchCode[i];
		}
		VirtualProtect((PVOID)0x42A331, sizeof(patchCode), old, &old);

		VirtualProtect((PVOID)PAYLOAD_ADDRESS_PLAYER, 4, PAGE_EXECUTE_WRITECOPY, &old);
		newOffset = (int)KeymapManagerSetInputs - PAYLOAD_NEXT_INSTR_PLAYER;
		s_origKeymapManager_SetInputs = reinterpret_cast<void (*)()>(*(int *)PAYLOAD_ADDRESS_PLAYER + PAYLOAD_NEXT_INSTR_PLAYER);
		*(int *)PAYLOAD_ADDRESS_PLAYER = newOffset;
		VirtualProtect((PVOID)PAYLOAD_ADDRESS_PLAYER, 4, old, &old);

		((unsigned *)0x00898680)[1] = 0x008986A8;
	}

	void deactivate()
	{
		DWORD old;

		if (!sfmlWindow)
			return;

		delete sfmlWindow;
		sfmlWindow = nullptr;

		VirtualProtect((PVOID)0x42A331, sizeof(patchCode), PAGE_EXECUTE_WRITECOPY, &old);
		for (unsigned i = 0; i < sizeof(patchCode); i++)
			((unsigned char *)0x42A331)[i] = originalCode[i];
		VirtualProtect((PVOID)0x42A331, sizeof(patchCode), old, &old);

		VirtualProtect((PVOID)PAYLOAD_ADDRESS_PLAYER, 4, PAGE_EXECUTE_WRITECOPY, &old);
		*(int *)PAYLOAD_ADDRESS_PLAYER = SokuLib::union_cast<int>(s_origKeymapManager_SetInputs) - PAYLOAD_NEXT_INSTR_PLAYER;
		VirtualProtect((PVOID)PAYLOAD_ADDRESS_PLAYER, 4, old, &old);
	}
}