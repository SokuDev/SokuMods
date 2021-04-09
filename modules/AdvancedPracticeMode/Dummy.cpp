//
// Created by PinkySmile on 23/02/2021.
//

#include <vector>
#include "Dummy.hpp"
#include "State.hpp"

namespace Practice
{
	static std::vector<SokuLib::KeyInput> nextDummyInputs;

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
		if (dummy.objectBase.action < SokuLib::ACTION_STAND_GROUND_HIT_SMALL_HITSTUN || dummy.objectBase.action >= SokuLib::ACTION_FORWARD_DASH)
			dummy.currentSpirit = max(min(settings.nonSaved.rightState.maxCurrentSpirit, 200 * (5 - settings.nonSaved.rightState.brokenOrbs)), 1);
		if (diff > 0) {
			manager.input.d = 1;
			manager.input.verticalAxis = -1;
			return;
		}
	}

	static bool isIdle(SokuLib::Action action)
	{
		if (action < SokuLib::ACTION_STAND_GROUND_HIT_SMALL_HITSTUN)
			return true;
		if (action >= SokuLib::ACTION_FORWARD_DASH && action < SokuLib::ACTION_5A)
			return true;
		return action == SokuLib::ACTION_FORWARD_AIRTECH || action == SokuLib::ACTION_BACKWARD_AIRTECH;
	}

	std::vector<std::string> moveNameToSequenceStrs(const std::string &input)
	{
		std::vector<std::string> sequenceStrs;

		sequenceStrs.reserve(input.size());
		for (char c : input) {
			if (sequenceStrs.empty())
				sequenceStrs.emplace_back(&c, 1);
			else {
				auto last = sequenceStrs.back().empty() ? 0 : sequenceStrs.back().back();

				if (c == ' ')
					sequenceStrs.emplace_back();
				else if (isdigit(last) && isdigit(c))
					sequenceStrs.emplace_back(&c, 1);
				else
					sequenceStrs.back().push_back(c);
			}
		}
		printf("Sequence for move %s is:\n", input.c_str());
		for (auto &str : sequenceStrs)
			printf("%s\n", str.c_str());
		return sequenceStrs;
	}

	void dummyBeforeHit(bool isHigh)
	{
		if (settings.nonSaved.enabled)
			return;

		auto &battle = SokuLib::getBattleMgr();
		auto &player = battle.leftCharacterManager;
		auto &dummy = battle.rightCharacterManager;
		auto direction = (player.objectBase.position.x < dummy.objectBase.position.x ? -1 : 1);

		switch (settings.block) {
		case BLOCK:
			dummy.keyMap.horizontalAxis = -direction;
			dummy.keyMap.verticalAxis = isHigh;
			break;
		case HIGH_BLOCKING:
			dummy.keyMap.horizontalAxis = -direction;
			dummy.keyMap.verticalAxis = 0;
			break;
		case LOW_BLOCKING:
			dummy.keyMap.horizontalAxis = -direction;
			dummy.keyMap.verticalAxis = 1;
			break;
		case RANDOM_HEIGHT_BLOCKING:
			dummy.keyMap.horizontalAxis = -direction;
			dummy.keyMap.verticalAxis = rand() % 2;
			break;
		case BLOCK_AFTER_FIRST_HIT:
			if (dummyHitCounter) {
				dummy.keyMap.horizontalAxis = -direction;
				dummy.keyMap.verticalAxis = isHigh;
			}
			break;
		case BLOCK_FIRST_HIT:
			if (!dummyHitCounter) {
				dummy.keyMap.horizontalAxis = -direction;
				dummy.keyMap.verticalAxis = isHigh;
			}
			break;
		default:
			dummy.keyMap.horizontalAxis = 0;
			break;
		}
	}

	std::vector<SokuLib::KeyInput> moveNameToSequence(const std::vector<std::string> &strs)
	{
		std::vector<SokuLib::KeyInput> results;

		results.reserve(strs.size() * 2);
		for (auto &str : strs) {
			if (str.empty())
				continue;
			if (results.empty())
				results.push_back(moveNameToInput(str));
			else {
				auto result = moveNameToInput(str);

				if (result.horizontalAxis == results.back().horizontalAxis && result.verticalAxis == results.back().verticalAxis)
					results.emplace_back();
				results.push_back(result);
			}
		}
		puts("Key sequence is:");
		for (auto &in : results)
			printf("h%i v%i a%i b%i c%i d%i ch%i s%i\n", in.horizontalAxis, in.verticalAxis, in.a, in.b, in.c, in.d, in.changeCard, in.spellcard);
		return results;
	}

	std::vector<SokuLib::KeyInput> moveNameToSequence(const std::string &input)
	{
		return moveNameToSequence(moveNameToSequenceStrs(input));
	}

	SokuLib::KeyInput moveNameToInput(const std::string &input)
	{
		SokuLib::KeyInput result;

		memset(&result, 0, sizeof(result));
		for (auto c : input) {
			switch (c) {
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				result.verticalAxis   = -((c - '1') / 3 - 1);
				result.horizontalAxis = (c - '1') % 3 - 1;
				break;
			case 'a':
			case 'A':
				result.a = 1;
				break;
			case 'b':
			case 'B':
				result.b = 1;
				break;
			case 'c':
			case 'C':
				result.c = 1;
				break;
			case 'd':
			case 'D':
				result.d = 1;
				break;
			case 's':
			case 'S':
				result.spellcard = 1;
				break;
			default:
				abort();
			}
		}
		return result;
	}

	void addInputSequence(const std::vector<SokuLib::KeyInput> &inputs)
	{
		for (auto &in : inputs)
			addNextInput(in);
	}

	void addInputSequence(const std::vector<std::string> &inputs)
	{
		addInputSequence(moveNameToSequence(inputs));
	}

	void addInputSequence(const std::string &inputs)
	{
		addInputSequence(moveNameToSequence(inputs));
	}

	void addNextInput(const SokuLib::KeyInput &input)
	{
		nextDummyInputs.push_back(input);
	}

	void addNextInput(const std::string &input)
	{
		addNextInput(moveNameToInput(input));
	}

	void moveDummy(SokuLib::KeymapManager &manager)
	{
		auto &battle = SokuLib::getBattleMgr();
		auto &player = battle.leftCharacterManager;
		auto &dummy = battle.rightCharacterManager;
		auto direction = (player.objectBase.position.x < dummy.objectBase.position.x ? -1 : 1);

		if (settings.nonSaved.playingMacro) {
			while (!settings.nonSaved.playingMacroBuffer.macroElems.empty() && !settings.nonSaved.playingMacroBuffer.macroElems.front().second)
				settings.nonSaved.playingMacroBuffer.macroElems.erase(settings.nonSaved.playingMacroBuffer.macroElems.begin());
			if (!settings.nonSaved.playingMacroBuffer.macroElems.empty()) {
				auto &next = settings.nonSaved.playingMacroBuffer.macroElems.front();
				auto input = next.first;

				input.horizontalAxis *= direction;
				memcpy(&manager.input, &input, sizeof(manager.input));
				next.second--;
				return;
			}
			if (!settings.nonSaved.playList.empty()) {
				if (settings.nonSaved.loopMacros)
					settings.nonSaved.playList.push_back(settings.nonSaved.playList.front());
				settings.nonSaved.playingMacroBuffer = settings.nonSaved.playList.front();
				settings.nonSaved.playList.pop_front();
				puts("Done !");
				return;
			}
			settings.nonSaved.playingMacro = false;
			puts("Done !");
		}
		if (!nextDummyInputs.empty()) {
			auto &next = nextDummyInputs.front();

			next.horizontalAxis *= direction;
			memcpy(&manager.input, &next, sizeof(manager.input));
			nextDummyInputs.erase(nextDummyInputs.begin());
			return;
		}

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