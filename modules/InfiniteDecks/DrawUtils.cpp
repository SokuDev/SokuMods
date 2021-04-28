//
// Created by PinkySmile on 03/03/2021.
//

#include <d3d9.h>
#include <D3dx9tex.h>
#include <cmath>
#include <SokuLib.hpp>
#include "DrawUtils.hpp"

#ifndef _DEBUG
#define puts(x)
#define printf(...)
#endif

namespace DrawUtils
{
	DxSokuColor::DxSokuColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a) noexcept :
		r(r),
		g(g),
		b(b),
		a(a)
	{
	}

	DxSokuColor::DxSokuColor(unsigned int color) noexcept :
		color(color)
	{
	}

	DxSokuColor::operator unsigned() const
	{
		return this->color;
	}

	DxSokuColor &DxSokuColor::operator=(unsigned int color)
	{
		this->color = color;
		return *this;
	}

	DxSokuColor DxSokuColor::operator+(const DxSokuColor &other) const
	{
		DxSokuColor result = *this;

		result += other;
		return result;
	}

	DxSokuColor &DxSokuColor::operator+=(const DxSokuColor &other)
	{
		this->a = std::ceil((other.a + this->a) / 2.);
		this->r = std::ceil((other.r + this->r) / 2.);
		this->g = std::ceil((other.g + this->g) / 2.);
		this->b = std::ceil((other.b + this->b) / 2.);
		return *this;
	}

	DxSokuColor DxSokuColor::operator*(const DxSokuColor &other) const
	{
		DxSokuColor result = *this;

		result *= other;
		return result;
	}

	DxSokuColor &DxSokuColor::operator*=(const DxSokuColor &other)
	{
		this->a = std::ceil((other.a * this->a) / 255.f);
		this->r = std::ceil((other.r * this->r) / 255.f);
		this->g = std::ceil((other.g * this->g) / 255.f);
		this->b = std::ceil((other.b * this->b) / 255.f);
		return *this;
	}

	const DxSokuColor DxSokuColor::White      {0xFF, 0xFF, 0xFF, 0xFF};
	const DxSokuColor DxSokuColor::Red        {0xFF, 0x00, 0x00, 0xFF};
	const DxSokuColor DxSokuColor::Blue       {0x00, 0x00, 0xFF, 0xFF};
	const DxSokuColor DxSokuColor::Green      {0x00, 0xFF, 0x00, 0xFF};
	const DxSokuColor DxSokuColor::Yellow     {0xFF, 0xFF, 0x00, 0xFF};
	const DxSokuColor DxSokuColor::Magenta    {0xFF, 0x00, 0xFF, 0xFF};
	const DxSokuColor DxSokuColor::Cyan       {0x00, 0xFF, 0xFF, 0xFF};
	const DxSokuColor DxSokuColor::Black      {0x00, 0x00, 0x00, 0xFF};
	const DxSokuColor DxSokuColor::Transparent{0x00, 0x00, 0x00, 0x00};

	Texture::Texture(int handle, const Vector2<unsigned> &size) noexcept :
		_handle(handle),
		_loaded(true),
		_size(size)
	{
	}

	Texture::~Texture() noexcept
	{
		this->destroy();
	}

	bool Texture::hasTexture() const noexcept
	{
		return this->_loaded;
	}

	void Texture::activate(int stage) const noexcept
	{
		if (this->hasTexture())
			SokuLib::textureMgr.setTexture(this->_handle, stage);
		else
			SokuLib::textureMgr.setTexture(0, stage);
	}

	void Texture::setHandle(int handle, const Vector2<unsigned> &size)
	{
		this->destroy();
		this->_loaded = true;
		this->_handle = handle;
		this->_size = size;
	}

	void Texture::swap(Texture &other)
	{
		int handle = this->_handle;
		bool loaded = this->_loaded;

		this->_handle = other._handle;
		this->_loaded = other._loaded;
		other._handle = handle;
		other._loaded = loaded;
	}

	void Texture::destroy()
	{
		if (this->_loaded)
			SokuLib::textureMgr.remove(this->_handle);
		this->_loaded = false;
	}

	int Texture::releaseHandle()
	{
		this->_loaded = false;
		return this->_handle;
	}

	Vector2<unsigned> Texture::getSize() const
	{
		return this->_size;
	}

	bool Texture::loadFromFile(Texture &buffer, const char *path)
	{
		HRESULT result;
		int handle;
		D3DXIMAGE_INFO info;

		printf("Loading texture %s\n", path);
		if (FAILED(result = D3DXGetImageInfoFromFile(path, &info))) {
			fprintf(stderr, "D3DXGetImageInfoFromFile(\"%s\", %p) failed with code %li.\n", path, &info, result);
			return false;
		}

		LPDIRECT3DTEXTURE9 *pphandle = SokuLib::textureMgr.allocate(&handle);

		*pphandle = nullptr;
		if (FAILED(result = D3DXCreateTextureFromFileExA(
			SokuLib::pd3dDev,
			path,
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
			fprintf(stderr, "D3DXCreateTextureFromFile(%p, \"%s\", %p) failed with code %li.\n", SokuLib::pd3dDev, path, pphandle, result);
			SokuLib::textureMgr.deallocate(handle);
			return false;
		}
		printf("Texture handle: %x, Size: %ux%u\n", handle, info.Width, info.Height);
		buffer.setHandle(handle, {info.Width, info.Height});
		return true;
	}

	Vector2<unsigned> RectangularRenderingElement::getSize() const
	{
		return this->_size;
	}

	Vector2<int> RenderingElement::getPosition() const
	{
		return this->_position;
	}

	void RenderingElement::setPosition(const Vector2<int> &newPos)
	{
		this->_position = newPos;
	}

	void RectangularRenderingElement::setSize(const Vector2<unsigned int> &newSize)
	{
		this->_size = newSize;
		if (this->_camera) {
			this->_vertex[2].x = this->_vertex[1].x = this->_camera->scale * (1.f * this->_position.x + this->_size.x + this->_camera->translate.x);
			this->_vertex[2].y = this->_vertex[3].y = this->_camera->scale * (1.f * this->_position.y + this->_size.y + this->_camera->translate.y);
		} else {
			this->_vertex[2].x = this->_vertex[1].x = 1.f * this->_position.x + this->_size.x;
			this->_vertex[2].y = this->_vertex[3].y = 1.f * this->_position.y + this->_size.y;
		}
	}

	void RectangularRenderingElement::setPosition(const Vector2<int> &newPos)
	{
		RenderingElement::setPosition(newPos);
		if (this->_camera) {
			this->_vertex[0].x = this->_vertex[3].x = this->_camera->scale * (this->_camera->translate.x + this->_position.x);
			this->_vertex[2].x = this->_vertex[1].x = this->_camera->scale * (1.f * this->_camera->translate.x + this->_position.x + this->_size.x);
			this->_vertex[0].y = this->_vertex[1].y = this->_camera->scale * (this->_camera->translate.y + this->_position.y);
			this->_vertex[2].y = this->_vertex[3].y = this->_camera->scale * (1.f * this->_camera->translate.y + this->_position.y + this->_size.y);
		} else {
			this->_vertex[0].x = this->_vertex[3].x = this->_position.x;
			this->_vertex[2].x = this->_vertex[1].x = 1.f * this->_position.x + this->_size.x;
			this->_vertex[0].y = this->_vertex[1].y = this->_position.y;
			this->_vertex[2].y = this->_vertex[3].y = 1.f * this->_position.y + this->_size.y;
		}
	}

	RectangularRenderingElement::RectangularRenderingElement(const SokuLib::Camera &camera) noexcept :
		_camera(&camera)
	{
	}

	void RectangularRenderingElement::setRect(const FloatRect &rect)
	{
		Vector2<int> pos{
			static_cast<int>(min(rect.x1, rect.x2)),
			static_cast<int>(min(rect.y1, rect.y2))
		};
		Vector2<unsigned> size{
			static_cast<unsigned int>(std::abs(rect.x1 - rect.x2)),
			static_cast<unsigned int>(std::abs(rect.y1 - rect.y2))
		};

		this->setSize(size);
		this->setPosition(pos);
	}

	void RectangularRenderingElement::rawSetRect(const Rect<Vector2<float>> &rect)
	{
		const Vector2<float> (&array)[4] = *reinterpret_cast<const Vector2<float>(*)[4]>(&rect);

		for (int i = 0; i < 4; i++) {
			this->_vertex[i].x = array[i].x;
			this->_vertex[i].y = array[i].y;
		}
	}

	void GradiantRect::draw() const
	{
		Vertex vertexs[4];
		Vertex borders[5];

		for (int i = 0; i < 4; i++) {
			vertexs[i] = borders[i] = this->_vertex[i];
			vertexs[i].color = this->fillColors[i];
			borders[i].color = this->borderColors[i];
		}
		borders[4] = borders[0];
		SokuLib::textureMgr.setTexture(0, 0);
		SokuLib::pd3dDev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vertexs, sizeof(*vertexs));
		SokuLib::pd3dDev->DrawPrimitiveUP(D3DPT_LINESTRIP, 4, borders, sizeof(*borders));
	}

	GradiantRect::GradiantRect(const SokuLib::Camera &camera) noexcept :
		RectangularRenderingElement(camera)
	{
	}

	void RectangleShape::draw() const
	{
		GradiantRect::draw();
	}

	void RectangleShape::setFillColor(const DxSokuColor &color)
	{
		for (auto &fillColor : this->fillColors)
			fillColor = color;
	}

	void RectangleShape::setBorderColor(const DxSokuColor &color)
	{
		for (auto &borderColor : this->borderColors)
			borderColor = color;
	}

	DxSokuColor RectangleShape::getFillColor() const
	{
		return *this->fillColors;
	}

	DxSokuColor RectangleShape::getBorderColor() const
	{
		return *this->borderColors;
	}

	RectangleShape::RectangleShape(const SokuLib::Camera &camera) noexcept :
		GradiantRect(camera)
	{
	}

	void Sprite::draw() const
	{
		Vertex vertexs[4];

		for (int i = 0; i < 4; i++) {
			vertexs[i] = this->_vertex[i];
			vertexs[i].x -= 0.5;
			vertexs[i].y -= 0.5;
			vertexs[i].color = this->fillColors[i] * this->tint;
		}

		auto size = this->texture.getSize();

		if (size.x && size.y) {
			float left = static_cast<float>(this->rect.left) / size.x;
			float top = static_cast<float>(this->rect.top) / size.y;
			float right = static_cast<float>(this->rect.left + this->rect.width) / size.x;
			float bottom = static_cast<float>(this->rect.top + this->rect.height) / size.y;

			vertexs[3].u = vertexs[0].u = left;
			vertexs[2].u = vertexs[1].u = right;
			vertexs[1].v = vertexs[0].v = top;
			vertexs[2].v = vertexs[3].v = bottom;
			vertexs[2].x++;
			vertexs[1].x++;
			vertexs[2].y++;
			vertexs[3].y++;
		}
		this->texture.activate();
		SokuLib::pd3dDev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vertexs, sizeof(*vertexs));
	}
}