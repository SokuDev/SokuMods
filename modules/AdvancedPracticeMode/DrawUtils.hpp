//
// Created by PinkySmile on 03/03/2021.
//

#ifndef SWRSTOYS_DRAWUTILS_HPP
#define SWRSTOYS_DRAWUTILS_HPP


#include <windows.h>
#include <d3d9types.h>

namespace Practice
{
	template<typename T>
	struct Vector2 {
		T x;
		T y;
	};

	struct DxSokuColor {
		union {
			struct {
				unsigned char b;
				unsigned char g;
				unsigned char r;
				unsigned char a;
			};
			unsigned color;
		};

		DxSokuColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a) noexcept;
		DxSokuColor(unsigned color) noexcept;

		DxSokuColor &operator=(unsigned color);
		DxSokuColor operator+(const DxSokuColor &other) const;
		DxSokuColor &operator+=(const DxSokuColor &other);

		template<typename T, typename = std::enable_if_t<std::is_arithmetic<T>::value>>
		DxSokuColor operator*(T alpha) const
		{
			DxSokuColor result = *this;

			result *= alpha;
			return result;
		}

		template<typename T, typename = std::enable_if_t<std::is_arithmetic<T>::value>>
		DxSokuColor &operator*=(T alpha)
		{
			this->a = std::min<unsigned char>(this->a * alpha, 255);
			return *this;
		}

		operator unsigned() const;

		static const DxSokuColor White;
		static const DxSokuColor Yellow;
		static const DxSokuColor Red;
		static const DxSokuColor Blue;
		static const DxSokuColor Green;
		static const DxSokuColor Magenta;
		static const DxSokuColor Cyan;
		static const DxSokuColor Black;
		static const DxSokuColor Transparent;
	};

	class Texture {
	private:
		int _handle;
		bool _loaded = false;
		Vector2<unsigned> _size;

	public:
		Texture() noexcept = default;
		Texture(int handle, const Vector2<unsigned> &size) noexcept;
		Texture(Texture &) = delete;
		Texture(Texture &&) = delete;
		Texture &operator=(Texture &) = delete;
		Texture &operator=(Texture &&) = delete;
		~Texture() noexcept;

		bool hasTexture() const noexcept;
		void activate(int stage = 0) const noexcept;
		Vector2<unsigned> getSize() const;

		void setHandle(int handle, const Vector2<unsigned> &size);
		void swap(Texture &other);
		void destroy();
		int releaseHandle();

		static bool loadFromFile(Texture &buffer, const char *path);
	};

	class RenderingElement {
	protected:
		Vector2<int> _position = {0, 0};

	public:
		Vector2<int> getPosition() const;

		virtual void setPosition(const Vector2<int> &);
		virtual void draw() const = 0;
	};

	struct Vertex {
		float x, y, z;
		float rhw; // RHW = reciprocal of the homogeneous (clip space) w coordinate of a vertex (the 4th dimension for computing the scaling and translating)
		D3DCOLOR color;
		float u, v;
	};

	class RectangularRenderingElement : public RenderingElement {
	private:
		Vector2<unsigned> size = {0, 0};

	protected:
		Vertex _vertex[4] = {
			{0, 0, 0, 1, 0x00000000, 0.0f, 0.0f},
			{0, 0, 0, 1, 0x00000000, 1.0f, 0.0f},
			{0, 0, 0, 1, 0x00000000, 1.0f, 1.0f},
			{0, 0, 0, 1, 0x00000000, 0.0f, 1.0f},
		};

	public:
		Vector2<unsigned> getSize() const;

		void setSize(const Vector2<unsigned> &);
		void setPosition(const Vector2<int> &) override;
	};

	struct TextureRect {
		int left;
		int top;
		unsigned width;
		unsigned height;
	};

	class Sprite : public RectangularRenderingElement {
	public:
		TextureRect rect;
		DxSokuColor tint = 0xFFFFFFFF;
		Texture texture;

		void draw() const override;
	};

	class GradiantRect : public RectangularRenderingElement {
	public:
		enum Corner {
			RECT_TOP_LEFT_CORNER,
			RECT_TOP_RIGHT_CORNER,
			RECT_BOTTOM_RIGHT_CORNER,
			RECT_BOTTOM_LEFT_CORNER,
		};

		DxSokuColor fillColors[4] = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};
		DxSokuColor borderColors[4] = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};

		void draw() const override;
	};

	class Rect : protected GradiantRect {
	public:
		void setFillColor(const DxSokuColor &color);
		void setBorderColor(const DxSokuColor &color);

		DxSokuColor getFillColor() const;
		DxSokuColor getBorderColor() const;

		void draw() const override;
		using RectangularRenderingElement::setSize;
		using RectangularRenderingElement::getSize;
		using RenderingElement::getPosition;
		using RectangularRenderingElement::setPosition;
	};
}


#endif //SWRSTOYS_DRAWUTILS_HPP
