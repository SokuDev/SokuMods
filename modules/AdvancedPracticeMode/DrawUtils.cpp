//
// Created by PinkySmile on 03/03/2021.
//

#include <d3d9.h>
#include <cmath>
#include <algorithm>
#include <SokuLib.hpp>
#include "DrawUtils.hpp"

namespace Practice
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

	const DxSokuColor DxSokuColor::White      {0xFF, 0xFF, 0xFF, 0xFF};
	const DxSokuColor DxSokuColor::Red        {0xFF, 0x00, 0x00, 0xFF};
	const DxSokuColor DxSokuColor::Blue       {0x00, 0x00, 0xFF, 0xFF};
	const DxSokuColor DxSokuColor::Green      {0x00, 0xFF, 0x00, 0xFF};
	const DxSokuColor DxSokuColor::Yellow     {0xFF, 0xFF, 0x00, 0xFF};
	const DxSokuColor DxSokuColor::Magenta    {0xFF, 0x00, 0xFF, 0xFF};
	const DxSokuColor DxSokuColor::Cyan       {0x00, 0xFF, 0x00, 0xFF};
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

	Vector2<unsigned> RectangularRenderingElement::getSize() const
	{
		return this->size;
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
		this->size = newSize;
		this->_vertex[2].x = this->_vertex[1].x = this->_position.x + this->size.x;
		this->_vertex[2].y = this->_vertex[3].y = this->_position.y + this->size.y;
	}

	void RectangularRenderingElement::setPosition(const Vector2<int> &newPos)
	{
		RenderingElement::setPosition(newPos);
		this->_vertex[0].x = this->_vertex[3].x = this->_position.x;
		this->_vertex[2].x = this->_vertex[1].x = this->_position.x + this->size.x;
		this->_vertex[0].y = this->_vertex[1].y = this->_position.y;
		this->_vertex[2].y = this->_vertex[3].y = this->_position.y + this->size.y;
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

	void Rect::draw() const
	{
		GradiantRect::draw();
	}

	void Rect::setFillColor(const DxSokuColor &color)
	{
		for (auto &fillColor : this->fillColors)
			fillColor = color;
	}

	void Rect::setBorderColor(const DxSokuColor &color)
	{
		for (auto &borderColor : this->borderColors)
			borderColor = color;
	}

	DxSokuColor Rect::getFillColor() const
	{
		return *this->fillColors;
	}

	DxSokuColor Rect::getBorderColor() const
	{
		return *this->borderColors;
	}

	void Sprite::draw() const
	{
		Vertex vertexs[4];

		for (int i = 0; i < 4; i++) {
			vertexs[i] = this->_vertex[i];
			vertexs[i].color = this->tint;
		}
		this->texture.activate();

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
		}
		SokuLib::pd3dDev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vertexs, sizeof(*vertexs));
	}
}