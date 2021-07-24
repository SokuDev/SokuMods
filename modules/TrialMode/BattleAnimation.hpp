//
// Created by 2deg and PinkySmile on 20/07/2021.
//

#include <SokuLib.hpp>
#include <vector>
#include <list>

#ifndef SWRSTOYS_BATTLEANIMATION_HPP
#define SWRSTOYS_BATTLEANIMATION_HPP


enum CharacterSide {
	LEFTCHAR,
	RIGHTCHAR,
	NONE
};

static int k = 0;

class MovingSprite{
private:
	float i = 0;
	SokuLib::Vector2f j = {0, 0};
	mutable SokuLib::DrawUtils::Sprite sprite{};

public:
	MovingSprite(const char *path, bool *success = nullptr);
	MovingSprite(const char *text, SokuLib::SWRFont &font, bool *success);
	~MovingSprite() = default;

	const SokuLib::DrawUtils::Sprite &render(CharacterSide characterFollowed = NONE,
		SokuLib::Vector2u spriteSize = {0, 0},
		SokuLib::Vector2f positionOffset = {0, 0},
		const SokuLib::Camera *camera = nullptr,
		SokuLib::Vector2u textureSize = {0, 0},
		SokuLib::Vector2b mirror = {false, false},
		float rotation = 0
	) const;

	void update(SokuLib::Vector2f velocity = {0, 0}, float angularVelocity = 0);
};

bool loadBattleInitSpriteOnce(const char *path, std::vector<std::unique_ptr<MovingSprite>> &spriteVec);
bool loadBattleInitTextOnce(const char *path, std::vector<std::unique_ptr<MovingSprite>> &spriteVec);

inline bool initBattleAnimation(
	std::vector<std::unique_ptr<MovingSprite>> &,
	bool (*fct)(const char *, std::vector<std::unique_ptr<MovingSprite>> &)
)
{
	return true;
}

template<typename ...Args>
inline bool initBattleAnimation(
	std::vector<std::unique_ptr<MovingSprite>> &spriteVec,
	bool (*fct)(const char *, std::vector<std::unique_ptr<MovingSprite>> &),
	const char *path,
	Args... args
)
{
	return fct(path, spriteVec) && initBattleAnimation(spriteVec, fct, args...);
}

inline bool initBattleAnimation(
	std::vector<std::unique_ptr<MovingSprite>> &spriteVec,
	bool (*fct)(const char *, std::vector<std::unique_ptr<MovingSprite>> &),
	std::vector<std::string> paths
)
{
	bool result = true;

	for (auto &path : paths)
		result &= fct(path.c_str(), spriteVec);
	return result;
}

class Dialog {
protected:
	std::vector<std::unique_ptr<MovingSprite>> _spriteVec;

public:
	virtual ~Dialog() = default;
	virtual void update() = 0;
	virtual bool onKeyPress() = 0;
	virtual void render(const SokuLib::CharacterManager *charSide) = 0;
};

class SokuDialog : public Dialog {

public:
	SokuDialog(std::vector<std::string> dialogLoad);
	~SokuDialog() = default;
};

class SokuThinking : public SokuDialog {

public:
	SokuThinking(std::vector<std::string> dialogLoad);
	~SokuThinking() = default;
};

class SokuTalking : public SokuDialog {

public:
	SokuTalking(std::vector<std::string> dialogLoad);
	~SokuTalking() = default;
};

class SokuStand : public Dialog {
private:
	enum CharStand {
		NOSTAND = -1,
		HAPPY,
		EMBARRASSED,
		SURPRISED,
		CONFIDENT,
		ANGRY,
		DEFEATED,
		WINNING,
		HAPPY2,
		CONCERNED,
		NB_STANDS
	};
	enum SpriteIndex {
		SPRITE_LEFT_STAND_START,
		SPRITE_RIGHT_STAND_START = NB_STANDS,
	};

	bool _isLeftTalking;
	CharStand _left;
	CharStand _right;
	std::list<std::pair<bool, CharStand>> _metaData;
	SokuLib::DrawUtils::RectangleShape _standDialogBox;

	static SokuStand::CharStand sideStand(char c);

public:
	SokuStand(std::vector<std::string> dialogLoad);
	~SokuStand() = default;
	void render(const SokuLib::CharacterManager *) override;
};

#endif //SWRSTOYS_BATTLEANIMATION_HPP
