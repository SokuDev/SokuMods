//
// Created by benoi on 20/07/2021.
//

#include "BattleAnimation.hpp"

static bool stopRepeat = false;
static SokuLib::DrawUtils::Sprite sprite{};
static int i = 0;

//Beware to modify Text size too much (because antialiasing)
/*bool loadBattleAnimationTextOnTextBox(char *dialogue, SokuLib::Vector2<unsigned int> textBoxSize)
{

}
*/
bool loadBattleAnimationSprite(const char *path,
			       const SokuLib::Camera *camera)
{
	sprite.setCamera(camera);

	if (!stopRepeat) {
		sprite.texture.loadFromGame(path);
		stopRepeat = true;
	}

	return true;
}


bool renderBattleAnimationSprite(bool characterFollowed,
				 SokuLib::Vector2u textureSize,
				 SokuLib::Vector2f positionOffset,
				 int rotation)
{
	if (characterFollowed == 0) {
		sprite.setPosition(SokuLib::Vector2<int>{
			static_cast<int>(SokuLib::getBattleMgr().rightCharacterManager.objectBase.position.x-64+positionOffset.x),
			static_cast<int>(-SokuLib::getBattleMgr().rightCharacterManager.objectBase.position.y-128+positionOffset.y),
		});
	} else{
		sprite.setPosition(SokuLib::Vector2<int>{
			static_cast<int>(SokuLib::getBattleMgr().leftCharacterManager.objectBase.position.x-64+positionOffset.x),
			static_cast<int>(-SokuLib::getBattleMgr().leftCharacterManager.objectBase.position.y-128+positionOffset.y),
		});
	}
	sprite.setSize({textureSize.x, textureSize.y});
	sprite.rect.top = sprite.rect.width = 0;
	sprite.rect.width = sprite.texture.getSize().x;
	sprite.rect.height = sprite.texture.getSize().y;
	sprite.setRotation(i);
	sprite.draw();
	i = i + rotation;

	return true;
}