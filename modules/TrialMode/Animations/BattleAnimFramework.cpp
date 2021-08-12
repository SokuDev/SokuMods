//
// Created by PinkySmile on 12/08/2021.
//

#include "BattleAnimation.hpp"

#define LEFT_TALK_POS    SokuLib::Vector2f{-96, -32}
#define RIGHT_TALK_POS   SokuLib::Vector2f{225, -32}
#define OFF_NOT_TALK_POS SokuLib::Vector2f{19, 20}

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
	puts("Loading dialogs");
	static const char *stands[] = {
		"data/character/%s/stand/\x8a\xf0.bmp",
		"data/character/%s/stand/\x8a\xbe.bmp",
		"data/character/%s/stand/\x8b\xc1.bmp",
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

	desc.r1 = 0xFF;
	desc.g1 = 0xFF;
	desc.b1 = 0xFF;
	desc.r2 = 0xA0;
	desc.g2 = 0xA0;
	desc.b2 = 0xFF;
	desc.height = 20;
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
	desc.weight = FW_BOLD;
	this->_lfont.create();
	this->_lfont.setIndirect(desc);
	desc.r2 = 0xFF;
	desc.b2 = 0xA0;
	this->_rfont.create();
	this->_rfont.setIndirect(desc);

	this->_standDialogBox.setSize({624, 0});
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
		sprite.setMirroring(true, false);
		sprite.rect.width = sprite.texture.getSize().x;
		sprite.rect.height = sprite.texture.getSize().y;
	}

	for (int i = 0; i < dialogLoad.size(); i++) {
		auto dialog = dialogLoad[i];

		printf("Process line %s\n", dialog.c_str());
		if (i == 0) {
			this->_isLeftTalking = (dialog[0] == 'L' || dialog[0] == 'l');
			this->_left = sideStand(dialog[1]);
			this->_right = sideStand(dialog[2]);
			this->_text.texture.createFromText(dialog.c_str() + 3, this->_isLeftTalking ? this->_lfont : this->_rfont, {624, 80});
			this->_text.setSize(this->_text.texture.getSize());
			this->_text.setPosition({16, 400});
			this->_text.rect.width = this->_text.getSize().x;
			this->_text.rect.height = this->_text.getSize().y;
			this->_text.tint.a = 0;
			if (this->_isLeftTalking) {
				this->_leftAlpha = 0xFF;
				this->_rightAlpha = 0xA0;
			} else {
				this->_leftAlpha = 0xA0;
				this->_rightAlpha = 0xFF;
			}
			printf("Current -> %s %s '%c'(%i) '%c'(%i)\n", this->_isLeftTalking ? "Left" : "Right", dialog.c_str() + 3, dialog[1], this->_left, dialog[2], this->_right);

			this->_leftAlphaStep  = ((this->_isLeftTalking ? 0xFF : 0xA0) - this->_leftAlpha ) / 0xF;
			this->_rightAlphaStep = ((this->_isLeftTalking ? 0xA0 : 0xFF) - this->_rightAlpha) / 0xF;
			this->_leftPos   = this->_leftExpectedPos   = LEFT_TALK_POS + (this->_isLeftTalking ? SokuLib::Vector2f{0, 0} : OFF_NOT_TALK_POS);
			this->_rightPos  = this->_rightExpectedPos  = RIGHT_TALK_POS + (this->_isLeftTalking ? OFF_NOT_TALK_POS : SokuLib::Vector2f{0, 0});
			this->_leftPos.x = this->_leftExpectedPos.x = -512;
			this->_rightPos.x= this->_rightExpectedPos.x= 640 + 512;
		} else {
			this->_metaData.emplace_back(dialog.c_str() + 3, dialog[0] == 'L' || dialog[0] == 'l', sideStand(dialog[1]), sideStand(dialog[2]));
			printf(
				"%u -> %s %s '%c'(%i) '%c'(%i)\n",
				this->_metaData.size(),
				std::get<1>(this->_metaData.back()) ? "Left" : "Right",
				std::get<0>(this->_metaData.back()).c_str(),
				dialog[1], std::get<2>(this->_metaData.back()),
				dialog[2], std::get<3>(this->_metaData.back())
				);
		}
	}
}

static void drawStand(SokuLib::DrawUtils::Sprite &stand, float alpha, SokuLib::Vector2i pos)
{
	stand.setPosition(pos);
	stand.tint.a = alpha;
	stand.draw();
}

void SokuStand::render() const
{
	if (this->_left != NOSTAND && !this->_isLeftTalking)
		drawStand(*this->_stands[this->_left  + SPRITE_LEFT_STAND_START],  this->_leftAlpha,  this->_leftPos.to<int>());
	if (this->_right != NOSTAND)
		drawStand(*this->_stands[this->_right + SPRITE_RIGHT_STAND_START], this->_rightAlpha, this->_rightPos.to<int>());
	if (this->_left != NOSTAND && this->_isLeftTalking)
		drawStand(*this->_stands[this->_left  + SPRITE_LEFT_STAND_START],  this->_leftAlpha,  this->_leftPos.to<int>());
	this->_standDialogBox.draw();
	this->_text.draw();
}

void SokuStand::update()
{
	auto leftStep = (this->_leftExpectedPos  - this->_leftPos ) * (1 / this->_leftStep);
	auto rightStep= (this->_rightExpectedPos - this->_rightPos) * (1 / this->_rightStep);
	this->_leftPos  += leftStep;
	this->_rightPos += rightStep;

	if (
		(this->_leftExpectedPos.x < this->_leftPos.x && leftStep.x >= 0) ||
		(this->_leftExpectedPos.x > this->_leftPos.x && leftStep.x <= 0)
		)
		this->_leftPos.x = this->_leftExpectedPos.x;
	if (
		(this->_leftExpectedPos.y < this->_leftPos.y && leftStep.y >= 0) ||
		(this->_leftExpectedPos.y > this->_leftPos.y && leftStep.y <= 0)
		)
		this->_leftPos.y = this->_leftExpectedPos.y;

	if (
		(this->_rightExpectedPos.x < this->_rightPos.x && rightStep.x >= 0) ||
		(this->_rightExpectedPos.x > this->_rightPos.x && rightStep.x <= 0)
		)
		this->_rightPos.x = this->_rightExpectedPos.x;
	if (
		(this->_rightExpectedPos.y < this->_rightPos.y && rightStep.y >= 0) ||
		(this->_rightExpectedPos.y > this->_rightPos.y && rightStep.y <= 0)
		)
		this->_rightPos.y = this->_rightExpectedPos.y;

	this->_leftAlpha  = max(min(this->_leftAlpha  + (char)this->_leftAlphaStep,  0xFF), 0xA0);
	this->_rightAlpha = max(min(this->_rightAlpha + (char)this->_rightAlphaStep, 0xFF), 0xA0);

	if (this->_draw) {
		if (this->_text.tint.a != 0xFF)
			this->_text.tint.a += 0xF;
		if (this->_standDialogBox.getSize().y != 80)
			this->_standDialogBox.setSize({624, this->_standDialogBox.getSize().y + 5});
		return;
	}
	if (this->_text.tint.a)
		this->_text.tint.a -= 0xF;
	if (this->_standDialogBox.getSize().y)
		this->_standDialogBox.setSize({624, this->_standDialogBox.getSize().y - 5});
}

bool SokuStand::onKeyPress()
{
	puts("Press !");
	if (this->_metaData.empty() && this->isAnimationFinished())
		return false;
	if (!this->_draw)
		return true;
	if (this->_metaData.empty())
		return false;
	puts("Next dialog");
	this->_isLeftTalking = std::get<1>(this->_metaData.front());
	this->_text.texture.createFromText(std::get<0>(this->_metaData.front()).c_str(), this->_isLeftTalking ? this->_lfont : this->_rfont, this->_text.texture.getSize());
	this->_text.fillColors[SokuLib::DrawUtils::GradiantRect::RECT_BOTTOM_LEFT_CORNER]  =
		this->_text.fillColors[SokuLib::DrawUtils::GradiantRect::RECT_BOTTOM_RIGHT_CORNER] =
			this->_isLeftTalking ? SokuLib::DrawUtils::DxSokuColor{0xA0, 0xA0, 0xFF} : SokuLib::DrawUtils::DxSokuColor{0xFF, 0xA0, 0xA0};
	this->_leftExpectedPos  = LEFT_TALK_POS + (this->_isLeftTalking ? SokuLib::Vector2f{0, 0} : OFF_NOT_TALK_POS);
	this->_rightExpectedPos = RIGHT_TALK_POS + (this->_isLeftTalking ? OFF_NOT_TALK_POS : SokuLib::Vector2f{0, 0});
	this->_leftStep  = 5;
	this->_rightStep = 5;
	this->_leftAlphaStep  = ((this->_isLeftTalking ? 0xFF : 0xA0) - this->_leftAlpha ) / 0xF;
	this->_rightAlphaStep = ((this->_isLeftTalking ? 0xA0 : 0xFF) - this->_rightAlpha) / 0xF;

	auto left = std::get<2>(this->_metaData.front());
	auto right = std::get<3>(this->_metaData.front());

	if (left == NOSTAND) {
		this->_leftExpectedPos.x = -512;
		this->_leftStep  = 10;
	} else
		this->_leftStep = 5;
	this->_left = left;

	if (right == NOSTAND) {
		this->_rightExpectedPos.x = 640 + 512;
		this->_rightStep = 10;
	} else
		this->_rightStep = 5;
	this->_right = right;

	printf(
		"Current -> %s %s %i(%i) %i(%i) (%f, %f) (%f, %f)\n",
		this->_isLeftTalking ? "Left" : "Right",
		std::get<0>(this->_metaData.front()).c_str(),
		left, this->_left,
		right, this->_right,
		this->_leftExpectedPos.x,
		this->_leftExpectedPos.y,
		this->_rightExpectedPos.x,
		this->_rightExpectedPos.y
		);
	this->_metaData.pop_front();
	return true;
}

unsigned int SokuStand::getCurrentDialog() const
{
	return this->_metaData.size();
}

SokuStand::~SokuStand()
{
	this->_lfont.destruct();
	this->_rfont.destruct();
}

void SokuStand::setHidden(bool hidden)
{
	if (hidden == !this->_draw)
		return;
	printf("Set to%s hidden\n", hidden ? "" : " not");
	if (!hidden) {
		this->_leftExpectedPos = LEFT_TALK_POS + (this->_isLeftTalking ? SokuLib::Vector2f{0, 0} : OFF_NOT_TALK_POS);
		if (this->_left == NOSTAND)
			this->_leftExpectedPos.x = -512;
		this->_rightExpectedPos = RIGHT_TALK_POS + (this->_isLeftTalking ? OFF_NOT_TALK_POS : SokuLib::Vector2f{0, 0});
		if (this->_right == NOSTAND)
			this->_rightExpectedPos.x = 640 + 512;
	} else {
		this->_leftExpectedPos.x  = -512;
		this->_rightExpectedPos.x = 640 + 512;
	}
	this->_leftStep  = 10;
	this->_rightStep = 10;
	this->_draw = !hidden;
}

bool SokuStand::isAnimationFinished() const
{
	auto ldiff = this->_leftPos - this->_leftExpectedPos;
	auto rdiff = this->_rightPos - this->_rightExpectedPos;

	if (std::abs(ldiff.x) > 2 || std::abs(ldiff.y) > 2)
		return false;
	if (std::abs(rdiff.x) > 2 || std::abs(rdiff.y) > 2)
		return false;
	return true;
}

void SokuStand::finishAnimations()
{
	this->_leftPos   = this->_leftExpectedPos;
	this->_rightPos  = this->_rightExpectedPos;
	this->_leftAlpha = this->_isLeftTalking ? 0xFF : 0xA0;
	this->_rightAlpha= this->_isLeftTalking ? 0xA0 : 0xFF;
	this->_text.tint.a = this->_draw ? 0xFF : 0x00;
	this->_standDialogBox.setSize({this->_standDialogBox.getSize().x, this->_draw ? 80U : 0U});
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
