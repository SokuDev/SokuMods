//
// Created by 2deg on 20/07/2021.
//

#include <SokuLib.hpp>
#include <vector>

#ifndef SWRSTOYS_BATTLEANIMATION_HPP
#define SWRSTOYS_BATTLEANIMATION_HPP


enum CharacterSide {
	LEFTCHAR,
	RIGHTCHAR,
	NONE
};

class MovingSprite{
private:
	float i = 0;
	SokuLib::Vector2f j = {0, 0};
	SokuLib::DrawUtils::Sprite sprite{};

public:
	MovingSprite(const char *path, bool *success = nullptr);
	MovingSprite(const char *text, SokuLib::SWRFont &font);
	~MovingSprite() = default;

	const SokuLib::DrawUtils::Sprite &render(
		CharacterSide characterFollowed = NONE,
		SokuLib::Vector2u spriteSize = {0, 0},
		SokuLib::Vector2f positionOffset = {0, 0},
		const SokuLib::Camera *camera = nullptr,
		SokuLib::Vector2i textureSize = {0, 0},
		SokuLib::Vector2b mirror = {false, false},
		float rotation = 0
	) const;

	void update(SokuLib::Vector2f velocity = {0, 0}, SokuLib::Vector2f angularVelocity = {0, 0});
};

bool loadBattleInitSpriteOnce(const char *path, std::vector<std::unique_ptr<MovingSprite>> &spriteVec);

inline bool initBattleAnimation(std::vector<std::unique_ptr<MovingSprite>> &)
{
	return true;
}

template<typename ...Args>
inline bool initBattleAnimation(std::vector<std::unique_ptr<MovingSprite>> &spriteVec, const char *path, Args... args)
{
	return loadBattleInitSpriteOnce(path, spriteVec) && initBattleAnimation(spriteVec, args...);
}

SokuLib::DrawUtils::Sprite &renderMovingSprite(MovingSprite &sprite,
							CharacterSide characterFollowed = NONE,
							SokuLib::Vector2u spriteSize = {0, 0},
							SokuLib::Vector2f positionOffset = {0, 0},
							const SokuLib::Camera *camera = nullptr,
							SokuLib::Vector2i textureSize = {0, 0},
							SokuLib::Vector2b mirror = {false, false},
				 			SokuLib::Vector2f movement = {0, 0},
							float rotation = 0);

class Dialog {
private:
	std::vector<std::unique_ptr<MovingSprite>> _spriteVec;

public:
	enum DBoxType {
		THINKINGBOX,
		TALKINGBOX
	};

	Dialog(const char *dialogLoad, DBoxType = TALKINGBOX);
	~Dialog();
	void Dialog::renderDialogWithBox(const SokuLib::CharacterManager &charSide);
};


#endif //SWRSTOYS_BATTLEANIMATION_HPP
