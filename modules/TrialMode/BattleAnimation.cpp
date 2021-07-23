//
// Created by 2deg on 20/07/2021.
//

#include <memory>
#include "BattleAnimation.hpp"
#include "Menu.hpp"

//Beware to modify Text size too much (because antialiasing)
/*bool loadBattleAnimationTextOnTextBox(char *dialogue, SokuLib::Vector2<unsigned int> textBoxSize)
{

}
*/

MovingSprite::MovingSprite(const char *path, bool *success)
{
	bool result = this->sprite.texture.loadFromGame(path);

	if (success)
		*success = result;
}

MovingSprite::MovingSprite(const char *text, SokuLib::SWRFont &font)
{
	SokuLib::Vector2i buffer;

	this->sprite.texture.createFromText(text, defaultFont16, {1300, 60}, &buffer);
	this->sprite.setSize(buffer.to<unsigned>());
	this->sprite.rect.width = buffer.x;
	this->sprite.rect.height = buffer.y;
}

void update(SokuLib::Vector2f velocity = {0, 0}, SokuLib::Vector2f angularVelocity = {0, 0});

bool loadBattleInitSpriteOnce(const char *path, std::vector<std::unique_ptr<MovingSprite>> &spriteVec)
{
	bool result;

	spriteVec.emplace_back(new MovingSprite{path, &result});
	return result;
}

SokuLib::DrawUtils::Sprite &renderBattleAnimationSprite(MovingSprite &sprite,
							CharacterSide characterFollowed,
							SokuLib::Vector2u textureSize,
							SokuLib::Vector2f positionOffset,
							const SokuLib::Camera *camera,
							SokuLib::Vector2b Mirror,
							SokuLib::Vector2f movement,
							float rotation)
{
	sprite.sprite.setCamera(camera);

	sprite.sprite.setMirroring(Mirror.x, Mirror.y);
	if (characterFollowed == NONE){
		sprite.sprite.setPosition(positionOffset.to<int>() + sprite.j.to<int>());
	} else {
		if (characterFollowed == LEFTCHAR) {
			sprite.sprite.setPosition(SokuLib::Vector2i{
				static_cast<int>(SokuLib::getBattleMgr().leftCharacterManager.objectBase.position.x +
				positionOffset.x + sprite.j.x),
				static_cast<int>(-SokuLib::getBattleMgr().leftCharacterManager.objectBase.position.y +
				positionOffset.y + sprite.j.y),
			});
		} else {
			sprite.sprite.setPosition(SokuLib::Vector2i{
				static_cast<int>(SokuLib::getBattleMgr().rightCharacterManager.objectBase.position.x +
						 positionOffset.x + sprite.j.x),
				static_cast<int>(-SokuLib::getBattleMgr().rightCharacterManager.objectBase.position.y +
						 positionOffset.y + sprite.j.y),
			});

		}
	}
	if (textureSize == SokuLib::Vector2i {0, 0}){
		sprite.sprite.setSize(sprite.sprite.texture.getSize());
	} else{
		sprite.sprite.setSize(textureSize);
	}

	sprite.sprite.rect.top = sprite.sprite.rect.width = 0;
	sprite.sprite.rect.width = sprite.sprite.texture.getSize().x;
	sprite.sprite.rect.height = sprite.sprite.texture.getSize().y;
	sprite.sprite.setRotation(sprite.i);
	sprite.sprite.draw();

	sprite.i = sprite.i + rotation;
	sprite.j = sprite.j + movement;

	return sprite.sprite;
}

Dialog::Dialog(const char *dialogLoad, DBoxType dBoxType)
{
	SokuLib::Vector2i buffer;

	if (dBoxType == TALKINGBOX) {
		initBattleAnimation(this->_spriteVec,
				    "data/scenario/effect/windowRight.bmp",
				    "data/scenario/effect/windowLoop.bmp",
				    "data/scenario/effect/windowLeft.bmp",
				    "data/scenario/effect/windowCenter.bmp");
	} else {
		initBattleAnimation(this->_spriteVec,
				    "data/scenario/effect/windowRight.bmp",
				    "data/scenario/effect/windowLoop.bmp",
				    "data/scenario/effect/windowLeft.bmp",
				    "data/scenario/effect/windowCenterB.bmp");
	}
	this->_spriteVec.emplace_back(new MovingSprite);
	this->_spriteVec.back()->sprite.texture.createFromText(dialogLoad, defaultFont16, {1300, 60}, &buffer);
	this->_spriteVec.back()->sprite.setSize(buffer.to<unsigned>());
	this->_spriteVec.back()->sprite.rect.width = buffer.x;
	this->_spriteVec.back()->sprite.rect.height = buffer.y;
}

void Dialog::renderDialogWithBox(const SokuLib::CharacterManager &charSide)
{
	renderBattleAnimationSprite(*this->_spriteVec[0], NONE, {0,0,}, {charSide.objectBase.position.x - 100, charSide.objectBase.position.y - 250}, &SokuLib::camera);

}