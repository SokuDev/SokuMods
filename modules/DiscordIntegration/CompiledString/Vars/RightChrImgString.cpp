//
// Created by PinkySmile on 15/01/2021.
//

#include "RightChrImgString.hpp"

#include "vars.hpp"
#include <SokuLib.hpp>

std::string RightChrImgString::getString() const {
	auto it = charactersImg.find(SokuLib::rightChar);

	return (it == charactersImg.end() ? charactersImg.at(SokuLib::CHARACTER_RANDOM) : it->second);
}
