//
// Created by PinkySmile on 18/08/2021.
//

#undef _D3D9_H_
#include <d3d9.h>
#include <fstream>
#include "gif_load.h"
#include "Images.hpp"
#include "Socket.hpp"
#include "Exceptions.hpp"
#include "version.h"

#ifndef _DEBUG
#define puts(...)
#define printf(...)
#define fprintf(...)
#endif

void unchunkBody(Socket::HttpResponse &res)
{
	std::string body;

	body.reserve(res.body.size());
	while (true) {
		auto pos = res.body.find_first_of("\r\n");

		if (pos == std::string::npos)
			return;

		try {
			auto chunkSize = std::stoul(res.body.substr(0, pos), nullptr, 16);

			if (!chunkSize)
				break;
			res.body = res.body.substr(pos + 2);
			body += res.body.substr(0, chunkSize);
			res.body = res.body.substr(chunkSize + 2);
		} catch (std::exception &e) {
			MessageBox(SokuLib::window, res.body.substr(0, 1000).c_str(), e.what(), MB_ICONERROR);
		}
	}
	res.body = body;
}

Socket::HttpResponse makeRequest(const std::string &link)
{
	Socket socket;
	std::string url = link.substr(7);
	auto pos = url.find_first_of('/');
	std::string host = pos == std::string::npos ? url : url.substr(0, pos);
	unsigned short port = 80;
	auto colon = host.find(':');
	auto path = url.substr(host.size());

	printf("%s -> ", link.c_str());
	if (colon != std::string::npos) {
		port = std::stoul(host.substr(colon + 1));
		host = host.substr(0, colon);
	}

	Socket::HttpRequest request;

	request.httpVer = "HTTP/1.1";
	request.method = "GET";
	request.host = host;
	request.portno = port;
	request.header["Accept"] = "image/png";
	request.header["User-Agent"] = "TrialMode " VERSION_STR;
	request.path = pos == std::string::npos ? "/" : path;
	try {
		auto res = socket.makeHttpRequest(request);

		printf("%i (%s)\n", res.returnCode, res.codeName.c_str());
		if (res.header["Transfer-Encoding"] == "chunked")
			unchunkBody(res);
		return res;
	} catch (HTTPErrorException &e) {
		printf("%i (%s)\n", e.getResponse().returnCode, e.getResponse().codeName.c_str());
		throw;
	}
}

SimpleImage::SimpleImage(const std::string &path, const SokuLib::Vector2i &pos)
{
	if (path.substr(0, 7) == "http://")
		loadFromLink(this->texture, path);
	else
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
{
}

void SimpleImage::render() const
{
	if (this->texture.hasTexture())
		this->draw();
}

void SimpleImage::reset()
{
}

bool SimpleImage::isValid() const
{
	return this->texture.hasTexture();
}

void SimpleImage::setPosition(SokuLib::Vector2i pos)
{
	SokuLib::DrawUtils::Sprite::setPosition(pos);
}

void loadFromLink(SokuLib::DrawUtils::Texture &texture, const std::string &link)
{
	HRESULT result;
	int handle;
	D3DXIMAGE_INFO info;
	Socket::HttpResponse res;

	try {
		res = makeRequest(link);
	} catch (std::exception &e) {
		puts(e.what());
	}
	printf("Loading texture %s\n", link.c_str());
	if (FAILED(result = D3DXGetImageInfoFromFileInMemory(res.body.c_str(), res.body.size(), &info))) {
		fprintf(stderr, "D3DXGetImageInfoFromFileInMemory(%p, %u, %p) failed with code %li.\n", res.body.c_str(), res.body.size(), &info, result);
		return;
	}

	LPDIRECT3DTEXTURE9 *pphandle = SokuLib::textureMgr.allocate(&handle);

	*pphandle = nullptr;
	if (FAILED(result = D3DXCreateTextureFromFileInMemoryEx(
		SokuLib::pd3dDev,
		res.body.c_str(),
		res.body.size(),
		info.Width,
		info.Height,
		info.MipLevels,
		D3DUSAGE_RENDERTARGET,
		info.Format,
		D3DPOOL_DEFAULT,
		D3DX_DEFAULT,
		D3DX_DEFAULT,
		0,
		&info,
		nullptr,
		pphandle
	))) {
		fprintf(stderr, "D3DXCreateTextureFromFileInMemoryEx(%p, %p, %u, %p) failed with code %li.\n", SokuLib::pd3dDev, res.body.c_str(), res.body.size(), pphandle, result);
		SokuLib::textureMgr.deallocate(handle);
		return;
	}
	printf("Texture handle: %x, Size: %ux%u\n", handle, info.Width, info.Height);
	texture.setHandle(handle, {info.Width, info.Height});
}

static void callback(void *data, struct GIF_WHDR *chunk)
{
	auto This = (AnimatedImage *)data;

	This->processChunk(chunk);
}

static DWORD __stdcall loader(void *data)
{
	auto This = (AnimatedImage *)data;

	This->dmutex.lock();
	printf("Loading GIF file %s\n", This->path.c_str());
	if (GIF_Load((void *)This->buf, This->fad.nFileSizeLow, callback, nullptr, This, 0) < 0) {
		fprintf(stderr, "Could not load GIF %s\n", This->path.c_str());
		delete[] This->buf;
		This->buf = nullptr;
		This->dmutex.unlock();
		return 0;
	}
	This->dmutex.unlock();
	This->_loading = false;

	printf("GIF %dx%d\n", This->_size.x, This->_size.y);
	This->thread = INVALID_HANDLE_VALUE;
	return 0;
}

AnimatedImage::AnimatedImage(const std::string &p, const SokuLib::Vector2i &pos, bool antiAliasing)
{
	puts("AnimatedImage() <-");
	this->path = p;

	if (this->path.substr(0, 7) == "http://") {
		auto res = makeRequest(this->path);

		this->fad.nFileSizeLow = res.body.size();
		this->buf = new unsigned char[this->fad.nFileSizeLow];
		memcpy(this->buf, res.body.c_str(), this->fad.nFileSizeLow);
	} else {
		std::ifstream stream{this->path, std::ifstream::binary};

		if (!GetFileAttributesEx(path.c_str(), GetFileExInfoStandard, &this->fad)) {
			fprintf(stderr, "GetFileAttributesEx(%s) failed with code %lX\n", this->path.c_str(), GetLastError());
			return;
		}

		if (this->fad.nFileSizeHigh) {
			fprintf(stderr, "%s is too big\n", this->path.c_str());
			return;
		}

		if (!stream) {
			fprintf(stderr, "Could not open %s\n", this->path.c_str());
			return;
		}
		this->buf = new unsigned char[this->fad.nFileSizeLow];

		printf("Reading file %s\n", this->path.c_str());
		stream.read(reinterpret_cast<char *>(this->buf), this->fad.nFileSizeLow);
	}
	this->_sprite.setPosition(pos);
	this->_loading = true;
	this->thread = CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(loader), this, 0, nullptr);
	puts("AnimatedImage() ->");
}

AnimatedImage::~AnimatedImage()
{
	puts("~AnimatedImage() <-");
	this->_needExit = true;
	this->destroymutex.lock();
	delete[] this->_frame;
	delete[] this->_lastFrame;
	this->destroymutex.unlock();
	this->dmutex.lock();
	delete[] this->buf;
	this->dmutex.unlock();
	puts("~AnimatedImage() ->");
}

void AnimatedImage::_init()
{
	HRESULT ret;
	int id;
	auto ddst = (uint32_t)(this->_size.x * this->_size.y);

	this->_frame = new(std::nothrow) SokuLib::DrawUtils::DxSokuColor[ddst];
	this->_lastFrame = new(std::nothrow) SokuLib::DrawUtils::DxSokuColor[ddst];
	if (!this->_frame || !this->_lastFrame)
		fprintf(stderr, "Memory allocation error\n");

	this->_pphandle = SokuLib::textureMgr.allocate(&id);
	if (FAILED(ret = D3DXCreateTexture(SokuLib::pd3dDev, 200, 150, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, this->_pphandle))) {
		SokuLib::textureMgr.deallocate(id);
		fprintf(stderr, "D3DXCreateTexture(SokuLib::pd3dDev, 200, 150, D3DX_DEFAULT, D3DUSAGE_RENDERTARGET, D3DFMT_A8B8G8R8, D3DPOOL_DEFAULT, %p) failed with code %i\n", this->_pphandle, ret);
		return;
	}
	this->_sprite.texture.setHandle(id, this->_size);
	this->_sprite.rect = {
		0, 0,
		static_cast<int>(this->_sprite.texture.getSize().x),
		static_cast<int>(this->_sprite.texture.getSize().y),
	};
	this->_sprite.setSize({200, 150});
	this->_updateTexture();
}

void AnimatedImage::update()
{
	this->_internalCtr += 1000 / 60.f;
	this->mutex.lock();
	if (this->_frames.empty())
		return this->mutex.unlock();
	while (this->_internalCtr >= this->_frames[this->_currentFrame]->delay) {
		if (!this->_loading || this->_currentFrame < this->_frames.size()) {
			this->_internalCtr -= this->_frames[this->_currentFrame]->delay;
			this->_currentFrame++;
			this->_currentFrame %= this->_frames.size();
			this->_updateTexture();
		} else
			this->_internalCtr = this->_frames[this->_currentFrame]->delay;
	}
	this->mutex.unlock();
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
	this->dmutex.unlock();
	this->_size.x = chunk->xdim;
	this->_size.y = chunk->ydim;
	if (this->_needExit)
		ExitThread(0);

	this->destroymutex.lock();
	uint32_t x, y, yoff, iter, ifin, dsrc, ddst;
	SokuLib::DrawUtils::DxSokuColor *pict, *prev;

	if (!chunk->ifrm)
		this->_init();
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
	this->mutex.lock();
	this->_frames.emplace_back(frameObj);
	this->mutex.unlock();
	this->dmutex.lock();
	if (this->_needExit) {
		this->destroymutex.unlock();
		this->dmutex.unlock();
		ExitThread(0);
	}
	this->destroymutex.unlock();
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
}

bool AnimatedImage::isValid() const
{
	return this->_sprite.texture.hasTexture();
}

void AnimatedImage::setPosition(SokuLib::Vector2i pos)
{
	this->_sprite.setPosition(pos);
}
