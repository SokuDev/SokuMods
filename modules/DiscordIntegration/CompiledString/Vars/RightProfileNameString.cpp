//
// Created by PinkySmile on 15/01/2021.
//

#include "RightProfileNameString.hpp"

#include "../../ShiftJISDecoder.hpp"
#include <SokuLib.hpp>

std::string RightProfileNameString::getString() const {
	switch (SokuLib::mainMode) {
	case SokuLib::BATTLE_MODE_VSSERVER:
	case SokuLib::BATTLE_MODE_VSCLIENT:
	case SokuLib::BATTLE_MODE_VSWATCH:
		return convertShiftJisToUTF8(SokuLib::getNetObject().profile2name);
	default:
		return convertShiftJisToUTF8(SokuLib::profile2.name);
	}
}
