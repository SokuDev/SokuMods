//
// Created by PinkySmile on 18/08/2021.
//

#undef _D3D9_H_
#include <d3d9.h>
#include "Images.hpp"

#ifndef _DEBUG
#define puts(...)
#define printf(...)
#define fprintf(...)
#endif

SimpleImage::SimpleImage(const std::string &path, const SokuLib::Vector2i &pos)
{
	this->texture.loadFromFile(path.c_str());
	this->rect = {
		0, 0,
		static_cast<int>(this->texture.getSize().x),
		static_cast<int>(this->texture.getSize().y),
	};
	this->setPosition(pos);
	this->setSize({200, 150});
}

void SimpleImage::update()
{}

void SimpleImage::render() const
{
	if (this->texture.hasTexture())
		this->draw();
}

void SimpleImage::reset()
{}

AnimatedImage::AnimatedImage(const std::string &path, const SokuLib::Vector2i &pos)
{
	int id;
	HRESULT ret;
	D3DLOCKED_RECT r;

	printf("Loading GIF file %s\n", path.c_str());
	auto gif = gd_open_gif(path.c_str());
	if (!gif) {
		fprintf(stderr, "Could not open %s\n", path.c_str());
		return;
	}

	printf("GIF %dx%d %d colors", gif->width, gif->height, gif->palette->size);
	auto frame = new(std::nothrow) unsigned char[gif->width * gif->height * 3];
	if (!frame) {
		fprintf(stderr, "Could not allocate frame (%i bytes)\n", gif->width * gif->height * 3);
		return;
	}

	while ((ret = gd_get_frame(gif)) > 0) {

		auto pphandle = SokuLib::textureMgr.allocate(&id);

		if (FAILED(ret = D3DXCreateTexture(SokuLib::pd3dDev, gif->width, gif->height, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, pphandle))) {
			SokuLib::textureMgr.deallocate(id);
			fprintf(stderr, "D3DXCreateTexture(SokuLib::pd3dDev, %i, %i, D3DX_DEFAULT, D3DUSAGE_RENDERTARGET, D3DFMT_A8B8G8R8, D3DPOOL_DEFAULT, %p) failed with code %i\n", gif->width, gif->height, pphandle, ret);
			continue;
		}

		gd_render_frame(gif, frame);
		if (FAILED(ret = (*pphandle)->LockRect(0, &r, nullptr, 0))) {
			fprintf(stderr, "(*this->_pphandle)->LockRect(0, &r, nullptr, D3DLOCK_DISCARD) failed with code %li\n", ret);
			SokuLib::textureMgr.deallocate(id);
			continue;
		}
		for (int y = 0; y < gif->height; y++) {
			auto pData = (SokuLib::DrawUtils::DxSokuColor *)((BYTE *)r.pBits + r.Pitch * y);

			for (int x = 0; x < gif->width; x++) {
				pData[x].a = 0xFF;
				pData[x].r = frame[(y * gif->width + x) * 3];
				pData[x].g = frame[(y * gif->width + x) * 3 + 1];
				pData[x].b = frame[(y * gif->width + x) * 3 + 2];
			}
		}
		if (FAILED(ret = (*pphandle)->UnlockRect(0)))
			fprintf(stderr, "(*this->_pphandle)->UnlockRect(0) failed with code %li\n", ret);

		auto frameObj = new Frame();

		frameObj->delay = gif->gce.delay * 10;
		frameObj->sprite.texture.setHandle(id, {gif->width, gif->height});
		frameObj->sprite.rect = {
			0, 0,
			static_cast<int>(frameObj->sprite.texture.getSize().x),
			static_cast<int>(frameObj->sprite.texture.getSize().y),
		};
		frameObj->sprite.setPosition(pos);
		frameObj->sprite.setSize({200, 150});
		this->_frames.emplace_back(frameObj);
	}
	if (ret == -1)
		fprintf(stderr, "gd_get_frame returned -1\n");
}

void AnimatedImage::update()
{
	this->_internalCtr += 1000 / 60.f;
	while (this->_internalCtr >= this->_frames[this->_currentFrame]->delay) {
		this->_internalCtr -= this->_frames[this->_currentFrame]->delay;
		this->_currentFrame++;
		this->_currentFrame %= this->_frames.size();
	}
}

void AnimatedImage::render() const
{
	this->_frames[this->_currentFrame]->sprite.draw();
}

void AnimatedImage::reset()
{
	this->_currentFrame = 0;
}
