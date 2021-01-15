//
// Created by PinkySmile on 15/01/2021.
//

#include <SokuLib.hpp>
#include "../../ShiftJISDecoder.hpp"
#include "LeftProfileNameString.hpp"

std::string LeftProfileNameString::getString() const
{
	switch (SokuLib::mainMode) {
	case SokuLib::BATTLE_MODE_VSSERVER:
	case SokuLib::BATTLE_MODE_VSCLIENT:
	case SokuLib::BATTLE_MODE_VSWATCH:
		return convertShiftJisToUTF8(SokuLib::getNetObject().profile1name);
	default:
		return convertShiftJisToUTF8(SokuLib::player1Profile);
	}
}
