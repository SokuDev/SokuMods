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
	bool fsPath = false;
	std::string path;
	SokuLib::DrawUtils::TextureRect rect = {0, 0, 0, 0};
	SokuLib::Vector2f translate = {0, 0};
	SokuLib::Vector2f untransformedRect = {0, 0};
	SokuLib::DrawUtils::Sprite sprite;
	SokuLib::Vector2<bool> mirror{false, false};
	float scale = 1;

	Icon(const std::string &path, const nlohmann::json &object);
};

class Scenario {
public:
	bool loaded = false;
	bool extra = false;
	volatile bool loading = false;
	bool canBeLocked = true;
	bool nameHiddenIfLocked = false;
	char score = -1;
	std::string file;
	std::string folder;
	std::string nameStr;
	std::string fileRel;
	std::string previewFile;
	std::string previewFileRel;
	std::string descriptionStr;
	std::unique_ptr<Image> preview;
	SokuLib::DrawUtils::Sprite name;
	SokuLib::DrawUtils::Sprite description;
	SokuLib::DrawUtils::Sprite scoreSprite;

	void loadPreview(bool force = false);
	Scenario(char score, int i, const std::string &path, const nlohmann::json &object);
	void setScore(char score);
};

class Pack {
public:
	bool previewFSAsset = false;
	std::string path;
	std::string nameStr;
	std::string category;
	std::string authorStr;
	std::string scorePath;
	std::string outroPath;
	std::string minVersion;
	std::string previewPath;
	std::string outroRelPath;
	std::string descriptionStr;
	std::unique_ptr<Icon> icon;
	std::vector<std::string> modes;
	std::vector<std::string> characters;
	SokuLib::DrawUtils::Sprite name;
	SokuLib::DrawUtils::Sprite error;
	SokuLib::DrawUtils::Sprite author;
	SokuLib::DrawUtils::Sprite preview;
	SokuLib::DrawUtils::Sprite description;
	std::vector<std::unique_ptr<Scenario>> scenarios;

	static bool isInvalidPath(const std::string &path);
	static bool checkVersion(const std::string &version);
	static int getVersionFromStr(const std::string &str);

	Pack() = default;
	Pack(const std::string &path, const nlohmann::json &object);
};

extern std::vector<std::string> authors;
extern std::vector<std::string> uniqueNames;
extern std::vector<std::string> uniqueModes;
extern std::vector<std::string> uniqueCategories;
extern std::vector<std::shared_ptr<Pack>> loadedPacks;
extern std::map<std::string, std::vector<std::shared_ptr<Pack>>> packsByName;
extern std::map<std::string, std::vector<std::shared_ptr<Pack>>> packsByMode;
extern std::map<std::string, std::vector<std::shared_ptr<Pack>>> packsByCategory;
extern std::map<unsigned, std::string> validCharacters;
extern std::map<unsigned, std::map<unsigned short, std::pair<std::string, unsigned int>>> characterCards;
extern const std::map<unsigned, std::string> swrCharacters;
extern bool hasSoku2;
extern char packsLocation[1024 + MAX_PATH];

void loadPacks();


#endif //SWRSTOYS_PACK_HPP
