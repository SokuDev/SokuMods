//
// Created by PinkySmile on 15/01/2021.
//

#include "LeftChrImgString.hpp"

#include "vars.hpp"
#include <SokuLib.hpp>

std::string LeftChrImgString::getString() const {
	auto it = charactersImg.find(SokuLib::leftChar);

	return (it == charactersImg.end() ? charactersImg.at(SokuLib::CHARACTER_RANDOM) : it->second);
}
