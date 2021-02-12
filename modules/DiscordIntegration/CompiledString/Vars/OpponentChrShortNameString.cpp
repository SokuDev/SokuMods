//
// Created by PinkySmile on 15/01/2021.
//

#include "OpponentChrShortNameString.hpp"

#include "vars.hpp"
#include <SokuLib.hpp>

std::string OpponentChrShortNameString::getString() const {
	switch (SokuLib::mainMode) {
	case SokuLib::BATTLE_MODE_VSSERVER:
		return charactersNames[SokuLib::leftChar].first;
	default:
		return charactersNames[SokuLib::rightChar].first;
	}
}
