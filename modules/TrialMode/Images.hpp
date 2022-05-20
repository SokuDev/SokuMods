//
// Created by PinkySmile on 18/08/2021.
//

#ifndef SWRSTOYS_IMAGES_HPP
#define SWRSTOYS_IMAGES_HPP

#include <SokuLib.hpp>
#include <mutex>

class Image {
public:
	virtual ~Image() = default;
	virtual void reset() = 0;
	virtual void update() = 0;
	virtual void render() const = 0;
	virtual bool isValid() const = 0;
	virtual void setPosition(SokuLib::Vector2i pos) = 0;
};

class SimpleImage : public Image, public SokuLib::DrawUtils::Sprite {
public:
	SimpleImage(const std::string &path, const SokuLib::Vector2i &pos);
	void reset() override;
	void update() override;
	void render() const override;
	bool isValid() const override;
	void setPosition(SokuLib::Vector2i pos) override;
};

class AnimatedImage : public Image {
private:
	struct Frame {
		uint32_t delay = 0;
		SokuLib::DrawUtils::DxSokuColor *buffer = nullptr;

		~Frame() { delete[] buffer; }
	};

	bool _antiAlias;
	IDirect3DTexture9 **_pphandle;
	SokuLib::DrawUtils::Sprite _sprite;
	std::vector<std::unique_ptr<Frame>> _frames;
	unsigned int _currentFrame = 0;
	float _internalCtr = 0;
	int _last = 0;
	std::mutex mutex;
	std::mutex destroymutex;

	void _updateTexture();
	void _copyDecodedFrame(struct GIF_WHDR *chunk, SokuLib::DrawUtils::DxSokuColor *pict, Frame *frameObj);
	void _processFrame(Frame *frameObj);
	void _init();

public:
	AnimatedImage(const std::string &path, const SokuLib::Vector2i &pos, bool antiAliasing);
	~AnimatedImage() override;
	void reset() override;
	void update() override;
	void processChunk(struct GIF_WHDR *chunk);
	void render() const override;
	bool isValid() const override;
	void setPosition(SokuLib::Vector2i pos) override;

	size_t size;
	std::string path;
	bool _loading = false;
	unsigned char *buf;
	SokuLib::Vector2u _size{0, 0};
	WIN32_FILE_ATTRIBUTE_DATA fad;
	SokuLib::DrawUtils::DxSokuColor *_frame = nullptr;
	SokuLib::DrawUtils::DxSokuColor *_lastFrame = nullptr;
	HANDLE thread = INVALID_HANDLE_VALUE;
	bool _needExit = false;
	std::mutex dmutex;
};

#endif //SWRSTOYS_IMAGES_HPP
