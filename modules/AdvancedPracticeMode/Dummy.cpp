//
// Created by PinkySmile on 23/02/2021.
//

#include "Dummy.hpp"
#include "State.hpp"

namespace Practice
{
	static void handleAirTech(SokuLib::CharacterManager &player, SokuLib::CharacterManager &dummy, SokuLib::KeymapManager &manager)
	{
		auto direction = (player.objectBase.position.x < dummy.objectBase.position.x ? -1 : 1);
		static int airtechdir = 0;

		if (!dummy.objectBase.frameCount)
			airtechdir = rand() % 2 * 2 - 1;
		manager.input.d = 1;
		switch (settings.airtech) {
		case FORWARD_AIRTECH:
			manager.input.horizontalAxis = direction;
			return;
		case BACKWARD_AIRTECH:
			manager.input.horizontalAxis = -direction;
			return;
		case RANDOM_AIRTECH:
			manager.input.horizontalAxis = airtechdir;
			return;
		}
	}

	static void handleGroundTech(SokuLib::CharacterManager &player, SokuLib::CharacterManager &dummy, SokuLib::KeymapManager &manager)
	{
		auto direction = (player.objectBase.position.x < dummy.objectBase.position.x ? -1 : 1);
		static int techdir = 0;

		if (!dummy.objectBase.frameCount)
			techdir = rand() % 3 - 1;
		manager.input.d = 1;
		switch (settings.tech) {
			case FORWARD_TECH:
				manager.input.horizontalAxis = direction;
				return;
			case BACKWARD_TECH:
				manager.input.horizontalAxis = -direction;
				return;
			case RANDOM_TECH:
				manager.input.horizontalAxis = techdir;
				return;
		}
	}

	static void moveDummyX(SokuLib::CharacterManager &dummy, SokuLib::KeymapManager &manager)
	{
		manager.input.horizontalAxis = settings.posX - dummy.objectBase.position.x;

		if (std::abs(manager.input.horizontalAxis) < dummy.objectBase.speed.x) {
			dummy.objectBase.position.x = settings.posX;
			manager.input.horizontalAxis = 0;
		}
	}

	static void moveDummyY(SokuLib::CharacterManager &dummy, SokuLib::KeymapManager &manager)
	{
		int diff = settings.posY - dummy.objectBase.position.y;

		if (diff == 0)
			return;
		dummy.airdashCount = 0;
		dummy.currentSpirit = 1000;
		if (diff > 0) {
			manager.input.d = 1;
			manager.input.verticalAxis = -1;
			return;
		}
	}

	static bool isIdle(SokuLib::Action action)
	{
		if (action < SokuLib::ACTION_GROUND_HIT_SMALL_HITSTUN)
			return true;
		if (action >= SokuLib::ACTION_FORWARD_DASH && action < SokuLib::ACTION_5A)
			return true;
		return action == 181;
	}

	void moveDummy(SokuLib::KeymapManager &manager)
	{
		auto &battle = SokuLib::getBattleMgr();
		auto &player = battle.leftCharacterManager;
		auto &dummy = battle.rightCharacterManager;

		rand();
		memset(&manager.input, 0, sizeof(manager.input));
		if (settings.airtech && dummy.objectBase.action >= SokuLib::ACTION_AIR_HIT_SMALL_HITSTUN && dummy.objectBase.action <= SokuLib::ACTION_AIR_HIT_GROUND_SLAMMED)
			return handleAirTech(player, dummy, manager);
		if (settings.tech && (dummy.objectBase.action == SokuLib::ACTION_KNOCKED_DOWN_STATIC || dummy.objectBase.action == SokuLib::ACTION_KNOCKED_DOWN))
			return handleGroundTech(player, dummy, manager);
		if (isIdle(dummy.objectBase.action)) {
			if (settings.posX != 0 && settings.posX != dummy.objectBase.position.x)
				return moveDummyX(dummy, manager);
			if (settings.posY != 0 && settings.posY != dummy.objectBase.position.y)
				return moveDummyY(dummy, manager);
		}
	}
}