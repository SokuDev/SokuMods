//
// Created by PinkySmile on 20/07/2021.
//

#ifndef SWRSTOYS_PACK_HPP
#define SWRSTOYS_PACK_HPP


#include <nlohmann/json.hpp>
#include <SokuLib.hpp>
#include "Images.hpp"

#ifdef _DEBUG
#define PANIC abort()
#else
#define PANIC
#endif

struct Icon {
	SokuLib::Vector2f translate = {0, 0};
	SokuLib::DrawUtils::Sprite sprite;

	Icon(const std::string &path, const nlohmann::json &object);
};

class Scenario {
public:
	bool canBeLocked = true;
	bool nameHiddenIfLocked = false;
	char score = -1;
	std::string file;
	std::string folder;
	std::string nameStr;
	std::unique_ptr<Image> preview;
	SokuLib::DrawUtils::Sprite name;
	SokuLib::DrawUtils::Sprite description;
	SokuLib::DrawUtils::Sprite scoreSprite;

	Scenario(char score, int i, const std::string &path, const nlohmann::json &object);
	void setScore(char score);
};

class Pack {
public:
	std::string nameStr;
	std::string category;
	std::string scorePath;
	std::unique_ptr<Icon> icon;
	std::vector<std::string> modes;
	SokuLib::DrawUtils::Sprite name;
	SokuLib::DrawUtils::Sprite error;
	SokuLib::DrawUtils::Sprite author;
	SokuLib::DrawUtils::Sprite preview;
	SokuLib::DrawUtils::Sprite description;
	std::vector<std::unique_ptr<Scenario>> scenarios;

	Pack(const std::string &path, const nlohmann::json &object);
};

extern std::vector<std::string> uniqueNames;
extern std::vector<std::string> uniqueModes;
extern std::vector<std::string> uniqueCategories;
extern std::vector<std::shared_ptr<Pack>> loadedPacks;
extern std::map<std::string, std::vector<std::shared_ptr<Pack>>> packsByName;
extern std::map<std::string, std::vector<std::shared_ptr<Pack>>> packsByMode;
extern std::map<std::string, std::vector<std::shared_ptr<Pack>>> packsByCategory;
extern std::map<unsigned, std::string> validCharacters;
extern const std::map<unsigned, std::string> swrCharacters;
extern bool hasSoku2;
extern char packsLocation[1024 + MAX_PATH];

void loadPacks();


#endif //SWRSTOYS_PACK_HPP
