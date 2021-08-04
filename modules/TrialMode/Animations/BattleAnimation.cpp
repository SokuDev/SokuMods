//
// Created by 2deg and PinkySmile on 20/07/2021.
//

#include <utility>
#include "BattleAnimation.hpp"

#define LEFT_TALK_POS    SokuLib::Vector2i{-96, -32}
#define RIGHT_TALK_POS   SokuLib::Vector2i{225, -32}
#define OFF_NOT_TALK_POS SokuLib::Vector2i{19, 20}

SokuStand::CharStand SokuStand::sideStand(char c)
{
	switch (c) {
	case ' ':
		return NOSTAND;
	case 'H':
		return HAPPY;
	case 'E':
		return EMBARRASSED;
	case 'S':
		return SURPRISED;
	case 'C':
		return CONFIDENT;
	case 'A':
		return ANGRY;
	case 'D':
		return DEFEATED;
	case 'W':
		return WINNING;
	case 'c':
		return CONCERNED;
	default:
		return HAPPY2;
	}
}

inline const char *getChrCodeName(SokuLib::Character chr)
{
	//try {
	//	return validCharacters.at(chr).c_str();
	//} catch (...) {}
	//return swrCharacters.at(chr).c_str();
	return ((const char *(*)(unsigned))0x43f3f0)(chr);
}

SokuStand::SokuStand(std::vector<std::string> dialogLoad)
{
	static const char *stands[] = {
		"data/character/%s/stand/\x8a=.bmp",
		"data/character/%s/stand/\x8a+.bmp",
		"data/character/%s/stand/\x8b-.bmp",
		"data/character/%s/stand/\x8c\x88.bmp",
		"data/character/%s/stand/\x93{.bmp",
		"data/character/%s/stand/\x95\x89.bmp",
		"data/character/%s/stand/\x95\x81.bmp",
		"data/character/%s/stand/\x97].bmp",
		"data/character/%s/stand/\x98\x66.bmp"
	};
	char buffer[256];
	const char *leftChar = getChrCodeName(SokuLib::leftChar);
	const char *rightChar = getChrCodeName(SokuLib::rightChar);
	SokuLib::FontDescription desc;

	desc.r1 = 255;
	desc.r2 = 255;
	desc.g1 = 255;
	desc.g2 = 255;
	desc.b1 = 255;
	desc.b2 = 255;
	desc.height = 18;
	desc.weight = FW_NORMAL;
	desc.italic = 0;
	desc.shadow = 1;
	desc.bufferSize = 1000000;
	desc.charSpaceX = 0;
	desc.charSpaceY = 0;
	desc.offsetX = 0;
	desc.offsetY = 0;
	desc.useOffset = 0;
	strcpy(desc.faceName, "MonoSpatialModSWR");
	desc.weight = FW_REGULAR;
	this->_font16.create();
	this->_font16.setIndirect(desc);

	this->_standDialogBox.setSize({624, 80});
	this->_standDialogBox.setPosition({8, 392});
	this->_standDialogBox.setFillColor(SokuLib::DrawUtils::DxSokuColor{0x20, 0x20, 0x20, 0xA0});
	this->_standDialogBox.setBorderColor(SokuLib::DrawUtils::DxSokuColor::Transparent);

	for (auto stand : stands) {
		this->_stands.emplace_back(new SokuLib::DrawUtils::Sprite());

		auto &sprite = *this->_stands.back();

		sprintf(buffer, stand, leftChar);
		sprite.texture.loadFromGame(buffer);
		sprite.setSize(sprite.texture.getSize());
		sprite.rect.width = sprite.texture.getSize().x;
		sprite.rect.height = sprite.texture.getSize().y;
	}
	for (auto stand : stands) {
		this->_stands.emplace_back(new SokuLib::DrawUtils::Sprite());

		auto &sprite = *this->_stands.back();

		sprintf(buffer, stand, rightChar);
		sprite.texture.loadFromGame(buffer);
		sprite.setSize(sprite.texture.getSize());
		sprite.rect.width = sprite.texture.getSize().x;
		sprite.rect.height = sprite.texture.getSize().y;
	}

	for (int i = 0; i < dialogLoad.size(); i++) {
		auto dialog = dialogLoad[i];

		if (i == 0) {
			this->_isLeftTalking = (dialog[0] == 'L' || dialog[0] == 'l');
			this->_left = sideStand(dialog[1]);
			this->_right = sideStand(dialog[2]);
			this->_text.texture.createFromText(dialog.c_str() + 3, this->_font16, {624, 80});
			this->_text.setSize(this->_text.texture.getSize());
			this->_text.setPosition({8, 392});
			if (this->_isLeftTalking) {
				if (this->_right != NOSTAND)
					this->_stands[this->_right + SPRITE_RIGHT_STAND_START]->tint.a = 0xA0;
			} else if (this->_left != NOSTAND)
				this->_stands[this->_left + SPRITE_LEFT_STAND_START]->tint.a = 0xA0;

			if (this->_left != NOSTAND)
				this->_stands[this->_left + SPRITE_LEFT_STAND_START]->setPosition(LEFT_TALK_POS + (this->_isLeftTalking ? SokuLib::Vector2i{0, 0} : OFF_NOT_TALK_POS));
			if (this->_right != NOSTAND)
				this->_stands[this->_right + SPRITE_RIGHT_STAND_START]->setPosition(RIGHT_TALK_POS + (this->_isLeftTalking ? OFF_NOT_TALK_POS : SokuLib::Vector2i{0, 0}));
		} else
			this->_metaData.emplace_back(dialog.c_str() + 2, dialog[0] == 'L' || dialog[0] == 'l', sideStand(dialog[1]));
	}
}

class ExternalBattleAnimation : public BattleAnimation {
private:
	HMODULE _module;
	BattleAnimation *_real;

public:
	ExternalBattleAnimation(HMODULE module, BattleAnimation *val) :
		_module(module),
		_real(val)
	{
	}

	~ExternalBattleAnimation() override
	{
		delete this->_real;
		FreeLibrary(this->_module);
	}

	bool update() override
	{
		return this->_real->update();
	}

	void render() const override
	{
		this->_real->render();
	}

	void onKeyPressed() override
	{
		this->_real->onKeyPressed();
	}
};

std::string getLastErrorAsString(DWORD errorMessageID)
{
	if (errorMessageID == 0) {
		return nullptr;
	}

	LPSTR messageBuffer = nullptr;

	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,
		errorMessageID,
		MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
		reinterpret_cast<LPSTR>(&messageBuffer),
		0,
		nullptr
	);

	std::string str = messageBuffer;

	LocalFree(messageBuffer);
	return str;
}

ExternalBattleAnimation *loadBattleAnimationFromExternalModule(const char *path, bool isIntro)
{
	HMODULE handle = LoadLibraryA(path);

	if (handle == nullptr)
		throw std::invalid_argument("LoadLibrary failed with code " + std::to_string(GetLastError()) + ": " + getLastErrorAsString(GetLastError()));

	std::string name = isIntro ? "getIntro" : "getOutro";
	auto fct = reinterpret_cast<BattleAnimation *(*)()>(GetProcAddress(handle, name.c_str()));

	if (!fct) {
		FreeLibrary(handle);
		throw std::invalid_argument("GetProcAddress(\"" + name + "\") failed with code " + std::to_string(GetLastError()) + ": " + getLastErrorAsString(GetLastError()));
	}

	BattleAnimation *val = fct();

	if (!val) {
		FreeLibrary(handle);
		throw std::invalid_argument(name + " returned nullptr");
	}
	return new ExternalBattleAnimation(handle, val);
}

BattleAnimation *loadBattleAnimation(const char *path, bool isIntro)
{
	if (!path)
		return nullptr;
	if (strlen(path) >= strlen(".script") && strcmp(&path[strlen(path) - strlen(".script")], ".script") == 0)
		throw std::invalid_argument("Battle animation scripts are not implemented for now.");
	if (strlen(path) >= strlen(".dll") && strcmp(&path[strlen(path) - strlen(".dll")], ".dll") == 0)
		return loadBattleAnimationFromExternalModule(path, isIntro);
	throw std::invalid_argument("Unknown");
}

void SokuStand::render() const
{
	if (!this->_draw)
		return;

	if (this->_left != NOSTAND && !this->_isLeftTalking)
		this->_stands[this->_left  + SPRITE_LEFT_STAND_START]->draw();
	if (this->_right != NOSTAND)
		this->_stands[this->_right + SPRITE_RIGHT_STAND_START]->draw();
	if (this->_left != NOSTAND && this->_isLeftTalking)
		this->_stands[this->_left  + SPRITE_LEFT_STAND_START]->draw();
	this->_standDialogBox.draw();
	this->_text.draw();
}

void SokuStand::update()
{
	if (this->_left != NOSTAND) {
		if (this->_isLeftTalking && this->_stands[this->_left + SPRITE_LEFT_STAND_START]->tint.a != 0xFF)
			this->_stands[this->_left + SPRITE_LEFT_STAND_START]->tint.a += 5;
		else if (!this->_isLeftTalking && this->_stands[this->_left + SPRITE_LEFT_STAND_START]->tint.a != 0xA0)
			this->_stands[this->_left + SPRITE_LEFT_STAND_START]->tint.a -= 5;
	}
	if (this->_right != NOSTAND) {
		if (!this->_isLeftTalking && this->_stands[this->_right + SPRITE_RIGHT_STAND_START]->tint.a != 0xFF)
			this->_stands[this->_right + SPRITE_RIGHT_STAND_START]->tint.a += 5;
		else if (this->_isLeftTalking && this->_stands[this->_right + SPRITE_RIGHT_STAND_START]->tint.a != 0xA0)
			this->_stands[this->_right + SPRITE_RIGHT_STAND_START]->tint.a -= 5;
	}
}

bool SokuStand::onKeyPress()
{
	if (!this->_draw) {
		this->_draw = true;
		return true;
	}
	if (this->_metaData.empty())
		return false;
	this->_text.texture.createFromText(std::get<0>(this->_metaData.front()).c_str(), this->_font16, this->_text.texture.getSize());
	this->_isLeftTalking = std::get<1>(this->_metaData.front());
	(this->_isLeftTalking ? this->_left : this->_right) = std::get<2>(this->_metaData.front());
	this->_text.fillColors[SokuLib::DrawUtils::GradiantRect::RECT_BOTTOM_LEFT_CORNER]  =
	this->_text.fillColors[SokuLib::DrawUtils::GradiantRect::RECT_BOTTOM_RIGHT_CORNER] =
	this->_isLeftTalking ? SokuLib::DrawUtils::DxSokuColor{0xA0, 0xA0, 0xFF} : SokuLib::DrawUtils::DxSokuColor{0xFF, 0xA0, 0xA0};
	if (this->_left != NOSTAND)
		this->_stands[this->_left + SPRITE_LEFT_STAND_START]->setPosition(LEFT_TALK_POS + (this->_isLeftTalking ? SokuLib::Vector2i{0, 0} : OFF_NOT_TALK_POS));
	if (this->_right != NOSTAND)
		this->_stands[this->_right + SPRITE_RIGHT_STAND_START]->setPosition(RIGHT_TALK_POS + (this->_isLeftTalking ? OFF_NOT_TALK_POS : SokuLib::Vector2i{0, 0}));
	this->_metaData.pop_front();
	return true;
}

unsigned int SokuStand::getCurrentDialog() const
{
	return this->_metaData.size();
}

SokuStand::~SokuStand()
{
	this->_font16.destruct();
}

MovingSprite::MovingSprite(
	void (*loader)(MovingSprite &),
	SokuLib::Vector2i pos,
	SokuLib::Camera *camera,
	SokuLib::Vector2u size,
	SokuLib::DrawUtils::TextureRect rect,
	float rotation,
	SokuLib::Vector2i speed,
	float angularSpeed
) :
	_speed(speed),
	_angularSpeed(angularSpeed)
{
	loader(*this);
	this->setPosition(pos);
	this->setCamera(camera);
	this->setSize({
		size.x ? size.x : this->texture.getSize().x,
		size.y ? size.y : this->texture.getSize().y,
	});
	this->rect = rect;
	this->setRotation(rotation);
}

void MovingSprite::update()
{
	this->setPosition(this->getPosition() + this->_speed);
	this->setRotation(this->getRotation() + this->_angularSpeed);
}

void AnimationLoop::update()
{
	//FUN_0043a250
}
