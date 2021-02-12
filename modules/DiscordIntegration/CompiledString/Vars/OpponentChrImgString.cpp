//
// Created by PinkySmile on 15/01/2021.
//

#include "OpponentChrImgString.hpp"

#include "vars.hpp"
#include <SokuLib.hpp>

std::string OpponentChrImgString::getString() const {
	switch (SokuLib::mainMode) {
	case SokuLib::BATTLE_MODE_VSSERVER:
		return charactersImg[SokuLib::leftChar];
	default:
		return charactersImg[SokuLib::rightChar];
	}
}
