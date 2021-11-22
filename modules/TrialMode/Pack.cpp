//
// Created by PinkySmile on 20/07/2021.
//

#include <fstream>
#include <process.h>
#include "Pack.hpp"
#include "Menu.hpp"
#include "version.h"

#ifndef _DEBUG
#define puts(...)
#define printf(...)
#endif

#define RED_COLOR  SokuLib::DrawUtils::DxSokuColor{0xFF, 0xA0, 0xA0}
#define BLUE_COLOR SokuLib::DrawUtils::DxSokuColor{0xA0, 0xA0, 0xFF}

bool hasSoku2 = false;
std::vector<std::string> uniqueNames;
std::vector<std::string> uniqueModes;
std::vector<std::string> uniqueCategories;
std::vector<std::shared_ptr<Pack>> loadedPacks;
std::map<std::string, std::vector<std::shared_ptr<Pack>>> packsByName;
std::map<std::string, std::vector<std::shared_ptr<Pack>>> packsByMode;
std::map<std::string, std::vector<std::shared_ptr<Pack>>> packsByCategory;
std::map<unsigned, std::map<unsigned short, std::pair<std::string, unsigned int>>> characterCards;
std::map<unsigned, std::string> validCharacters{
	{ SokuLib::CHARACTER_REIMU, "reimu" },
	{ SokuLib::CHARACTER_MARISA, "marisa" },
	{ SokuLib::CHARACTER_ALICE, "alice" },
	{ SokuLib::CHARACTER_PATCHOULI, "patchouli" },
	{ SokuLib::CHARACTER_SANAE, "sanae" },
	{ SokuLib::CHARACTER_CIRNO, "chirno" },
	{ SokuLib::CHARACTER_MEILING, "meirin" },
	{ SokuLib::CHARACTER_UTSUHO, "utsuho" },
	{ SokuLib::CHARACTER_SUWAKO, "suwako" },
};
const std::map<unsigned, std::string> swrCharacters{
	{ SokuLib::CHARACTER_YOUMU, "youmu" },
	{ SokuLib::CHARACTER_REMILIA, "remilia" },
	{ SokuLib::CHARACTER_SAKUYA, "sakuya" },
	{ SokuLib::CHARACTER_YUYUKO, "yuyuko" },
	{ SokuLib::CHARACTER_YUKARI, "yukari" },
	{ SokuLib::CHARACTER_SUIKA, "suika" },
	{ SokuLib::CHARACTER_REISEN, "udonge" },
	{ SokuLib::CHARACTER_AYA, "aya" },
	{ SokuLib::CHARACTER_KOMACHI, "komachi" },
	{ SokuLib::CHARACTER_IKU, "iku" },
	{ SokuLib::CHARACTER_TENSHI, "tenshi" },
};
char packsLocation[1024 + MAX_PATH];

static std::vector<std::string> authors{
	"9 Baka Cirno",
	"Illuminated Catfish",
	"Fun frog that never wakes back up",
	"Marisa's tongue",
	"The great wall",
	"Spinning Ran",
	"Hieda no Akyuu",
	"Book stealing rats",
	"The library's cat",
	"Insane Marisa",
	"A scary jelly donut",
	"The light eater",
	"Frightened Youmu",
};

bool isInvalidPath(const std::string &path)
{
	if (path.find("/../") != std::string::npos)
		return true;
	if (path.find("\\../") != std::string::npos)
		return true;
	if (path.find("/..\\") != std::string::npos)
		return true;
	if (path.find("\\..\\") != std::string::npos)
		return true;
	if (path.find(':') != std::string::npos)
		return true;
	return false;
}

static void generateErrorMsg(Pack &pack, bool swrNeeded, const std::vector<std::string> &missingChars, const std::string *version = nullptr)
{
	std::string msg;

	if (version)
		msg = "Mod version >" + *version + " required.";
	else if (swrNeeded && !missingChars.empty()) {
		if (!hasSoku2)
			msg = "Soku2 and SWR are missing";
		else
			msg = "SWR and some characters are missing";
	} else if (!missingChars.empty()) {
		if (!hasSoku2)
			msg = "Soku2 is missing";
		else {
			msg = "Missing characters ";
			for (int i = 0; i < missingChars.size(); i++) {
				if (i != 0)
					msg += ", ";
				msg += "\"" + missingChars[i] + "\"";
			}
		}
	} else if (swrNeeded)
		msg = "SWR is missing";

	pack.error.texture.createFromText(msg.c_str(), defaultFont10, {0x100, 30});
	pack.error.setSize(pack.error.texture.getSize());
	pack.error.rect = {
		0, 0,
		static_cast<int>(pack.error.texture.getSize().x),
		static_cast<int>(pack.error.texture.getSize().y),
	};
	pack.error.tint = SokuLib::DrawUtils::DxSokuColor::Red;
	pack.name.fillColors[SokuLib::DrawUtils::GradiantRect::RECT_BOTTOM_LEFT_CORNER]  = RED_COLOR;
	pack.name.fillColors[SokuLib::DrawUtils::GradiantRect::RECT_BOTTOM_RIGHT_CORNER] = RED_COLOR;
}

static void makeAuthorStr(Pack &pack, const std::string &str)
{
	SokuLib::Vector2i size;

	pack.author.texture.createFromText(("By " + str).c_str(), defaultFont10, {0x100, 14}, &size);
	pack.author.rect = {
		0, 0,
		static_cast<int>(size.x),
		static_cast<int>(size.y),
	};
	pack.author.setSize((size - 1).to<unsigned>());
}

int getVersionFromStr(const std::string &str)
{
	size_t pos = 0;
	int version = 0;

	for (char c : str) {
		if (std::isdigit(c))
			break;
		pos++;
	}

	std::string realStr = str.substr(pos);

	try {
		version |= std::stoul(realStr, &pos);
	} catch (...) {
		throw std::invalid_argument(str + " is not a valid version string (invalid major \"" + realStr + "\")");
	}

	realStr = realStr.substr(pos + 1);
	try {
		version <<= 8;
		version |= std::stoul(realStr, &pos);
	} catch (...) {
		throw std::invalid_argument(str + " is not a valid version string (invalid minor \"" + realStr + "\")");
	}

	realStr = realStr.substr(pos + 1);
	try {
		version <<= 16;
		version |= std::stoul(realStr, &pos);
	} catch (...) {
		throw std::invalid_argument(str + " is not a valid version string (invalid build number \"" + realStr + "\")");
	}
	return version;
}

bool checkVersion(const std::string &version)
{
	if (version == VERSION_STR)
		return true;

	int reqReleaseState = 3 - (version.substr(0, strlen("beta ")) == "beta ") - (version.substr(0, strlen("alpha ")) == "alpha ") * 2;
	int releaseState = 3 - (strncmp(VERSION_STR, "beta ", strlen("beta ")) == 0) - (strncmp(VERSION_STR, "alpha ", strlen("alpha ")) == 0) * 2;

	if (releaseState < reqReleaseState)
		return false;

	int myVersion = getVersionFromStr(VERSION_STR);
	int reqVersion = getVersionFromStr(version);

	return myVersion >= reqVersion;
}

Pack::Pack(const std::string &path, const nlohmann::json &object)
{
	SokuLib::Vector2i size;
	bool swrNeeded = false;
	std::vector<std::string> invalidChars;

	if (!object.is_object()) {
		MessageBox(
			SokuLib::window,
			("Trial pack " + path + " is not valid: pack.json doesn't contain a pack object").c_str(),
			"Trial pack loading error",
			MB_ICONERROR
		);
		return;
	}

	this->path = path;
	this->scorePath = path + "/score.dat";
	this->category = object.contains("category") && object["category"].is_string() ? object["category"] : "no category";

	this->nameStr = object.contains("name") && object["name"].is_string() ? object["name"].get<std::string>() : path;
	this->name.texture.createFromText(
		(this->category + ": " + this->nameStr).c_str(),
		defaultFont12, {0x100, 30}, &size
	);
	this->name.rect = {0, 0, size.x, size.y};
	this->name.setSize((size - 1).to<unsigned>());

	if (object.contains("outro") && object["outro"].is_string()) {
		std::string relative = object["outro"];

		if (!isInvalidPath(relative)) {
			this->outroRelPath = relative;
			this->outroPath = path + "/" + relative;
		}
	}

	if (object.contains("icon")) {
		this->icon = std::make_unique<Icon>(path, object["icon"]);
		if (!this->icon->sprite.texture.hasTexture())
			this->icon.reset();
	}

	if (object.contains("stand") && object["stand"].is_object()) {
		auto &obj = object["stand"];

		if (!obj.contains("path") || !obj["path"].is_string())
			goto invalidPreview;
		if (obj.contains("isPath") && obj["isPath"]) {
			std::string relative = obj["path"];

			if (isInvalidPath(relative)) {
				printf("%s is not a valid path\n", relative.c_str());
				goto invalidPreview;
			}
			this->preview.texture.loadFromFile((path + "/" + (this->previewPath = relative)).c_str());
			this->previewGameAsset = true;
		} else
			this->preview.texture.loadFromGame((this->previewPath = obj["path"]).c_str());
		this->preview.rect = {
			0, 0,
			static_cast<int>(this->preview.texture.getSize().x),
			static_cast<int>(this->preview.texture.getSize().y),
		};
		this->preview.setPosition({398, 128});
		this->preview.setSize({200, 150});
	}

invalidPreview:
	if (object.contains("description") && object["description"].is_string())
		this->descriptionStr = object["description"];
	this->description.texture.createFromText(
		object.contains("description") && object["description"].is_string() ? object["description"].get<std::string>().c_str() : "No description provided",
		defaultFont12, {300, 150}
	);
	this->description.rect = {
		0, 0,
		static_cast<int>(this->description.texture.getSize().x),
		static_cast<int>(this->description.texture.getSize().y),
	};
	this->description.setPosition({356, 286});
	this->description.setSize(this->description.texture.getSize());

	if (object.contains("modes") && object["modes"].is_array()) {
		for (auto &obj : object["modes"]) {
			if (obj.is_string())
				this->modes.push_back(obj);
			else
				puts("A mode is not a string in the pack ?");
		}
	}

	if (object.contains("characters") && object["characters"].is_array()) {
		for (auto &elem : object["characters"]) {
			if (!elem.is_string())
				continue;

			std::string str = elem;

			this->characters.push_back(str);
			if (
				std::find_if(
					swrCharacters.begin(),
					swrCharacters.end(),
					[str](const std::pair<unsigned, std::string> &s) {
						return s.second == str;
					}
				) != swrCharacters.end()
			)
				swrNeeded = SokuLib::SWRUnlinked;
			else if (
				std::find_if(
					validCharacters.begin(),
					validCharacters.end(),
					[str](const std::pair<unsigned, std::string> &s) {
						return s.second == str;
					}
				) == validCharacters.end() &&
				std::find(
					invalidChars.begin(),
					invalidChars.end(),
					str
				) == invalidChars.end()
			)
				invalidChars.push_back(str);
		}
	} else
		MessageBox(
			SokuLib::window,
			("Warning: Trial pack " + path + " does not define the character list used.\n"
			 "If some Soku2 or SWR characters are used, you might encounter some crashes if they are not available.").c_str(),
			"Trial pack warning",
			MB_ICONWARNING
		);

	std::string version = VERSION_STR;

	if (object.contains("min_version") && object["min_version"].is_string())
		version = this->minVersion = object["min_version"];
	try {
		if (!checkVersion(version))
			generateErrorMsg(*this, swrNeeded, invalidChars, &version);
		else if (!invalidChars.empty() || swrNeeded)
			generateErrorMsg(*this, swrNeeded, invalidChars);
		else {
			if (object.contains("author") && object["author"].is_string())
				this->authorStr = object["author"];
			makeAuthorStr(
				*this,
				object.contains("author") && object["author"].is_string() ?
				object["author"].get<std::string>() :
				authors[rand() % authors.size()]
			);
			this->name.fillColors[SokuLib::DrawUtils::GradiantRect::RECT_BOTTOM_LEFT_CORNER] = BLUE_COLOR;
			this->name.fillColors[SokuLib::DrawUtils::GradiantRect::RECT_BOTTOM_RIGHT_CORNER] = BLUE_COLOR;
		}
	} catch (std::exception &e) {
		MessageBox(
			SokuLib::window,
			("Trial pack " + path + " is not valid: pack.json: " + e.what()).c_str(),
			"Trial pack loading error",
			MB_ICONERROR
		);
		return;
	}

	if (!object.contains("scenarios") || !object["scenarios"].is_array()) {
		MessageBox(
			SokuLib::window,
			("Trial pack " + path + " is not valid: pack.json doesn't contain a valid scenarios list").c_str(),
			"Trial pack loading error",
			MB_ICONERROR
		);
		return;
	}

	auto &scenarii = object["scenarios"];
	std::ifstream stream{this->scorePath, std::ifstream::binary};

	for (int i = 0; i < scenarii.size(); i++) {
		char score;

		stream.read(&score, sizeof(score));

		auto scene = new Scenario(score, i, path, scenarii[i]);

		if (stream.fail())
			scene->score = -1;
		if (scene->file.empty()) {
			delete scene;
			MessageBox(
				SokuLib::window,
				("Warning: Trial pack " + path + ": scenario file #" + std::to_string(i) + " is invalid.\n").c_str(),
				"Trial pack warning",
				MB_ICONWARNING
			);
			continue;
		}
		this->scenarios.emplace_back(scene);
		if (this->error.texture.hasTexture())
			scene->name.tint = SokuLib::DrawUtils::DxSokuColor::Red;
	}

	std::map<Scenario *, int> _map;

	for (int i = 0; i < this->scenarios.size(); i++)
		_map[&*this->scenarios[i]] = i;

	std::sort(this->scenarios.begin(), this->scenarios.end(), [&_map](const std::unique_ptr<Scenario> &s1, const std::unique_ptr<Scenario> &s2){
		if (s1->extra != s2->extra)
			return s2->extra;
		return _map[&*s1] < _map[&*s2];
	});
}

Scenario::Scenario(char score, int i, const std::string &path, const nlohmann::json &object)
{
	this->setScore(score);
	if (!object.is_object())
		return;

	if (!object.contains("file") || !object["file"].is_string())
		return;
	this->folder = path + "\\";
	this->file = this->folder + object["file"].get<std::string>();
	this->nameStr = (object.contains("name") && object["name"].is_string() ? object["name"].get<std::string>() : object["file"].get<std::string>());
	this->name.texture.createFromText(
		this->nameStr.c_str(),
		defaultFont10, {0x100, 30}
	);
	this->name.rect = {
		0, 0,
		static_cast<int>(this->name.texture.getSize().x),
		static_cast<int>(this->name.texture.getSize().y),
	};
	this->name.setSize(this->name.texture.getSize());

	this->description.texture.createFromText(
		object.contains("description") && object["description"].is_string() ? object["description"].get<std::string>().c_str() : "No description provided",
		defaultFont12, {300, 150}
	);
	this->description.rect = {
		0, 0,
		static_cast<int>(this->description.texture.getSize().x),
		static_cast<int>(this->description.texture.getSize().y),
	};
	this->description.setPosition({356, 286});
	this->description.setSize(this->description.texture.getSize());

	if (object.contains("preview") && object["preview"].is_string())
		this->previewFile = path + object["preview"].get<std::string>();

	if (object.contains("may_be_locked") && object["may_be_locked"].is_boolean())
		this->canBeLocked = object["may_be_locked"];
	if (object.contains("name_hidden_when_locked") && object["name_hidden_when_locked"].is_boolean())
		this->nameHiddenIfLocked = object["name_hidden_when_locked"];
	if (object.contains("extra") && object["extra"].is_boolean())
		this->extra = object["extra"];
}

void Scenario::setScore(char score)
{
	if (score != -1) {
		if (this->score == -1) {
			this->scoreSprite.texture.loadFromGame("data/infoeffect/result/rankFont.bmp");
			this->scoreSprite.setSize({32, 32});
		}
		this->scoreSprite.rect.left = score * this->scoreSprite.texture.getSize().x / 4;
		this->scoreSprite.rect.width = this->scoreSprite.texture.getSize().x / 4;
		this->scoreSprite.rect.height = this->scoreSprite.texture.getSize().y;
	}
	this->score = score;
}

void loadPreview(Scenario *scenario)
{
	if (scenario->previewFile.size() > 4 && scenario->previewFile.substr(scenario->previewFile.size() - 4, 4) == ".gif")
		scenario->preview = std::make_unique<AnimatedImage>(scenario->previewFile, SokuLib::Vector2i{398, 128}, true);
	else
		scenario->preview = std::make_unique<SimpleImage>(scenario->previewFile, SokuLib::Vector2i{398, 128});
	scenario->loading = false;
	::loading--;
}

void Scenario::loadPreview()
{
	if (this->loading || this->preview)
		return;
	this->loading = true;
	::loading++;
	_beginthread(reinterpret_cast<_beginthread_proc_type>(::loadPreview), 0, this);
}

Icon::Icon(const std::string &path, const nlohmann::json &object)
{
	if (!object.is_object())
		return;

	if (!object.contains("path") || !object["path"].is_string())
		return;
	if (object.contains("offset") && object["offset"].is_object()) {
		auto &off = object["offset"];

		if (off.contains("x") && off["x"].is_number())
			this->translate.x = off["x"];
		if (off.contains("y") && off["y"].is_number())
			this->translate.y = off["y"];
	}
	if (object.contains("scale") && object["scale"].is_number())
		this->scale = object["scale"];

	if (object.contains("xMirror") && object["xMirror"].is_boolean())
		this->mirror.x = object["xMirror"];
	if (object.contains("yMirror") && object["yMirror"].is_boolean())
		this->mirror.y = object["yMirror"];

	this->sprite.setMirroring(this->mirror.x, this->mirror.y);
	if (object.contains("isPath") && object["isPath"]) {
		std::string relative = object["path"];

		if (isInvalidPath(relative)) {
			MessageBox(
				SokuLib::window,
				("Warning: Trial pack " + path + " has an invalid icon path \"" + relative + "\".").c_str(),
				"Trial pack warning",
				MB_ICONWARNING
			);
			return;
		}
		this->path = relative;
		this->sprite.texture.loadFromFile((path + "/" + this->path).c_str());
		this->gameAsset = true;
	} else {
		this->path = object["path"];
		this->sprite.texture.loadFromGame(this->path.c_str());
	}

	this->rect.width = min(68 / this->scale, this->sprite.texture.getSize().x);
	this->rect.height = min(27 / this->scale, this->sprite.texture.getSize().y);
	if (object.contains("rect") && object["rect"].is_object()) {
		auto &rec = object["rect"];

		if (rec.contains("top") && rec["top"].is_number())
			this->rect.top = rec["top"];
		if (rec.contains("left") && rec["left"].is_number())
			this->rect.left = rec["left"];
		if (rec.contains("width") && rec["width"].is_number())
			this->rect.width = min(68 / this->scale, rec["width"].get<float>());
		if (rec.contains("height") && rec["height"].is_number())
			this->rect.height = min(27 / this->scale, rec["height"].get<float>());
	}

	this->sprite.setSize({
		static_cast<unsigned int>(this->rect.width * this->scale),
		static_cast<unsigned int>(this->rect.height * this->scale)
	});
	this->sprite.rect = this->rect;
	this->sprite.fillColors[SokuLib::DrawUtils::GradiantRect::RECT_BOTTOM_LEFT_CORNER] = SokuLib::DrawUtils::DxSokuColor::White * 0.25;
	this->sprite.fillColors[SokuLib::DrawUtils::GradiantRect::RECT_BOTTOM_RIGHT_CORNER]= SokuLib::DrawUtils::DxSokuColor::White * 0.25;
}

void loadPacks()
{
	printf("Loading packs in %s\n", packsLocation);

	WIN32_FIND_DATAA data;
	HANDLE findHandle = FindFirstFileA(packsLocation, &data);
	char buffer[sizeof(packsLocation) + MAX_PATH + 9];
	size_t starPos = strlen(packsLocation) - 1;

	if (findHandle == INVALID_HANDLE_VALUE) {
		MessageBox(
			SokuLib::window,
			("Trial packs location " + std::string(packsLocation) + " is not valid.\n").c_str(),
			"Trial pack loading error",
			MB_ICONERROR
		);
		return;
	}
	strcpy(buffer, packsLocation);
	buffer[starPos] = 0;

	do {
		if (strcmp(data.cFileName, ".") == 0 || strcmp(data.cFileName, "..") == 0)
			continue;
		strcpy(&buffer[starPos], data.cFileName);
		printf("Looking for pack.json in %s\n", buffer);
		strcat(buffer, "\\pack.json");

		std::ifstream stream{buffer};
		nlohmann::json val;

		if (stream.fail()) {
			printf("%s -> Not found...\n", buffer);
			continue;
		}
		try {
			stream >> val;
		} catch (std::exception &e) {
			MessageBox(
				SokuLib::window,
				("File " + std::string(buffer) + " is not valid: " + e.what() + ".\n").c_str(),
				"Trial pack loading error",
				MB_ICONERROR
			);
			buffer[starPos] = 0;
			continue;
		}
		buffer[strlen(buffer) - 9] = 0;

		auto pack = new Pack(std::string(buffer), val);

		loadedPacks.emplace_back(pack);

		packsByName[pack->nameStr].push_back(loadedPacks.back());
		packsByCategory[pack->category].push_back(loadedPacks.back());
		if (std::find(uniqueCategories.begin(), uniqueCategories.end(), pack->category) == uniqueCategories.end())
			uniqueCategories.push_back(pack->category);
		if (std::find(uniqueNames.begin(), uniqueNames.end(), pack->nameStr) == uniqueNames.end())
			uniqueNames.push_back(pack->nameStr);
		for (auto &mode : pack->modes) {
			packsByName[mode].push_back(loadedPacks.back());
			if (std::find(uniqueModes.begin(), uniqueModes.end(), mode) == uniqueModes.end())
				uniqueModes.push_back(mode);
		}
		buffer[starPos] = 0;
	} while (FindNextFileA(findHandle, &data));

	std::sort(loadedPacks.begin(), loadedPacks.end(), [](std::shared_ptr<Pack> pack1, std::shared_ptr<Pack> pack2){
		if (pack1->error.texture.hasTexture() != pack2->error.texture.hasTexture())
			return pack2->error.texture.hasTexture();
		return pack1->category < pack2->category;
	});
	std::sort(uniqueCategories.begin(), uniqueCategories.end());
	std::sort(uniqueNames.begin(), uniqueNames.end());
	std::sort(uniqueModes.begin(), uniqueModes.end());
}
