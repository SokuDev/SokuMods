//
// Created by PinkySmile on 23/02/2021.
//

#include "Logic.hpp"
#include "State.hpp"
#include "Gui.hpp"

namespace Practice
{
	void handlePlayerInput(SokuLib::KeymapManager &manager)
	{
		if (!settings.controlDummy)
			return;

		auto arraySrc = reinterpret_cast<int *>(&manager.input);
		auto arrayDest = reinterpret_cast<int *>(&dummy);

		for (int i = 0; i < 8; i++) {
			if (arraySrc[i] < 0)
				arrayDest[i] = min(arrayDest[i] - 1, -1);
			else if (arraySrc[i] > 0)
				arrayDest[i] = max(arrayDest[i] + 1, 1);
			else
				arrayDest[i] = 0;
		}
		memset(&manager.input, 0, sizeof(manager.input));
	}

	void handleDummyInput(SokuLib::KeymapManager &manager)
	{
		if (settings.controlDummy)
			memcpy(&manager.input, &dummy, sizeof(manager.input));
		else
			memset(&manager.input, 0, sizeof(manager.input));
	}

	void handleInput(SokuLib::KeymapManager &manager)
	{
		auto &mgr = SokuLib::getBattleMgr();

		if (&manager == mgr.leftCharacterManager.keyManager->keymapManager)
			return handlePlayerInput(manager);
		if (mgr.rightCharacterManager.keyManager && &manager == mgr.rightCharacterManager.keyManager->keymapManager)
			return handleDummyInput(manager);
	}

	void render()
	{
		sf::Event event;

		if (sfmlWindow) {
			sfmlWindow->clear(sf::Color(0xAA, 0xAA, 0xAA));
			try {
				Practice::updateGuiState();
			} catch (std::exception &e) {
				puts(e.what());
				throw;
			}
			while (sfmlWindow->pollEvent(event))
				Practice::gui.handleEvent(event);
			Practice::gui.draw();
			sfmlWindow->display();
		}
	}
}