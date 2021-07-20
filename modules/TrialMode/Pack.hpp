//
// Created by PinkySmile on 20/07/2021.
//

#ifndef SWRSTOYS_PACK_HPP
#define SWRSTOYS_PACK_HPP


#include <nlohmann/json.hpp>
#include <SokuLib.hpp>

#ifdef _DEBUG
#define PANIC abort()
#else
#define PANIC
#endif

struct Icon {
	SokuLib::DrawUtils::Sprite sprite;
	SokuLib::DrawUtils::Vector2<int> offset = {0, 0};
	SokuLib::DrawUtils::Vector2<float> scale = {0, 0};

	Icon(const std::string &path, const nlohmann::json &object);
};

struct Scenario {
	std::string file;
	SokuLib::DrawUtils::Sprite name;
	SokuLib::DrawUtils::Sprite preview;
	SokuLib::DrawUtils::Sprite description;

	Scenario(int i, const std::string &path, const nlohmann::json &object);
};

struct Pack {
	std::string category;
	std::unique_ptr<Icon> icon;
	std::vector<std::unique_ptr<Scenario>> scenarios;
	SokuLib::DrawUtils::Sprite name;
	SokuLib::DrawUtils::Sprite error;
	SokuLib::DrawUtils::Sprite author;

	Pack(const std::string &path, const nlohmann::json &object);
};

extern std::vector<std::unique_ptr<Pack>> loadedPacks;
extern std::map<unsigned, std::string> validCharacters;
extern const std::map<unsigned, std::string> swrCharacters;
extern bool hasSoku2;
extern char packsLocation[1024 + MAX_PATH];

void loadPacks();


#endif //SWRSTOYS_PACK_HPP
