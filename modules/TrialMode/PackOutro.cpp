//
// Created by Gegel85 on 05/11/2021.
//

#include <sol/sol.hpp>
#include "PackOutro.hpp"
#include "Menu.hpp"

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

PackOutro::PackOutro(const std::string &packPath, const std::string &file)
{
	this->_background.texture.loadFromGame("data/scenario/effect/EdBack000.png");
	this->_background.setSize({640, 480});
	this->_background.rect.width = this->_background.texture.getSize().x;
	this->_background.rect.height = this->_background.texture.getSize().y;
	this->_background.tint.a = 0;

	this->_lua.open_libraries(
		sol::lib::base,
		sol::lib::package,
		sol::lib::coroutine,
		sol::lib::string,
		sol::lib::table,
		sol::lib::math,
		sol::lib::debug,
		sol::lib::utf8
	);
	this->_lua["getPackScores"] = getCurrentPackScores;
	this->_lua["packPath"] = packPath;
	this->_lua["playBGM"] = sol::overload(
		SokuLib::playBGM,
		[](const std::string &path, double loopStart, double loopEnd){
			_loopStart = loopStart;
			_loopEnd = loopEnd;
			VirtualProtect((PVOID)0x418cc5, 5, PAGE_EXECUTE_WRITECOPY, &old);
			og = SokuLib::TamperNearJmpOpr(0x418cc5, editLoop);
			SokuLib::playBGM(path.c_str());
		}
	);
	this->_lua["playBgm"] = this->_lua["playBGM"];
	this->_lua["loadBackground"] = [this](const std::string &path){
		auto sprite = new SokuLib::DrawUtils::Sprite{};

		if (!sprite->texture.loadFromFile(path.c_str())) {
			delete sprite;
			throw std::invalid_argument("Cannot load file " + path);
		}
		this->_sprites.emplace_back(sprite);
		sprite->setSize({640, 480});
		sprite->rect.width = sprite->texture.getSize().x;
		sprite->rect.height = sprite->texture.getSize().y;
		sprite->tint.a = 0;
	};
	this->_lua["addCommand"] = [this](const std::string &command){
		this->_commands.push_back(command);
	};
	this->_lua.script_file(file);
}

void PackOutro::draw() const
{
	this->_background.draw();
	for (auto &sprite : this->_sprites)
		sprite->draw();
}

void PackOutro::update()
{
	if (this->_background.tint.a != 0xFF)
		this->_background.tint.a += 0xF;
	for (int i = 0; i < this->_sprites.size(); i++) {
		auto &sprite = *this->_sprites[i];

		if (i == this->_currentSprite) {
			if (sprite.tint.a < 0xFF)
				sprite.tint.a += 0xF;
		} else {
			if (sprite.tint.a)
				sprite.tint.a -= 0xF;
		}
	}
}
