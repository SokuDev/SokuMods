//
// Created by PinkySmile on 18/08/2021.
//

#ifndef SWRSTOYS_IMAGES_HPP
#define SWRSTOYS_IMAGES_HPP

#include <SokuLib.hpp>
#include "gifdec.h"

class Image {
public:
	virtual ~Image() = default;
	virtual void reset() = 0;
	virtual void update() = 0;
	virtual void render() const = 0;
};

class SimpleImage : public Image, public SokuLib::DrawUtils::Sprite {
public:
	SimpleImage(const std::string &path, const SokuLib::Vector2i &pos);
	void reset() override;
	void update() override;
	void render() const override;
};

class AnimatedImage : public Image {
private:
	struct Frame {
		uint32_t delay;
		SokuLib::DrawUtils::Sprite sprite;
	};
	std::vector<std::unique_ptr<Frame>> _frames;
	unsigned int _currentFrame = 0;
	float _internalCtr = 0;

public:
	AnimatedImage(const std::string &path, const SokuLib::Vector2i &pos);
	void reset() override;
	void update() override;
	void render() const override;
};

#endif //SWRSTOYS_IMAGES_HPP
