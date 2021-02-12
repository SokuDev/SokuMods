//
// Created by PinkySmile on 15/01/2021.
//

#include "MyProfileNameString.hpp"

#include "../../ShiftJISDecoder.hpp"
#include <SokuLib.hpp>

std::string MyProfileNameString::getString() const {
	switch (SokuLib::mainMode) {
	case SokuLib::BATTLE_MODE_VSSERVER:
		return convertShiftJisToUTF8(SokuLib::getNetObject().profile2name);
	case SokuLib::BATTLE_MODE_VSCLIENT:
		return convertShiftJisToUTF8(SokuLib::getNetObject().profile1name);
	default:
		return convertShiftJisToUTF8(SokuLib::player1Profile);
	}
}
