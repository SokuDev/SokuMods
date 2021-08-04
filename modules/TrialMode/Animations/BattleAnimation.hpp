//
// Created by 2deg and PinkySmile on 20/07/2021.
//

#include <SokuLib.hpp>
#include <vector>
#include <list>

#ifndef SWRSTOYS_BATTLEANIMATION_HPP
#define SWRSTOYS_BATTLEANIMATION_HPP


class BattleAnimation {
public:
	virtual ~BattleAnimation() = default;
	virtual bool update() = 0;
	virtual void render() const = 0;
	virtual void onKeyPressed() = 0;
};

class AnimationLoop {
public:
	void update();
};

BattleAnimation *loadBattleAnimation(const char *path, bool isIntro);

class MovingSprite : public SokuLib::DrawUtils::Sprite {
private:
	SokuLib::Vector2i _speed;
	float _angularSpeed;

public:
	MovingSprite(
		void (*loader)(MovingSprite &),
		SokuLib::Vector2i pos,
		SokuLib::Camera *camera = nullptr,
		SokuLib::Vector2u size = {0, 0},
		SokuLib::DrawUtils::TextureRect rect = {-1, -1, -1, -1},
		float rotation = 0,
		SokuLib::Vector2i speed = {0, 0},
		float angularSpeed = 0
	);
	void update();
};

class Dialog {
public:
	virtual ~Dialog() = default;
	virtual void update() = 0;
	virtual bool onKeyPress() = 0;
	virtual void render() const = 0;
	virtual unsigned getCurrentDialog() const = 0;
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

	SokuLib::SWRFont _font16;
	CharStand _left;
	CharStand _right;
	bool _isLeftTalking;
	bool _draw = false;
	SokuLib::DrawUtils::Sprite _text;
	std::list<std::tuple<std::string, bool, CharStand>> _metaData;
	SokuLib::DrawUtils::RectangleShape _standDialogBox;
	std::vector<std::unique_ptr<SokuLib::DrawUtils::Sprite>> _stands;

	static SokuStand::CharStand sideStand(char c);

public:
	SokuStand(std::vector<std::string> dialogLoad);
	~SokuStand();
	void render() const override;
	void update() override;
	bool onKeyPress() override;

	unsigned int getCurrentDialog() const override;
};

#endif //SWRSTOYS_BATTLEANIMATION_HPP