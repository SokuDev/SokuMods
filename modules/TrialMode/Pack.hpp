//
// Created by PinkySmile on 20/07/2021.
//

#ifndef SWRSTOYS_PACK_HPP
#define SWRSTOYS_PACK_HPP


#include <nlohmann/json.hpp>
#include <SokuLib.hpp>

struct Icon {
	SokuLib::DrawUtils::Sprite sprite;
	SokuLib::DrawUtils::Vector2<int> offset = {0, 0};
	SokuLib::DrawUtils::Vector2<float> scale = {0, 0};

	Icon(const nlohmann::json &object);
};

struct Scenario {
	std::string file;
	SokuLib::DrawUtils::Sprite name;
	SokuLib::DrawUtils::Sprite preview;
	SokuLib::DrawUtils::Sprite description;

	Scenario(const nlohmann::json &object);
};

struct Pack {
	std::string category;
	std::unique_ptr<Icon> icon;
	std::vector<Scenario> scenarios;
	SokuLib::DrawUtils::Sprite name;
	SokuLib::DrawUtils::Sprite error;
	SokuLib::DrawUtils::Sprite author;

	Pack(const nlohmann::json &object);
};

extern std::vector<Pack> loadedPacks;

void loadPacks();


#endif //SWRSTOYS_PACK_HPP
