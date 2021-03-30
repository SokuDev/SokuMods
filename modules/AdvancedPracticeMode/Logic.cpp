//
// Created by PinkySmile on 23/02/2021.
//

#include "Logic.hpp"
#include "State.hpp"
#include "Gui.hpp"
#include "Dummy.hpp"
#include "Inputs.hpp"
#include "Hitboxes.hpp"

namespace Practice
{
	static int useCard = -1;

	bool compareKeyInputs(const SokuLib::KeyInput &input1, const SokuLib::KeyInput &input2)
	{
		return	!input1.a == !input2.a &&
			!input1.b == !input2.b &&
			!input1.c == !input2.c &&
			!input1.d == !input2.d &&
			!input1.changeCard == !input2.changeCard &&
			!input1.spellcard == !input2.spellcard &&
			!copysign(input1.horizontalAxis, abs(input1.horizontalAxis)) == !copysign(input2.horizontalAxis, abs(input2.horizontalAxis)) &&
			!copysign(input1.verticalAxis,   abs(input1.verticalAxis))   == !copysign(input2.verticalAxis,   abs(input2.verticalAxis));
	}

	void playerUseCard(int handSize)
	{
		useCard = handSize;
	}

	static void addInputToMacro(SokuLib::KeyInput input, SokuLib::Direction direction)
	{
		input.horizontalAxis *= direction;
		if (!settings.nonSaved.recordBuffer->empty() && compareKeyInputs(settings.nonSaved.recordBuffer->back().first, input))
			settings.nonSaved.recordBuffer->back().second++;
		else
			settings.nonSaved.recordBuffer->push_back({input, 1});
	}

	void handlePlayerInput(SokuLib::KeymapManager &manager)
	{
		if (settings.nonSaved.recordingMacro && !settings.nonSaved.recordForDummy)
			addInputToMacro(manager.input, SokuLib::getBattleMgr().leftCharacterManager.objectBase.direction);

		if (useCard != -1) {
			memset(&manager.input, 0, sizeof(manager.input));
			manager.input.changeCard = useCard;
			manager.input.spellcard = !useCard;
			useCard--;
			return;
		}

		if (!settings.nonSaved.controlDummy && !(settings.nonSaved.recordingMacro && settings.nonSaved.recordForDummy))
			return;

		auto arraySrc = reinterpret_cast<int *>(&manager.input);
		auto arrayDest = reinterpret_cast<int *>(&lastPlayerInputs);

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
		if (settings.nonSaved.recordingMacro && settings.nonSaved.recordForDummy)
			addInputToMacro(lastPlayerInputs, SokuLib::getBattleMgr().rightCharacterManager.objectBase.direction);
		if (settings.nonSaved.controlDummy || (settings.nonSaved.recordingMacro && settings.nonSaved.recordForDummy))
			memcpy(&manager.input, &lastPlayerInputs, sizeof(manager.input));
		else
			moveDummy(manager);
	}

	void handleInput(SokuLib::KeymapManager &manager)
	{
		if (SokuLib::sceneId != SokuLib::SCENE_BATTLE)
			return;

		auto &mgr = SokuLib::getBattleMgr();

		if (&manager == mgr.leftCharacterManager.keyManager->keymapManager)
			return handlePlayerInput(manager);
		if (mgr.rightCharacterManager.keyManager && &manager == mgr.rightCharacterManager.keyManager->keymapManager)
			return handleDummyInput(manager);
	}

	void render()
	{
		sf::Event event;

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
		drawAllBoxes();
		displayInputs();
	}

	void applyCharacterState(const CharacterState &state, SokuLib::CharacterManager &manager, SokuLib::Character character)
	{
		if (!idleCounter)
		{
			if (state.HPInstantRegen)
				manager.objectBase.hp = state.hp;

			if (state.SPInstantRegen)
				manager.currentSpirit = state.maxCurrentSpirit;

			manager.maxSpirit = 1000 - state.brokenOrbs * 200;

			if (manager.currentSpirit >= state.maxCurrentSpirit)
				manager.currentSpirit = state.maxCurrentSpirit;
			manager.timeWithBrokenOrb = 0;
		}

		memcpy(&manager.skillMap, &state.skillMap, sizeof(state.skillMap));
		manager.controlRod = state.rodLevel;
		manager.sacrificialDolls = state.dollLevel;
		manager.tenguFans = state.fanLevel;
		manager.drops = min(state.dropLevel, 2);
		if (state.dropLevel == 3)
			manager.dropInvulTimeLeft = 2;
		manager.grimoires = state.grimoireLevel;
		if (character == SokuLib::CHARACTER_REISEN)
			manager.elixirUsed = state.specialValue;
		else if (character == SokuLib::CHARACTER_YUYUKO)
			manager.resurrectionButterfliesUsed = state.specialValue;
	}

	void weatherControl() {
		if (settings.forceWeather) {
			if (settings.weatherResetRequest) {
				settings.weatherResetRequest = SokuLib::activeWeather != SokuLib::WEATHER_CLEAR;
				SokuLib::weatherCounter = 1;
			} else if (SokuLib::activeWeather == SokuLib::WEATHER_CLEAR)
				SokuLib::weatherCounter = -1;
			else
				SokuLib::weatherCounter = settings.weatherTime;
		}
	}

	void update()
	{
		updateInputLists();
		if (SokuLib::mainMode != SokuLib::BATTLE_MODE_PRACTICE)
			return;

		if (settings.nonSaved.activated) {
			auto &battle = SokuLib::getBattleMgr();
			auto leftAction = battle.leftCharacterManager.objectBase.action;
			auto rightAction = battle.rightCharacterManager.objectBase.action;

			if (idleCounter)
				idleCounter--;
			if (dummyHitCounter)
				dummyHitCounter--;

			if (
				leftAction  > SokuLib::ACTION_GROUND_HIT_SMALL_HITSTUN ||
				rightAction > SokuLib::ACTION_GROUND_HIT_SMALL_HITSTUN
			)
				idleCounter = 20;
			if (
				rightAction >= SokuLib::ACTION_GROUND_HIT_SMALL_HITSTUN &&
				rightAction < SokuLib::ACTION_FORWARD_AIRTECH
			)
				dummyHitCounter = 20;
			SokuLib::practiceSettings->state = SokuLib::DUMMY_STATE_2P_CONTROL;
			weatherControl();
			applyCharacterState(settings.nonSaved.leftState,  battle.leftCharacterManager,  SokuLib::leftChar);
			applyCharacterState(settings.nonSaved.rightState, battle.rightCharacterManager, SokuLib::rightChar);

			switch (settings.counter) {
			case COUNTER_OFF:
				SokuLib::practiceSettings->counter = SokuLib::COUNTER_OFF;
				break;
			case COUNTER_ON:
				SokuLib::practiceSettings->counter = SokuLib::COUNTER_ON;
				break;
			case COUNTER_RANDOM:
				SokuLib::practiceSettings->counter = rand() % 2 ? SokuLib::COUNTER_ON : SokuLib::COUNTER_OFF;
				break;
			}
		}
	}
}