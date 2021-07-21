//
// Created by 2deg on 20/07/2021.
//

#include <SokuLib.hpp>

#ifndef SWRSTOYS_BATTLEANIMATION_HPP
#define SWRSTOYS_BATTLEANIMATION_HPP

//bool 	LoadBattleAnimationTextOnTextBox(const char *dialogue, SokuLib::Vector2u textBoxSize, const SokuLib::Camera *camera = nullptr);

bool	loadBattleAnimationSprite(const char *path, const SokuLib::Camera *camera = nullptr);

//bool	LoadBattleAnimation(const char *dialogue, const char *path, SokuLib::Vector2u textBoxSize, const SokuLib::Camera *camera = nullptr);



bool renderBattleAnimationSprite(bool characterFollowed,
				 SokuLib::Vector2u textureSize,
				 SokuLib::Vector2f positionOffset = {0, 0},
				 int rotation = 0);


#endif //SWRSTOYS_BATTLEANIMATION_HPP
