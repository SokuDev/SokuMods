//
// Created by PinkySmile on 15/01/2021.
//

#include "OpponentProfileNameString.hpp"

#include "../../ShiftJISDecoder.hpp"
#include <SokuLib.hpp>

std::string OpponentProfileNameString::getString() const {
	switch (SokuLib::mainMode) {
	case SokuLib::BATTLE_MODE_VSSERVER:
		return convertShiftJisToUTF8(SokuLib::getNetObject().profile1name);
	case SokuLib::BATTLE_MODE_VSCLIENT:
		return convertShiftJisToUTF8(SokuLib::getNetObject().profile2name);
	default:
		return convertShiftJisToUTF8(SokuLib::player2Profile);
	}
}
