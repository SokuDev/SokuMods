//
// Created by PinkySmile on 18/08/2021.
//

#undef _D3D9_H_
#include <d3d9.h>
#include <fstream>
#include "gif_load.h"
#include "Images.hpp"

#ifndef _DEBUG
#define puts(...)
#define printf(...)
#define fprintf(...)
#endif

SimpleImage::SimpleImage(const std::string &path, const SokuLib::Vector2i &pos, bool CRTEffect)
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

static void callback(void *data, struct GIF_WHDR *chunk)
{
	auto This = (AnimatedImage *)data;

	This->processChunk(chunk);
}

AnimatedImage::AnimatedImage(const std::string &path, const SokuLib::Vector2i &pos, bool antiAliasing, bool CRTEffect)
{
	std::ifstream stream{path, std::ifstream::binary};
	int id;
	HRESULT ret;

	WIN32_FILE_ATTRIBUTE_DATA fad;

	if (!GetFileAttributesEx(path.c_str(), GetFileExInfoStandard, &fad)) {
		fprintf(stderr, "GetFileAttributesEx(%s) failed with code %lX\n", path.c_str(), GetLastError());
		return;
	}

	if (fad.nFileSizeHigh) {
		fprintf(stderr, "%s is too big\n", path.c_str());
		return;
	}

	this->_pos = pos;
	if (!stream) {
		fprintf(stderr, "Could not open %s\n", path.c_str());
		return;
	}

	auto buf = new unsigned char[fad.nFileSizeLow];

	printf("Reading file %s\n", path.c_str());
	stream.read(reinterpret_cast<char *>(buf), fad.nFileSizeLow);

	this->_pphandle = SokuLib::textureMgr.allocate(&id);

	printf("Loading GIF file %s\n", path.c_str());
	if (GIF_Load((void *)buf, fad.nFileSizeLow, callback, nullptr, this, 0) < 0) {
		fprintf(stderr, "Could not load GIF %s\n", path.c_str());
		delete[] buf;
		return;
	}

	printf("GIF %dx%d\n", this->_size.x, this->_size.y);
	this->_sprite.texture.setHandle(id, this->_size);
	this->_sprite.rect = {
		0, 0,
		static_cast<int>(this->_sprite.texture.getSize().x),
		static_cast<int>(this->_sprite.texture.getSize().y),
	};
	this->_sprite.setPosition(this->_pos);
	this->_sprite.setSize({200, 150});

	delete[] buf;
	delete[] this->_frame;
	delete[] this->_lastFrame;

	if (FAILED(ret = D3DXCreateTexture(SokuLib::pd3dDev, 200, 150, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, this->_pphandle))) {
		SokuLib::textureMgr.deallocate(id);
		fprintf(stderr, "D3DXCreateTexture(SokuLib::pd3dDev, 200, 150, D3DX_DEFAULT, D3DUSAGE_RENDERTARGET, D3DFMT_A8B8G8R8, D3DPOOL_DEFAULT, %p) failed with code %i\n", this->_pphandle, ret);
		return;
	}
	this->_updateTexture();
}

void AnimatedImage::update()
{
	this->_internalCtr += 1000 / 60.f;
	if (this->_frames.empty())
		return;
	while (this->_internalCtr >= this->_frames[this->_currentFrame]->delay) {
		this->_internalCtr -= this->_frames[this->_currentFrame]->delay;
		this->_currentFrame++;
		this->_currentFrame %= this->_frames.size();
		this->_updateTexture();
	}
}

void AnimatedImage::render() const
{
	this->_sprite.draw();
}

void AnimatedImage::reset()
{
	this->_currentFrame = 0;
	this->_updateTexture();
}

void AnimatedImage::processChunk(struct GIF_WHDR *chunk)
{
	this->_size.x = chunk->xdim;
	this->_size.y = chunk->ydim;

	uint32_t x, y, yoff, iter, ifin, dsrc, ddst;
	SokuLib::DrawUtils::DxSokuColor *pict, *prev;

	if (!chunk->ifrm) {
		ddst = (uint32_t)(chunk->xdim * chunk->ydim);
		this->_frame = new(std::nothrow) SokuLib::DrawUtils::DxSokuColor[ddst];
		this->_lastFrame = new(std::nothrow) SokuLib::DrawUtils::DxSokuColor[ddst];
		if (!this->_frame || !this->_lastFrame)
			fprintf(stderr, "Memory allocation error\n");
	}
	if (!this->_frame || !this->_lastFrame)
		return;
	/** [TODO:] the frame is assumed to be inside global bounds,
		    however it might exceed them in some GIFs; fix me. **/

	auto frameObj = new Frame();

	frameObj->delay = chunk->time < 0 ? 0 : chunk->time * 10;
	frameObj->buffer = new(std::nothrow) SokuLib::DrawUtils::DxSokuColor[200 * 150];
	if (!frameObj->buffer) {
		delete frameObj;
		fprintf(stderr, "Memory allocation error\n");
		return;
	}
	pict = this->_frame;
	ddst = (uint32_t)(chunk->xdim * chunk->fryo + chunk->frxo);
	ifin = (!(iter = (chunk->intr)? 0 : 4))? 4 : 5; /** interlacing support **/
	for (dsrc = (uint32_t)-1; iter < ifin; iter++)
		for (yoff = 16U >> ((iter > 1)? iter : 1), y = (8 >> iter) & 7; y < (uint32_t)chunk->fryd; y += yoff)
			for (x = 0; x < (uint32_t)chunk->frxd; x++)
				if (chunk->tran != (long)chunk->bptr[++dsrc]) {
					auto &color = chunk->cpal[chunk->bptr[dsrc]];

					pict[chunk->xdim * y + x + ddst].r = color.R;
					pict[chunk->xdim * y + x + ddst].g = color.G;
					pict[chunk->xdim * y + x + ddst].b = color.B;
					pict[chunk->xdim * y + x + ddst].a = 0xFF;
				}
	this->_copyDecodedFrame(chunk, pict, frameObj);
	this->_processFrame(frameObj);
	if ((chunk->mode == GIF_PREV) && !this->_last) {
		chunk->frxd = chunk->xdim;
		chunk->fryd = chunk->ydim;
		chunk->mode = GIF_BKGD;
		ddst = 0;
	} else {
		this->_last = (chunk->mode == GIF_PREV) ? this->_last : (chunk->ifrm + 1);
		pict = ((chunk->mode == GIF_PREV) ? this->_frame : this->_lastFrame);
		prev = ((chunk->mode == GIF_PREV) ? this->_lastFrame : this->_frame);
		memcpy(pict, prev, chunk->xdim * chunk->ydim * sizeof(*pict));
	}
	if (chunk->mode == GIF_BKGD) /** cutting a hole for the next frame **/
		for (chunk->bptr[0] = (uint8_t)((chunk->tran >= 0)? chunk->tran : chunk->bkgd), y = 0, pict = this->_frame; y < (uint32_t)chunk->fryd; y++)
			for (x = 0; x < (uint32_t)chunk->frxd; x++) {
				auto &color = chunk->cpal[chunk->bptr[0]];

				pict[chunk->xdim * y + x + ddst].r = color.R;
				pict[chunk->xdim * y + x + ddst].g = color.G;
				pict[chunk->xdim * y + x + ddst].b = color.B;
				pict[chunk->xdim * y + x + ddst].a = 0xFF;
			}
	this->_frames.emplace_back(frameObj);
}

void AnimatedImage::_updateTexture()
{
	HRESULT ret;
	D3DLOCKED_RECT r;

	if (this->_frames.empty())
		return;
	if (FAILED(ret = (*this->_pphandle)->LockRect(0, &r, nullptr, 0))) {
		fprintf(stderr, "(*this->_pphandle)->LockRect(0, &r, nullptr, D3DLOCK_DISCARD) failed with code %li\n", ret);
		return;
	}

	memcpy(r.pBits, this->_frames[this->_currentFrame]->buffer, sizeof(*this->_frames[this->_currentFrame]->buffer) * 200 * 150);
	if (FAILED(ret = (*this->_pphandle)->UnlockRect(0)))
		fprintf(stderr, "(*this->_pphandle)->UnlockRect(0) failed with code %li\n", ret);
}

void AnimatedImage::_copyDecodedFrame(struct GIF_WHDR *chunk, SokuLib::DrawUtils::DxSokuColor *pict, Frame *frameObj)
{
	if (!this->_antiAlias) {
		for (auto x = 0; x < 200; x++)
			for (auto y = 0; y < 150; y++)
				frameObj->buffer[y * 200 + x] = pict[chunk->xdim * (y * chunk->ydim / 150) + (x * chunk->xdim / 200)];
		return;
	}

	for (auto x = 0; x < 200; x++)
		for (auto y = 0; y < 150; y++)
			frameObj->buffer[y * 200 + x] = pict[chunk->xdim * (y * chunk->ydim / 150) + (x * chunk->xdim / 200)];
}

void AnimatedImage::_processFrame(AnimatedImage::Frame *frameObj)
{
	if (!this->_CRTEffect)
		return;
	for (auto y = 0; y < 150; y += 2)
		for (auto x = 0; x < 200; x++) {
			auto &color = frameObj->buffer[y * 200 + x];

			color.r = color.r * 5 / 6;
			color.g = color.g * 5 / 6;
			color.b = color.b * 5 / 6;
		}
}
