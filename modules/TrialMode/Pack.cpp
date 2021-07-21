//
// Created by PinkySmile on 20/07/2021.
//

#include <fstream>
#include "Pack.hpp"
#include "Menu.hpp"

#define RED_COLOR  SokuLib::DrawUtils::DxSokuColor{0xFF, 0xA0, 0xA0}
#define BLUE_COLOR SokuLib::DrawUtils::DxSokuColor{0xA0, 0xA0, 0xFF}

bool hasSoku2 = false;
std::vector<std::string> uniqueCategories;
std::vector<std::shared_ptr<Pack>> loadedPacks;
std::map<unsigned, std::string> validCharacters{
	{ SokuLib::CHARACTER_REIMU, "reimu" },
	{ SokuLib::CHARACTER_MARISA, "marisa" },
	{ SokuLib::CHARACTER_SAKUYA, "sakuya" },
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

	if (msg.empty())
		PANIC;
	pack.error.texture.createFromText(msg.c_str(), defaultFont8, {0x100, 30});
	pack.error.setSize(pack.error.texture.getSize());
	pack.error.rect = {
		0, 0,
		static_cast<int>(pack.error.texture.getSize().x),
		static_cast<int>(pack.error.texture.getSize().y),
	};
	pack.error.fillColors[SokuLib::DrawUtils::GradiantRect::RECT_BOTTOM_LEFT_CORNER] = RED_COLOR;
	pack.error.fillColors[SokuLib::DrawUtils::GradiantRect::RECT_BOTTOM_RIGHT_CORNER]= RED_COLOR;
	pack.name.fillColors[SokuLib::DrawUtils::GradiantRect::RECT_BOTTOM_LEFT_CORNER]  = RED_COLOR;
	pack.name.fillColors[SokuLib::DrawUtils::GradiantRect::RECT_BOTTOM_RIGHT_CORNER] = RED_COLOR;
}

static void makeAuthorStr(Pack &pack, const std::string &str)
{
	pack.author.texture.createFromText(("By " + str).c_str(), defaultFont8, {0x100, 30});
	pack.author.rect = {
		0, 0,
		static_cast<int>(pack.author.texture.getSize().x),
		static_cast<int>(pack.author.texture.getSize().y),
	};
	pack.author.setSize(pack.author.texture.getSize());
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
	if (!object.contains("scenarios") || !object["scenarios"].is_array()) {
		MessageBox(
			SokuLib::window,
			("Trial pack " + path + " is not valid: pack.json doesn't contain a valid scenarios list").c_str(),
			"Trial pack loading error",
			MB_ICONERROR
		);
		return;
	}

	this->category = object.contains("category") && object["category"].is_string() ? object["category"] : "no category";

	this->_name = object.contains("icon") && object["name"].is_string() ? object["name"].get<std::string>() : path;
	this->name.texture.createFromText(this->_name.c_str(), defaultFont12, {0x100, 32}, &size);
	this->name.rect = {0, 0, size.x, size.y,};
	this->name.setSize((size - 1).to<unsigned>());
	printf("Resulting size: %u %u\n", this->name.texture.getSize().x, this->name.texture.getSize().y);

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
		else if (object.contains("author") && object["author"].is_string()) {
			makeAuthorStr(*this, object["author"]);
			this->name.fillColors[SokuLib::DrawUtils::GradiantRect::RECT_BOTTOM_LEFT_CORNER]  = BLUE_COLOR;
			this->name.fillColors[SokuLib::DrawUtils::GradiantRect::RECT_BOTTOM_RIGHT_CORNER] = BLUE_COLOR;
			if (object.contains("icon")) {
				this->icon = std::make_unique<Icon>(path, object["icon"]);
				if (!this->icon->sprite.texture.hasTexture())
					this->icon.reset();
			}
		}
	} else
		MessageBox(
			SokuLib::window,
			("Warning: Trial pack " + path + " does not define the character list used.\n"
			 "If some Soku2 or SWR characters are used, you might encounter some crashes if they are not available.").c_str(),
			"Trial pack warning",
			MB_ICONWARNING
		);

	auto &scenarios = object["scenarios"];

	for (int i = 0; i < scenarios.size(); i++) {
		auto scene = new Scenario(i, path, scenarios[i]);

		if (scene->file.empty()) {
			delete scene;
			MessageBox(
				SokuLib::window,
				("Warning: Trial pack " + path + ": scenario preview #" + std::to_string(i) + " is invalid.\n").c_str(),
				"Trial pack warning",
				MB_ICONWARNING
			);
		} else
			this->scenarios.emplace_back(scene);
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

	if (std::find(uniqueCategories.begin(), uniqueCategories.end(), this->category) == uniqueCategories.end())
		uniqueCategories.push_back(this->category);
}

Scenario::Scenario(int i, const std::string &path, const nlohmann::json &object)
{
	if (!object.is_object())
		return;

	if (!object.contains("file") || !object["file"].is_string())
		return;
	this->file = object["file"];
	this->name.texture.createFromText(
		(object.contains("name") && object["name"].is_string() ? object["name"].get<std::string>() : "Scenario #" + std::to_string(i)).c_str(),
		defaultFont8, {0x100, 30}
	);
	this->name.rect = {
		0, 0,
		static_cast<int>(this->name.texture.getSize().x),
		static_cast<int>(this->name.texture.getSize().y),
	};
	this->name.setSize(this->name.texture.getSize());

	this->description.texture.createFromText(
		object.contains("description") && object["description"].is_string() ? object["description"].get<std::string>().c_str() : "No description provided",
		defaultFont8, {0x100, 30}
	);
	this->description.rect = {
		0, 0,
		static_cast<int>(this->description.texture.getSize().x),
		static_cast<int>(this->description.texture.getSize().y),
	};
	this->description.setSize(this->description.texture.getSize());

	if (object.contains("preview") && object["preview"].is_string()) {
		this->preview.texture.loadFromFile((path + object["preview"].get<std::string>()).c_str());
		this->preview.rect = {
			0, 0,
			static_cast<int>(this->preview.texture.getSize().x),
			static_cast<int>(this->preview.texture.getSize().y),
		};
		this->preview.setSize(this->preview.texture.getSize());
	}
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
		stream >> val;
		buffer[starPos] = 0;

		auto pack = new Pack(std::string(buffer), val);

		if (!pack->scenarios.empty())
			loadedPacks.emplace_back(pack);
		else {
			puts("Invalid pack");
			delete pack;
		}
	} while (FindNextFileA(findHandle, &data));
}