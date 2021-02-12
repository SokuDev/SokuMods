//
// Created by PinkySmile on 15/01/2021.
//

#include "MyChrImgString.hpp"

#include "vars.hpp"
#include <SokuLib.hpp>

std::string MyChrImgString::getString() const {
	switch (SokuLib::mainMode) {
	case SokuLib::BATTLE_MODE_VSSERVER:
		return charactersImg[SokuLib::rightChar];
	default:
		return charactersImg[SokuLib::leftChar];
	}
}
