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
		unsigned timer = 0;
		bool wasBlocking = false;
		bool wasAttacking = false;
		bool started = false;
	};

	extern std::map<SokuLib::Action, unsigned char> blockStun;

	std::optional<int> getFrameAdvantage(const SokuLib::CharacterManager &attacker, const SokuLib::CharacterManager &blocker, BlockingState &state);
	void displayFrameStuff();
}


#endif //SWRSTOYS_GAP_HPP
