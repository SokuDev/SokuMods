//
// Created by PinkySmile on 15/01/2021.
//

#include "MyChrFullNameString.hpp"

#include "vars.hpp"
#include <SokuLib.hpp>

std::string MyChrFullNameString::getString() const {
	switch (SokuLib::mainMode) {
	case SokuLib::BATTLE_MODE_VSSERVER:
		return charactersNames[SokuLib::rightChar].second;
	default:
		return charactersNames[SokuLib::leftChar].second;
	}
}
