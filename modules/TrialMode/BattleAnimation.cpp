//
// Created by 2deg and PinkySmile on 20/07/2021.
//

#include <memory>
#include <utility>
#include "BattleAnimation.hpp"
#include "Menu.hpp"
#include "Pack.hpp"

//MovingSprite Function (manage loading and rendering all sprite individually)

MovingSprite::MovingSprite(const char *path, bool *success)
{
	bool result = this->sprite.texture.loadFromGame(path);

	if (success)
		*success = result;
}

MovingSprite::MovingSprite(const char *text, SokuLib::SWRFont &font, bool *success)
{
	SokuLib::Vector2i buffer;

	bool result = this->sprite.texture.createFromText(text, font, {1300, 60}, &buffer);
	this->sprite.setSize(buffer.to<unsigned>());
	this->sprite.rect.width = buffer.x;
	this->sprite.rect.height = buffer.y;

	if (success)
		*success = result;
}



const SokuLib::DrawUtils::Sprite &MovingSprite::render(CharacterSide characterFollowed,
						       SokuLib::Vector2u spriteSize,
						       SokuLib::Vector2f positionOffset,
						       const SokuLib::Camera *camera,
						       SokuLib::Vector2u textureSize,
						       SokuLib::Vector2b mirror,
						       float rotation) const
{
	this->sprite.setCamera(camera);

	this->sprite.setMirroring(mirror.x, mirror.y);
	if (characterFollowed == NONE){
		this->sprite.setPosition(positionOffset.to<int>());
	} else {
		if (characterFollowed == LEFTCHAR) {
			this->sprite.setPosition(SokuLib::Vector2i{
				static_cast<int>(SokuLib::getBattleMgr().leftCharacterManager.objectBase.position.x +
						 positionOffset.x + this->j.x),
				static_cast<int>(-SokuLib::getBattleMgr().leftCharacterManager.objectBase.position.y +
						 positionOffset.y + this->j.y),
			});
		} else {
			this->sprite.setPosition(SokuLib::Vector2i{
				static_cast<int>(SokuLib::getBattleMgr().rightCharacterManager.objectBase.position.x +
						 positionOffset.x),
				static_cast<int>(-SokuLib::getBattleMgr().rightCharacterManager.objectBase.position.y +
						 positionOffset.y),
			});
		}
	}
	if (textureSize == SokuLib::Vector2i {0, 0}){
		this->sprite.setSize(this->sprite.texture.getSize());
	} else {
		this->sprite.setSize(textureSize);
	}

	sprite.rect.top = sprite.rect.width = 0;
	sprite.rect.width = sprite.texture.getSize().x;
	sprite.rect.height = sprite.texture.getSize().y;
	this->sprite.setRotation(rotation + this->i);
	this->sprite.draw();

	return sprite;
}

void MovingSprite::update(SokuLib::Vector2f velocity, float angularVelocity)
{
	this->i += angularVelocity;
	this->j += velocity;
	return;
}

//Function to load multiple texture at once

bool loadBattleInitSpriteOnce(const char *path, std::vector<std::unique_ptr<MovingSprite>> &spriteVec)
{
	bool result;

	spriteVec.emplace_back(new MovingSprite{path, &result});
	return result;
}

bool loadBattleInitTextOnce(const char *text, std::vector<std::unique_ptr<MovingSprite>> &spriteVec)
{
	bool result;

	spriteVec.emplace_back(new MovingSprite{text,defaultFont16, &result});
	return result;
}

//SokuDialog function (responsible of making dialog box including the text within)

SokuDialog::SokuDialog(std::vector<std::string> dialogLoad) {
	initBattleAnimation(this->_spriteVec, loadBattleInitSpriteOnce,
				    "data/scenario/effect/windowRight.bmp",
				    "data/scenario/effect/windowLoop.bmp",
				    "data/scenario/effect/windowLeft.bmp",
				    "data/scenario/effect/windowCenter.bmp");
	initBattleAnimation(this->_spriteVec, loadBattleInitTextOnce, std::move(dialogLoad));
}

//SokuThinking function (load the thinking type box)

SokuThinking::SokuThinking(std::vector<std::string> dialogLoad) :
	SokuDialog(std::move(dialogLoad))
{

	initBattleAnimation(this->_spriteVec, loadBattleInitSpriteOnce,
			    "data/scenario/effect/windowCenterB.bmp");
}


//SokuTalking function (load the talking type box)

SokuTalking::SokuTalking(std::vector<std::string> dialogLoad) :
	SokuDialog(std::move(dialogLoad))
{

	initBattleAnimation(this->_spriteVec, loadBattleInitSpriteOnce,
			    "data/scenario/effect/windowCenter.bmp");
}

//SokuStand function (responsible of making result box including text and stand within)

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

const char *getChrCodeName(SokuLib::Character chr)
{
	try {
		return validCharacters.at(chr).c_str();
	} catch (...) {}
	if (SokuLib::SWRUnlinked)
		return nullptr;
	try {
		return swrCharacters.at(chr).c_str();
	} catch (...) {}
	return nullptr;
}



SokuStand::SokuStand(std::vector<std::string> dialogLoad)
{
	std::string leftChar = getChrCodeName(SokuLib::leftChar);
	std::string rightChar = getChrCodeName(SokuLib::rightChar);
	this->_standDialogBox.setSize({624, 80});
	this->_standDialogBox.setPosition({8, 392});

	initBattleAnimation(
		this->_spriteVec, loadBattleInitSpriteOnce,
		("data/character/" + leftChar + "/stand/è-.bmp").c_str(),
		("data/character/" + leftChar + "/stand/è¥.bmp").c_str(),
		("data/character/" + leftChar + "/stand/ï-.bmp").c_str(),
		("data/character/" + leftChar + "/stand/îê.bmp").c_str(),
		("data/character/" + leftChar + "/stand/ô{.bmp").c_str(),
		("data/character/" + leftChar + "/stand/òë.bmp").c_str(),
		("data/character/" + leftChar + "/stand/òë.bmp").c_str(),
		("data/character/" + leftChar + "/stand/ù].bmp").c_str(),
		("data/character/" + leftChar + "/stand/ÿf.bmp").c_str()
	);
	initBattleAnimation(
		this->_spriteVec, loadBattleInitSpriteOnce,
		("data/character/" + rightChar + "/stand/è-.bmp").c_str(),
		("data/character/" + rightChar + "/stand/è¥.bmp").c_str(),
		("data/character/" + rightChar + "/stand/ï-.bmp").c_str(),
		("data/character/" + rightChar + "/stand/îê.bmp").c_str(),
		("data/character/" + rightChar + "/stand/ô{.bmp").c_str(),
		("data/character/" + rightChar + "/stand/òë.bmp").c_str(),
		("data/character/" + rightChar + "/stand/òë.bmp").c_str(),
		("data/character/" + rightChar + "/stand/ù].bmp").c_str(),
		("data/character/" + rightChar + "/stand/ÿf.bmp").c_str()
	);
	for (int i = 0; i < dialogLoad.size(); i++) {
		auto dialog = dialogLoad[i];
		if (i == 0) {
			this->_isLeftTalking = (dialog[0] == 'L');
			this->_left = sideStand(dialog[1]);
			this->_right = sideStand(dialog[2]);
		} else
			this->_metaData.emplace_back(dialog[0] == 'L', sideStand(dialog[1]));
		loadBattleInitTextOnce(dialog.c_str() + 2 + (i == 0), this->_spriteVec);
	}
}

void SokuStand::render(const SokuLib::CharacterManager *)
{

}

