//
// Created by PinkySmile on 15/01/2021.
//

#include <SokuLib.hpp>
#include "vars.hpp"
#include "LeftChrImgString.hpp"

std::string LeftChrImgString::getString() const
{
	return charactersImg[SokuLib::leftChar];
}
