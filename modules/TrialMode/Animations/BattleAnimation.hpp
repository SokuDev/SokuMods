//
// Created by 2deg and PinkySmile on 20/07/2021.
//

#include <SokuLib.hpp>
#include <vector>
#include <list>
#include <memory>
#include <stdexcept>

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

BattleAnimation *loadBattleAnimation(const char *packPath, const char *path, bool isIntro);

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
protected:
	bool _draw = false;

public:
	virtual ~Dialog() = default;
	virtual void update() = 0;
	virtual bool onKeyPress() = 0;
	virtual void render() const = 0;
	virtual void setHidden(bool hidden) = 0;
	bool isHidden() const { return !this->_draw; };
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

	SokuLib::SWRFont _lfont;
	SokuLib::SWRFont _rfont;
	CharStand _left = NOSTAND;
	CharStand _right = NOSTAND;
	SokuLib::Vector2f _leftPos;
	SokuLib::Vector2f _rightPos;
	float _leftStep = 1;
	float _rightStep = 1;
	SokuLib::Vector2f _leftExpectedPos;
	SokuLib::Vector2f _rightExpectedPos;
	unsigned char _leftAlpha = 0xFF;
	unsigned char _rightAlpha = 0xFF;
	unsigned char _leftAlphaStep;
	unsigned char _rightAlphaStep;
	bool _isLeftTalking;
	SokuLib::DrawUtils::Sprite _text;
	std::list<std::tuple<std::string, bool, CharStand, CharStand>> _metaData;
	mutable SokuLib::DrawUtils::RectangleShape _standDialogBox;
	mutable std::vector<std::unique_ptr<SokuLib::DrawUtils::Sprite>> _stands;

	static SokuStand::CharStand sideStand(char c);

public:
	SokuStand(std::vector<std::string> dialogLoad);
	~SokuStand();
	void render() const override;
	void update() override;
	bool onKeyPress() override;
	void setHidden(bool hidden) override;
	unsigned int getCurrentDialog() const override;
	bool isAnimationFinished() const;
	void finishAnimations();
};

#endif //SWRSTOYS_BATTLEANIMATION_HPP
