//
// Created by PinkySmile on 03/03/2021.
//

#ifndef SWRSTOYS_DRAWUTILS_HPP
#define SWRSTOYS_DRAWUTILS_HPP


#include <windows.h>
#include <d3dx9.h>
#include <SokuLib.hpp>

namespace Practice
{
	template<typename T>
	struct Vector2 {
		T x;
		T y;

		template<typename T2>
		bool operator==(const Vector2<T2> &other) const
		{
			return this->x == other.x && this->y == other.y;
		}

		template<typename T2>
		Vector2<T> &operator-=(Vector2<T2> other)
		{
			this->x -= other.x;
			this->y -= other.y;
			return *this;
		}

		template<typename T2>
		Vector2<T> operator-(const Vector2<T2> &other) const
		{
			return {
				static_cast<T>(this->x - other.x),
				static_cast<T>(this->y - other.y)
			};
		}

		template<typename T2>
		Vector2<T> &operator+=(Vector2<T2> other)
		{
			this->x += other.x;
			this->y += other.y;
			return *this;
		}

		template<typename T2>
		Vector2<T> operator+(const Vector2<T2> &other) const
		{
			return {
				static_cast<T>(this->x + other.x),
				static_cast<T>(this->y + other.y)
			};
		}

		template<typename T2>
		Vector2<T> &operator*=(T2 scalar)
		{
			this->x *= scalar;
			this->y *= scalar;
			return *this;
		}

		template<typename T2>
		Vector2<T> operator*(T2 scalar) const
		{
			return {
				static_cast<T>(this->x * scalar),
				static_cast<T>(this->y * scalar)
			};
		}

		operator D3DXVECTOR2() const
		{
			return {
				static_cast<float>(this->x),
				static_cast<float>(this->y)
			};
		}
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

		DxSokuColor() = default;
		DxSokuColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a) noexcept;
		DxSokuColor(unsigned color) noexcept;

		DxSokuColor &operator=(unsigned color);
		DxSokuColor operator+(const DxSokuColor &other) const;
		DxSokuColor &operator+=(const DxSokuColor &other);
		DxSokuColor operator*(const DxSokuColor &other) const;
		DxSokuColor &operator*=(const DxSokuColor &other);

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
		static bool loadFromGame(Texture &buffer, const char *path);
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

		template<typename T>
		Vertex operator+(const Vector2<T> &vec) const
		{
			return {
				this->x + vec.x,
				this->y + vec.y,
				this->z,
				this->rhw,
				this->color,
				this->u,
				this->v
			};
		}

		template<typename T>
		Vertex &operator+=(const Vector2<T> &vec)
		{
			this->x += vec.x;
			this->y += vec.y;
			return *this;
		}
	};

	template<typename T>
	struct Rect {
		T x1;
		T y1;
		T x2;
		T y2;
	};
	typedef Rect<float> FloatRect;

	class RectangularRenderingElement : public RenderingElement {
	private:
		Vector2<unsigned> _size = {0, 0};
		const SokuLib::Camera *_camera = nullptr;

	protected:
		Vertex _vertex[4] = {
			{0, 0, 0, 1, 0x00000000, 0.0f, 0.0f},
			{0, 0, 0, 1, 0x00000000, 1.0f, 0.0f},
			{0, 0, 0, 1, 0x00000000, 1.0f, 1.0f},
			{0, 0, 0, 1, 0x00000000, 0.0f, 1.0f},
		};

	public:
		Vector2<unsigned> getSize() const;

		RectangularRenderingElement() noexcept = default;
		RectangularRenderingElement(const SokuLib::Camera &camera) noexcept;
		void setSize(const Vector2<unsigned> &);
		void setPosition(const Vector2<int> &) override;
		void setRect(const FloatRect &rect);
		void rawSetRect(const Rect<Vector2<float>> &rect);
	};

	struct TextureRect {
		int left;
		int top;
		int width;
		int height;
	};

	class Sprite : public RectangularRenderingElement {
	public:
		TextureRect rect;
		DxSokuColor tint = 0xFFFFFFFF;
		DxSokuColor fillColors[4] = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};
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

		GradiantRect() noexcept = default;
		GradiantRect(const SokuLib::Camera &camera) noexcept;
		void draw() const override;
	};

	class RectangleShape : protected GradiantRect {
	public:
		void setFillColor(const DxSokuColor &color);
		void setBorderColor(const DxSokuColor &color);

		DxSokuColor getFillColor() const;
		DxSokuColor getBorderColor() const;

		RectangleShape() noexcept = default;
		RectangleShape(const SokuLib::Camera &camera) noexcept;
		void draw() const override;
		using RectangularRenderingElement::setSize;
		using RectangularRenderingElement::getSize;
		using RenderingElement::getPosition;
		using RectangularRenderingElement::setPosition;
		using RectangularRenderingElement::setRect;
		using RectangularRenderingElement::rawSetRect;
	};
}


#endif //SWRSTOYS_DRAWUTILS_HPP
