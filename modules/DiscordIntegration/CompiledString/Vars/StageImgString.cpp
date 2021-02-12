//
// Created by PinkySmile on 15/01/2021.
//

#include "StageImgString.hpp"

#include <SokuLib.hpp>

std::string StageImgString::getString() const {
	return "stage_" + std::to_string(SokuLib::flattenStageId(SokuLib::stageId) + 1);
}
