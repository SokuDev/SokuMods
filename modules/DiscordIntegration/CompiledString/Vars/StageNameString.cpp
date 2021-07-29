//
// Created by PinkySmile on 15/01/2021.
//

#include "StageNameString.hpp"

#include "vars.hpp"
#include <SokuLib.hpp>

std::string StageNameString::getString() const
{

	switch (SokuLib::stageId) {
	case SokuLib::STAGE_HAKUREI_SHRINE_BROKEN:
	case SokuLib::STAGE_FOREST_OF_MAGIC:
	case SokuLib::STAGE_CREEK_OF_GENBU:
	case SokuLib::STAGE_YOUKAI_MOUNTAIN:
	case SokuLib::STAGE_MYSTERIOUS_SEA_OF_CLOUD:
	case SokuLib::STAGE_BHAVA_AGRA:
	case SokuLib::STAGE_HAKUREI_SHRINE:
	case SokuLib::STAGE_KIRISAME_MAGIC_SHOP:
	case SokuLib::STAGE_SCARLET_DEVIL_MANSION_CLOCK_TOWER:
	case SokuLib::STAGE_FOREST_OF_DOLLS:
	case SokuLib::STAGE_SCARLET_DEVIL_MANSION_LIBRARY:
	case SokuLib::STAGE_NETHERWORLD:
	case SokuLib::STAGE_SCARLET_DEVIL_MANSION_FOYER:
	case SokuLib::STAGE_HAKUGYOKUROU_SNOWY_GARDEN:
	case SokuLib::STAGE_BAMBOO_FOREST_OF_THE_LOST:
	case SokuLib::STAGE_SHORE_OF_MISTY_LAKE:
	case SokuLib::STAGE_MORIYA_SHRINE:
	case SokuLib::STAGE_MOUTH_OF_GEYSER:
	case SokuLib::STAGE_CATWALK_OF_GEYSER:
	case SokuLib::STAGE_FUSION_REACTOR_CORE:
		return stagesNames[SokuLib::flattenStageId(SokuLib::stageId)];
	default:
		return "Invalid stage";
	}
}
