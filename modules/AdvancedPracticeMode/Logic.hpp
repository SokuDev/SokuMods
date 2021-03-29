//
// Created by PinkySmile on 23/02/2021.
//

#ifndef SWRSTOYS_LOGIC_HPP
#define SWRSTOYS_LOGIC_HPP


#include <SokuLib.hpp>

namespace Practice
{
	bool compareKeyInputs(const SokuLib::KeyInput &input1, const SokuLib::KeyInput &input2);
	void handleInput(SokuLib::KeymapManager &manager);
	void render();
	void update();
	void playerUseCard(int handSize);
}


#endif //SWRSTOYS_LOGIC_HPP
