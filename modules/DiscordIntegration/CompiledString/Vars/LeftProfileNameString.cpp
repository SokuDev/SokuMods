//
// Created by PinkySmile on 15/01/2021.
//

#include "LeftProfileNameString.hpp"

#include "../../ShiftJISDecoder.hpp"
#include <SokuLib.hpp>

std::string LeftProfileNameString::getString() const {
	switch (SokuLib::mainMode) {
	case SokuLib::BATTLE_MODE_VSSERVER:
	case SokuLib::BATTLE_MODE_VSCLIENT:
	case SokuLib::BATTLE_MODE_VSWATCH:
		return convertShiftJisToUTF8(SokuLib::getNetObject().profile1name);
	default:
		return convertShiftJisToUTF8(SokuLib::profile1.name);
	}
}
