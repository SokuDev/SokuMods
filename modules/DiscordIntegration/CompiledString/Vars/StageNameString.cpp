//
// Created by PinkySmile on 15/01/2021.
//

#include "StageNameString.hpp"

#include "vars.hpp"
#include <SokuLib.hpp>

std::string StageNameString::getString() const {
	return stagesNames[SokuLib::flattenStageId(SokuLib::stageId)];
}
