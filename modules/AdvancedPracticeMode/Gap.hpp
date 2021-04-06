//
// Created by PinkySmile on 05/04/2021.
//

#ifndef SWRSTOYS_GAP_HPP
#define SWRSTOYS_GAP_HPP


#include <optional>
#include <map>
#include <SokuLib.hpp>

namespace Practice
{
	struct BlockingState
	{
		int lastFrame;
		SokuLib::Action lastAction;
		unsigned hitTimer = 0;
		unsigned timer = 0;
		int gapCounter = -1;
		bool wasBlocking = false;
		bool wasAttacking = false;
		bool started = false;
	};

	extern std::map<SokuLib::Action, unsigned char> hitStun;
	extern std::map<SokuLib::Action, unsigned char> blockStun;

	std::optional<int> getFrameAdvantage(const SokuLib::CharacterManager &attacker, const SokuLib::CharacterManager &blocker, BlockingState &state);
	void displayFrameStuff();
	void updatedFrameStuff();
}


#endif //SWRSTOYS_GAP_HPP
