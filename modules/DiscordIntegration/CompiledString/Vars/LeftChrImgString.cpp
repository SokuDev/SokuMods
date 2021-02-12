//
// Created by PinkySmile on 15/01/2021.
//

#include "LeftChrImgString.hpp"

#include "vars.hpp"
#include <SokuLib.hpp>

std::string LeftChrImgString::getString() const {
	return charactersImg[SokuLib::leftChar];
}
