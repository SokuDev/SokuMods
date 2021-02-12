//
// Created by PinkySmile on 15/01/2021.
//

#include "MyChrShortNameString.hpp"

#include "vars.hpp"
#include <SokuLib.hpp>

std::string MyChrShortNameString::getString() const {
	switch (SokuLib::mainMode) {
	case SokuLib::BATTLE_MODE_VSSERVER:
		return charactersNames[SokuLib::rightChar].first;
	default:
		return charactersNames[SokuLib::leftChar].first;
	}
}
