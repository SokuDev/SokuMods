//
// Created by PinkySmile on 05/11/2021.
//

#include <memory>
#include <sol/sol.hpp>
#include <fstream>
#include "PackOutro.hpp"
#include "Menu.hpp"
#include "Patches.hpp"

static DWORD old;
static double _loopStart, _loopEnd;
static void (__stdcall *og)(int);

static void __stdcall editLoop(int ptr) {
	//int samplePerSec = *reinterpret_cast<int*>(ptr+0x12fc);
	*reinterpret_cast<double*>(ptr+0x12e8) = _loopStart;
	*reinterpret_cast<double*>(ptr+0x12f0) = _loopEnd;
	SokuLib::TamperNearJmpOpr(0x418cc5, og);
	VirtualProtect((PVOID)0x418cc5, 5, old, &old);
}

const std::map<std::string, void (PackOutro::*)(const std::string &args)> PackOutro::_commandsCallbacks{
	{ "end",                   &PackOutro::_endCmd },
	{ "text",                  &PackOutro::_textCmd },
	{ "clear",                 &PackOutro::_clearCmd },
	{ "color",                 &PackOutro::_colorCmd },
	{ "playBGM",               &PackOutro::_setBGMCmd },
	{ "background",            &PackOutro::_backgroundCmd },
	{ "playAnimation",         &PackOutro::_playAnimCmd },
	{ "playGameAnimation",     &PackOutro::_initOpeningObject },
	{ "generateGameAnimation", &PackOutro::_generateAnimation }
};

PackOutro::PackOutro(const std::string &packPath, const std::string &file) :
	_packPath(packPath)
{
	size_t pos;

	while ((pos = this->_packPath.find('\\')) != std::string::npos)
		this->_packPath[pos] = '/';

	this->_background.texture.loadFromGame("data/scenario/effect/EdBack000.png");
	this->_background.setSize({640, 480});
	this->_background.rect.width = this->_background.texture.getSize().x;
	this->_background.rect.height = this->_background.texture.getSize().y;
	this->_background.tint.a = 255;

	this->_rect.setSize({640, 480});
	this->_rect.setFillColor(SokuLib::Color::Black);
	this->_rect.setBorderColor(SokuLib::Color::Transparent);

	this->_lua = new sol::state{};
	this->_lua->open_libraries(
		sol::lib::base,
		sol::lib::package,
		sol::lib::coroutine,
		sol::lib::string,
		sol::lib::table,
		sol::lib::math,
		sol::lib::debug,
		sol::lib::utf8
	);
	(*this->_lua)["getPackScores"] = getCurrentPackScores;
	(*this->_lua)["packPath"] = packPath;
	(*this->_lua)["loadAnimation"] = [this](const std::string &path) {
		this->_animation = std::make_unique<LuaBattleAnimation>(this->_packPath.c_str(), path.c_str());
	};
	(*this->_lua)["loadBackground"] = [this](const std::string &path){
		auto sprite = new SokuLib::DrawUtils::Sprite{};
		bool result = path.find(':') == std::string::npos ?
			sprite->texture.loadFromGame(path.c_str()) :
			sprite->texture.loadFromFile(path.c_str());

		this->_sprites.emplace_back(sprite);
		sprite->setPosition({94, 15});
		sprite->setSize({480, 368});
		sprite->rect.width = sprite->texture.getSize().x;
		sprite->rect.height = sprite->texture.getSize().y;
		if (this->_sprites.size() > 1)
			sprite->tint.a = 0;
		return result;
	};
	(*this->_lua)["addCommand"] = [this](const std::string &command){
		this->_commands.push_back(command);
	};
	this->_lua->script_file(file);
}

PackOutro::~PackOutro()
{
	delete this->_lua;
	this->_destroyOpeningObject();
}

void PackOutro::draw() const
{
	this->_rect.draw();
	this->_background.draw();
	for (auto &sprite : this->_sprites)
		sprite->draw();
	for (auto &sprite : this->_text)
		sprite->draw();
	if (this->_hasOpeningObject) {
		auto FUN_004571d0 = reinterpret_cast<int (__thiscall *)(const void *)>(0x4571d0);

		FUN_004571d0(this->_openingObject);
	}
	if (this->_animation && this->_playingAnimation)
		this->_animation->render();
}

bool PackOutro::update()
{
	bool anim = false;
	auto FUN_00457890 = reinterpret_cast<int (__thiscall *)(void *)>(0x457890);

	if (this->_hasOpeningObject)
		FUN_00457890(this->_openingObject);
	if (this->_animation && this->_playingAnimation) {
		auto result = this->_animation->update();

		if (SokuLib::inputMgrs.input.a == 1 || SokuLib::inputMgrs.input.b)
			this->_animation->onKeyPressed();
		if (!result)
			this->_animation.reset();
	}
	if (this->_finished)
		return false;
	if (this->_wait) {
		if (this->_commands[this->_currentCommand].substr(0, strlen("playBGM")) == "playBGM")
			this->_processNextCommand();
		this->_wait--;
		return true;
	}
	if (this->_background.tint.a && this->_ended) {
		this->_background.tint.a -= 0x5;
		anim = true;
	}
	for (int i = 0; i < this->_sprites.size(); i++) {
		auto &sprite = *this->_sprites[i];

		if (i == this->_currentSprite && !this->_ended) {
			if (sprite.tint.a < 0xFF) {
				sprite.tint.a += 0x5;
				anim = true;
			}
		} else {
			if (sprite.tint.a) {
				sprite.tint.a -= 0x5;
				anim = true;
			}
		}
	}
	if (anim)
		return true;
	if (!this->_text.empty() && this->_text.back()->tint.a != 0xFF) {
		this->_text.back()->tint.a += 0x5;
		return true;
	}
	if (this->_currentCommand == this->_commands.size()) {
		if (SokuLib::inputMgrs.input.a != 1)
			return true;
		this->_finished = true;
		return false;
	}
	if (!this->_text.empty() && this->_text.back()->tint.a != 0xFF) {
		this->_text.back()->tint.a += 0x5;
		return true;
	}
	if (this->_textWait) {
		this->_textWait--;
		return true;
	}
	this->_processNextCommand();
	return this->update();
}

void PackOutro::_processNextCommand()
{
	auto cmdLine = this->_commands[this->_currentCommand++];
	auto pos = cmdLine.find(' ');
	auto cmd = cmdLine.substr(0, pos);
	auto args = pos == std::string::npos ? "" : cmdLine.substr(pos + 1);
	auto it = PackOutro::_commandsCallbacks.find(cmd);

	if (it == PackOutro::_commandsCallbacks.end())
		throw std::invalid_argument(cmd + " is not a recognized command.");
	printf("Execute command %s %s\n", cmd.c_str(), args.c_str());
	return (this->*it->second)(args);
}

void PackOutro::_endCmd(const std::string &args)
{
	this->_ended = true;
	this->_clearCmd(args);
}

void PackOutro::_textCmd(const std::string &args)
{
	auto sprite = new SokuLib::DrawUtils::Sprite{};
	SokuLib::Vector2i size;

	this->_text.emplace_back(sprite);
	sprite->texture.createFromText(args.c_str(), defaultFont12, {640, 20}, &size);
	sprite->rect.width = size.x;
	sprite->rect.height = size.y;
	sprite->setSize(size.to<unsigned>());
	sprite->setPosition({30, 372 + 20 * static_cast<int>(this->_text.size())});
	sprite->fillColors[SokuLib::DrawUtils::GradiantRect::RECT_TOP_LEFT_CORNER]     = this->_colorUp;
	sprite->fillColors[SokuLib::DrawUtils::GradiantRect::RECT_TOP_RIGHT_CORNER]    = this->_colorUp;
	sprite->fillColors[SokuLib::DrawUtils::GradiantRect::RECT_BOTTOM_LEFT_CORNER]  = this->_colorDown;
	sprite->fillColors[SokuLib::DrawUtils::GradiantRect::RECT_BOTTOM_RIGHT_CORNER] = this->_colorDown;
	sprite->tint.a = 0;
	this->_textWait = 120;
}

void PackOutro::_clearCmd(const std::string &args)
{
	this->_text.clear();
}

void PackOutro::_colorCmd(const std::string &args)
{
	auto pos = args.find(' ');

	if (pos == std::string::npos) {
		try {
			auto color = std::stoul(args, nullptr, 16);

			this->_colorDown.r = color >> 16 & 0xFF;
			this->_colorDown.g = color >> 8 & 0xFF;
			this->_colorDown.b = color >> 0 & 0xFF;
			this->_colorDown.a = 0xFF;
			this->_colorUp.color = this->_colorDown.color;
		} catch (...) {
			throw std::invalid_argument(args + " is not a valid hexadecimal number");
		}
		return;
	}
	try {
		auto color = std::stoul(args.substr(0, pos), nullptr, 16);

		this->_colorDown.r = color >> 16 & 0xFF;
		this->_colorDown.g = color >> 8 & 0xFF;
		this->_colorDown.b = color >> 0 & 0xFF;
		this->_colorDown.a = 0xFF;
	} catch (...) {
		throw std::invalid_argument(args.substr(0, pos - 1) + " is not a valid hexadecimal number");
	}
	try {
		auto color = std::stoul(args.substr(pos + 1), nullptr, 16);

		this->_colorUp.r = color >> 16 & 0xFF;
		this->_colorUp.g = color >> 8 & 0xFF;
		this->_colorUp.b = color >> 0 & 0xFF;
		this->_colorUp.a = 0xFF;
	} catch (...) {
		throw std::invalid_argument(args.substr(pos + 1) + " is not a valid hexadecimal number");
	}
}

void PackOutro::_backgroundCmd(const std::string &args)
{
	try {
		this->_currentSprite = std::stoul(args);
	} catch (...) {
		throw std::invalid_argument(args + " is not a valid background number.");
	}
	this->_clearCmd(args);
}

void PackOutro::_setBGMCmd(const std::string &args)
{
	std::string path;
	std::string arg = args;
	size_t pos;
	bool f = false;

	if (arg[0] != '"')
		pos = arg.find(' ');
	else {
		arg.erase(arg.begin());
		pos = arg.find('"');
		f = true;
	}

	path = arg.substr(0, pos);
	if (pos + 1 + f >= arg.size())
		return SokuLib::playBGM(path.c_str());
	arg = arg.substr(pos + 1 + f);
	pos = arg.find(' ');
	try {
		_loopStart = std::stof(arg.substr(0, pos));
	} catch (...) {
		throw std::invalid_argument(arg.substr(0, pos) + " is not a valid floating number");
	}
	try {
		_loopEnd = std::stof(arg.substr(pos + 1));
	} catch (...) {
		throw std::invalid_argument(arg.substr(pos + 1) + " is not a valid floating number");
	}
	VirtualProtect((PVOID)0x418cc5, 5, PAGE_EXECUTE_WRITECOPY, &old);
	og = SokuLib::TamperNearJmpOpr(0x418cc5, editLoop);
	SokuLib::playBGM(path.c_str());
}

void PackOutro::_playAnimCmd(const std::string &args)
{
	this->_playingAnimation = true;
	//this->_animation = std::make_unique<LuaBattleAnimation>(this->_packPath.c_str(), args.c_str());
}

void PackOutro::_initOpeningObject(const std::string &_file)
{
	size_t pos;
	std::string file = _file;
	auto FUN_0041f270 = reinterpret_cast<void (__thiscall *)(void *)>(0x41f270);
	auto FUN_0045a110 = reinterpret_cast<void (__thiscall *)(void *)>(0x45a110);
	auto FUN_0045a660 = reinterpret_cast<void (__thiscall *)(void *, const char *)>(0x45a660);

	while ((pos = file.find('\\')) != std::string::npos)
		file[pos] = '/';
	this->_destroyOpeningObject();
	FUN_0041f270(&this->_openingObject);
	FUN_0045a110(&this->_openingObject);
	applyFileSystemPatch();
	printf("Loading opening animation %s\n", file.c_str());
	FUN_0045a660(&this->_openingObject, file.c_str());
	this->_hasOpeningObject = true;
}

void PackOutro::_destroyOpeningObject()
{
	if (!this->_hasOpeningObject)
		return;

	auto FUN_004593b0 = reinterpret_cast<void (__thiscall *)(void *)>(0x4593b0);
	auto FUN_0041f340 = reinterpret_cast<void (__thiscall *)(void *)>(0x41f340);

	FUN_004593b0(&this->_openingObject);
	FUN_0041f340(&this->_openingObject);
	removeFileSystemPatch();
	this->_hasOpeningObject = false;
}

void PackOutro::_generateAnimation(const std::string &args)
{
	std::ifstream istream{args, std::fstream::binary};
	std::ofstream ostream;
	std::string other = args;
	std::string line;
	auto pos = other.find_last_of('.');
	uint8_t a = 0x8b;
	uint8_t b = 0x71;

	printf("Opening animation %s\n", args.c_str());
	if (istream.fail())
		throw std::invalid_argument("Cannot open " + args);
	if (pos != std::string::npos)
		other = other.substr(0, pos);
	other += "_generated.cv0";
	printf("Opening result file %s\n", other.c_str());
	ostream.open(other, std::fstream::binary);
	if (ostream.fail())
		throw std::invalid_argument("Cannot open " + other);
	while (std::getline(istream, line)) {
		for (pos = line.find("{{pack_path}}"); pos != std::string::npos; pos = line.find("{{pack_path}}"))
			line = line.substr(0, pos) + this->_packPath + line.substr(pos + 13);
		printf("%s\n", line.c_str());
		for (char c : line) {
			c ^= a;
			ostream.write(&c, 1);
			a += b;
			b -= 0x6b;
		}

		char c = '\n' ^ a;

		ostream.write(&c, 1);
		a += b;
		b -= 0x6b;
	}
	puts("Closing files");
	ostream.close();
	istream.close();
}
