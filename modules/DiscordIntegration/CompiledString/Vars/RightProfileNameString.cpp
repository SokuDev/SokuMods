//
// Created by PinkySmile on 15/01/2021.
//

#include <SokuLib.hpp>
#include "../../ShiftJISDecoder.hpp"
#include "RightProfileNameString.hpp"

std::string RightProfileNameString::getString() const
{
	switch (SokuLib::mainMode) {
	case SokuLib::BATTLE_MODE_VSSERVER:
	case SokuLib::BATTLE_MODE_VSCLIENT:
	case SokuLib::BATTLE_MODE_VSWATCH:
		return convertShiftJisToUTF8(SokuLib::getNetObject().profile2name);
	default:
		return convertShiftJisToUTF8(SokuLib::player2Profile);
	}
}
