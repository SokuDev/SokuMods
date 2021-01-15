//
// Created by PinkySmile on 15/01/2021.
//

#include <SokuLib.hpp>
#include "vars.hpp"
#include "StageNameString.hpp"

std::string StageNameString::getString() const
{
	return stagesNames[SokuLib::flattenStageId(SokuLib::stageId)];
}
