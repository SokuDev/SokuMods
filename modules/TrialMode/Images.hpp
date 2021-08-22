//
// Created by PinkySmile on 18/08/2021.
//

#ifndef SWRSTOYS_IMAGES_HPP
#define SWRSTOYS_IMAGES_HPP

#include <SokuLib.hpp>

class Image {
public:
	virtual ~Image() = default;
	virtual void reset() = 0;
	virtual void update() = 0;
	virtual void render() const = 0;
};

class SimpleImage : public Image, public SokuLib::DrawUtils::Sprite {
public:
	SimpleImage(const std::string &path, const SokuLib::Vector2i &pos, bool CRTEffect);
	void reset() override;
	void update() override;
	void render() const override;
};

class AnimatedImage : public Image {
private:
	struct Frame {
		uint32_t delay = 0;
		SokuLib::DrawUtils::DxSokuColor *buffer = nullptr;

		~Frame() { delete[] buffer; }
	};

	bool _antiAlias;
	bool _CRTEffect;
	IDirect3DTexture9 **_pphandle;
	SokuLib::DrawUtils::Sprite _sprite;
	SokuLib::Vector2i _pos;
	SokuLib::DrawUtils::DxSokuColor *_frame = nullptr;
	SokuLib::DrawUtils::DxSokuColor *_lastFrame = nullptr;
	std::vector<std::unique_ptr<Frame>> _frames;
	unsigned int _currentFrame = 0;
	float _internalCtr = 0;
	int _last = 0;
	SokuLib::Vector2u _size{0, 0};

	void _updateTexture();
	void _copyDecodedFrame(struct GIF_WHDR *chunk, SokuLib::DrawUtils::DxSokuColor *pict, Frame *frameObj);
	void _processFrame(Frame *frameObj);

public:
	AnimatedImage(const std::string &path, const SokuLib::Vector2i &pos, bool antiAliasing, bool CRTEffect);
	~AnimatedImage() override = default;
	void reset() override;
	void update() override;
	void processChunk(struct GIF_WHDR *chunk);
	void render() const override;
};

#endif //SWRSTOYS_IMAGES_HPP
