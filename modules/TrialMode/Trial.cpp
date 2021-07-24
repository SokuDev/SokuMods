//
// Created by PinkySmile on 23/07/2021.
//

#include "Trial.hpp"
#include "ComboTrial.hpp"

const std::map<std::string, std::function<Trial *(SokuLib::Character player, const nlohmann::json &json)>> Trial::_factory{
	{ "combo", [](SokuLib::Character player, const nlohmann::json &json){ return new ComboTrial(player, json); } }
};

Trial *Trial::create(SokuLib::Character player, const nlohmann::json &json)
{
	if (!json.contains("type") || !json["type"].is_string())
		return nullptr;
	return Trial::_factory.at(json["type"])(player, json);
}
