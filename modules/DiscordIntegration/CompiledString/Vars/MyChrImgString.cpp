//
// Created by PinkySmile on 15/01/2021.
//

#include "MyChrImgString.hpp"

#include "vars.hpp"
#include <SokuLib.hpp>

std::string MyChrImgString::getString() const {
	std::map<unsigned int, std::string>::iterator it;

	switch (SokuLib::mainMode) {
	case SokuLib::BATTLE_MODE_VSSERVER:
		it = charactersImg.find(SokuLib::rightChar);
		break;
	default:
		it = charactersImg.find(SokuLib::leftChar);
	}
	return (it == charactersImg.end() ? charactersImg.at(SokuLib::CHARACTER_RANDOM) : it->second);
}
