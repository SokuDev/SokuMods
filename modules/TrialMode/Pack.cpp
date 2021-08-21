//
// Created by PinkySmile on 20/07/2021.
//

#include <fstream>
#include "Pack.hpp"
#include "Menu.hpp"

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
	"Baka Cirno",
	"Illuminated Catfish",
	"Fun frog that never wakes back up",
	"Marisa's tongue",
	"The great wall",
	"Spinning Ran",
	"Hieda no Akyuu",
	"Book stealing rats",
	"Sakuya the cat",
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

static void generateErrorMsg(Pack &pack, bool swrNeeded, const std::vector<std::string> &missingChars)
{
	std::string msg;

	if (swrNeeded && !missingChars.empty()) {
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

Pack::Pack(const std::string &path, const nlohmann::json &object)
{
	SokuLib::Vector2i size;

	if (!object.is_object()) {
		MessageBox(
			SokuLib::window,
			("Trial pack " + path + " is not valid: pack.json doesn't contain a pack object").c_str(),
			"Trial pack loading error",
			MB_ICONERROR
		);
		return;
	}

	this->scorePath = path + "/score.dat";
	this->category = object.contains("category") && object["category"].is_string() ? object["category"] : "no category";

	this->nameStr = object.contains("name") && object["name"].is_string() ? object["name"].get<std::string>() : path;
	this->name.texture.createFromText(
		(this->category + ": " + this->nameStr).c_str(),
		defaultFont12, {0x100, 30}, &size
	);
	this->name.rect = {0, 0, size.x, size.y};
	this->name.setSize((size - 1).to<unsigned>());

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

			if (!isInvalidPath(relative)) {
				printf("%s is not a valid path\n", relative.c_str());
				goto invalidPreview;
			}
			this->preview.texture.loadFromFile((path + "/" + relative).c_str());
		} else
			this->preview.texture.loadFromGame(obj["path"].get<std::string>().c_str());
		this->preview.rect = {
			0, 0,
			static_cast<int>(this->preview.texture.getSize().x),
			static_cast<int>(this->preview.texture.getSize().y),
		};
		this->preview.setPosition({398, 128});
		this->preview.setSize({200, 150});
	}

invalidPreview:
	this->description.texture.createFromText(
		object.contains("description") && object["description"].is_string() ? object["description"].get<std::string>().c_str() : "No description provided",
		defaultFont12, {300, 150}
	);
	this->description.rect = {
		0, 0,
		static_cast<int>(this->description.texture.getSize().x),
		static_cast<int>(this->description.texture.getSize().y),
	};
	this->description.setPosition({356, 280});
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
		bool swrNeeded = false;
		std::vector<std::string> invalidChars;

		for (auto &elem : object["characters"]) {
			if (!elem.is_string())
				continue;

			std::string str = elem;

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

		if (!invalidChars.empty() || swrNeeded)
			generateErrorMsg(*this, swrNeeded, invalidChars);
		else {
			makeAuthorStr(
				*this,
				object.contains("author") && object["author"].is_string() ?
					object["author"].get<std::string>() :
					authors[rand() % authors.size()]
			);
			this->name.fillColors[SokuLib::DrawUtils::GradiantRect::RECT_BOTTOM_LEFT_CORNER]  = BLUE_COLOR;
			this->name.fillColors[SokuLib::DrawUtils::GradiantRect::RECT_BOTTOM_RIGHT_CORNER] = BLUE_COLOR;
		}
	} else
		MessageBox(
			SokuLib::window,
			("Warning: Trial pack " + path + " does not define the character list used.\n"
			 "If some Soku2 or SWR characters are used, you might encounter some crashes if they are not available.").c_str(),
			"Trial pack warning",
			MB_ICONWARNING
		);

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

		stream.read(&score, 1);

		auto scene = new Scenario(score, i, path, scenarii[i]);

		if (stream.fail())
			scene->score = -1;
		if (scene->file.empty()) {
			delete scene;
			MessageBox(
				SokuLib::window,
				("Warning: Trial pack " + path + ": scenario preview #" + std::to_string(i) + " is invalid.\n").c_str(),
				"Trial pack warning",
				MB_ICONWARNING
			);
			continue;
		}
		this->scenarios.emplace_back(scene);
		if (this->error.texture.hasTexture())
			scene->name.tint = SokuLib::DrawUtils::DxSokuColor::Red;
	}
	if (this->scenarios.empty()) {
		MessageBox(
			SokuLib::window,
			("Trial pack " + path + " has no valid scenario and will be discarded.\n").c_str(),
			"Trial pack loading error",
			MB_ICONERROR
		);
		return;
	}
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
	this->nameStr = (object.contains("name") && object["name"].is_string() ? object["name"].get<std::string>() : "Scenario #" + std::to_string(i));
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
	this->description.setPosition({356, 280});
	this->description.setSize(this->description.texture.getSize());

	if (object.contains("preview") && object["preview"].is_string()) {
		auto str = path + object["preview"].get<std::string>();

		if (str.size() > 4 && str.substr(str.size() - 4, 4) == ".gif")
			this->preview.reset(new AnimatedImage(str, {398, 128}));
		else
			this->preview.reset(new SimpleImage(str, {398, 128}));
	}

	if (object.contains("may_be_locked") && object["may_be_locked"].is_boolean())
		this->canBeLocked = object["may_be_locked"];
	if (object.contains("name_hidden_when_locked") && object["name_hidden_when_locked"].is_boolean())
		this->nameHiddenIfLocked = object["name_hidden_when_locked"];
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

Icon::Icon(const std::string &path, const nlohmann::json &object)
{
	float scale = 1;
	SokuLib::DrawUtils::TextureRect rect = {0, 0, 0, 0};
	SokuLib::Vector2<bool> mirror{false, false};

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
		scale = object["scale"];

	if (object.contains("xMirror") && object["xMirror"].is_boolean())
		mirror.x = object["xMirror"];
	if (object.contains("yMirror") && object["yMirror"].is_boolean())
		mirror.y = object["yMirror"];

	this->sprite.setMirroring(mirror.x, mirror.y);
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
		this->sprite.texture.loadFromFile((path + "/" + relative).c_str());
	} else
		this->sprite.texture.loadFromGame(object["path"].get<std::string>().c_str());

	rect.width = min(68 / scale, this->sprite.texture.getSize().x);
	rect.height = min(27 / scale, this->sprite.texture.getSize().y);
	if (object.contains("rect") && object["rect"].is_object()) {
		auto &rec = object["rect"];

		if (rec.contains("top") && rec["top"].is_number())
			rect.top = rec["top"];
		if (rec.contains("left") && rec["left"].is_number())
			rect.left = rec["left"];
		if (rec.contains("width") && rec["width"].is_number())
			rect.width = min(68 / scale, rec["width"].get<float>());
		if (rec.contains("height") && rec["height"].is_number())
			rect.height = min(27 / scale, rec["height"].get<float>());
	}

	this->sprite.setSize({
		static_cast<unsigned int>(rect.width * scale),
		static_cast<unsigned int>(rect.height * scale)
	});
	this->sprite.rect = rect;
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
			printf("%s -> Not found...", buffer);
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

		if (!pack->scenarios.empty()) {
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
		} else {
			puts("Invalid pack :(");
			delete pack;
		}
		buffer[starPos] = 0;
	} while (FindNextFileA(findHandle, &data));
}