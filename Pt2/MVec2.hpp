#pragma once

#include <App.hpp>

#include <initializer_list>
#include <stdexcept>
#include <cmath>
#include <stdio.h>

template <typename real>
class MVec2;

using MVec2f32 = MVec2<f32>;
using MVec2f64 = MVec2<f64>;

// Implementation of CGA for 2D space
template <typename real>
class MVec2
{
public:
	MVec2(real e0, real e1, real e2, real e12)
		: e0(e0)
		, e1(e1), e2(e2), ep(0), en(0)
		, e12(e12), e1p(0), e1n(0), e2p(0), e2n(0), epn(0)
		, e12p(0), e12n(0) , e1pn(0), e2pn(0)
		, e12pn(0)
	{}

	MVec2(std::initializer_list<real> list)
	{
		if (list.size() != 16)
			throw std::invalid_argument("Initializer list must contain exactly 16 elements.");
		std::copy(list.begin(), list.end(), data);
	}

	// Members
	inline real GetE0() const { return e0; }
	inline real GetE1() const { return e1; }
	inline real GetE2() const { return e2; }
	inline real GetE12() const { return e12; }
	inline void SetE0(real v) { e0 = v; }
	inline void SetE1(real v) { e1 = v; }
	inline void SetE2(real v) { e2 = v; }
	inline void SetE12(real v) { e12 = v; }

	// Negate operator
	inline MVec2 operator-() const { return MVec2(-e0, -e1, -e2, -e12); }

	// Reverse operator
	inline MVec2 operator~() const { return MVec2(e0, e1, e2, -e12); }

	// Dual
	inline MVec2 GetDual() const { return MVec2(-e12, -e2, e1, e0); }

	// Inverse
	inline MVec2 GetInverse() const
	{
		const auto& z0 = *this;
		const real norm = (z0 % z0).e0;
		const real inorm = norm == 0.f ? 0.f : 1.f / norm;
		return ~z0 * inorm;
	}

	// Addition
	inline MVec2 operator+(const MVec2& z) const
	{
		return MVec2
		(
			e0 + z.e0, e1 + z.e1, e2 + z.e2, e12 + z.e12
		);
	}

	// Subtraction
	inline MVec2 operator-(const MVec2& z) const
	{
		return MVec2
		(
			e0 - z.e0, e1 - z.e1, e2 - z.e2, e12 - z.e12
		);
	}

	// Scalar product
	inline MVec2 operator*(real s) const
	{
		return MVec2
		(
			e0 * s, e1 * s, e2 * s, e12 * s
		);
	}

	// Scalar division
	inline MVec2 operator/(real s) const
	{
		s = s == 0.f ? 0.f : s;
		return MVec2
		(
			e0 / s, e1 / s, e2 / s, e12 / s
		);
	}

	// Scalar product
	inline MVec2 operator%(const MVec2& z) const
	{
		return MVec2
		(
			e0 * z.e0 + e1 * z.e1 + e2 * z.e2 - e12 * z.e12,
			0,
			0,
			0
		);
	}

	// TODO: Left contraction
	inline MVec2 operator<<(const MVec2& z) const
	{
		return MVec2
		(
			0,
			0,
			0,
			0
		);
	}

	// TODO: Right contraction
	inline MVec2 operator>>(const MVec2& z) const
	{
		return MVec2
		(
			0,
			0,
			0,
			0
		);
	}

	// Outer product
	inline MVec2 operator^(const MVec2& z) const
	{
		return MVec2
		(
			0,
			e0 * z.e1 + e1 * z.e0 - e2 * z.e12 + e12 * z.e2,
			e0 * z.e2 + e1 * z.e12 + e2 * z.e0 - e12 * z.e1,
			e0 * z.e12 + e1 * z.e2 - e2 * z.e1 + e12 * z.e0
		);
	}

	// Geometric product
	inline MVec2 operator*(const MVec2& z) const
	{
		const auto& z0 = *this;
		return (z0 % z) + (z0 ^ z);
	}

	inline MVec2 operator/(const MVec2& z) const
	{
		const auto& z0 = *this;
		return z0 * z.GetInverse();
	}

	// Meet operator
	inline MVec2 operator|(const MVec2& z) const { return (GetDual() ^ z.GetDual()).GetDual(); }

	// Grade
	inline MVec2 GetGrade(i32 i) const
	{
		switch (i)
		{
		case 0: return MVec2({ e0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 });
		case 1: return MVec2({ 0, e1, e2, ep, en, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 });
		case 2: return MVec2({ 0, 0, 0, e12, e1p, e1n, e2p, e2n, epn, 0, 0, 0, 0, 0, 0, 0 });
		case 3: return MVec2({ 0, 0, 0, 0, 0, 0, 0, 0, 0, e12p, e12n, e1pn, e2pn, 0 });
		case 4: return MVec2({ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, e12pn });

		default: return MVec2(0, 0, 0, 0);
		}
	}

	// Elements
	static MVec2 Point(real x, real y)
	{
		static real half = (real)0.5;
		return MVec2{ 0, x, y, half * (x*x + y*y), 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
	}

	// Exponent
	static MVec2 Exp(const MVec2& z)
	{
		MVec2 result(1, 0, 0, 0);

		constexpr i32 terms = 10;
		for (i32 n = 1; n <= terms; ++n)
		{
			real nf = 1;
			for (i32 f = 2; f <= n; ++f)
			{
				nf *= f;
			}
			real in = 1 / nf;

			MVec2 zp = z;
			for (i32 p = 1; p < n; ++p)
			{
				zp = zp * z;
			}

			result = result + (zp * in);
		}

		return result;
	}

	// Infinity
	static MVec2 Inf()
	{
		return MVec2( 0, 0, 0, 0 );
	}
	
	// Utils
	inline void Draw(u32 color) const
	{
		real a = (std::signbit(e12) ? -1 : 1)* std::sqrt(std::abs(e12));
		App::BeginDraw(true, true, 1.f, 1.f);
		App::AddVertex(0.f, 0.f, -1.f, color);
		App::AddVertex(a, 0.f, -1.f, color);
		App::AddVertex(0, a, -1.f, color);
		App::AddVertex(a, a, -1.f, color);
		App::AddVertex(0, a, -1.f, color);
		App::AddVertex(a, 0.f, -1.f, color);
		App::EndDraw(2);
		App::BeginDraw(true, true, 1.f, 5.f);
		App::AddVertex(0.f, 0.f, 0.f, color);
		App::AddVertex(e1, e2, 0.f, color);
		App::EndDraw(1);
		App::BeginDraw(true, true, 10.f, 1.f);
		App::AddVertex(e1, e2, 1.f, color);
		App::AddVertex(e0, 0.f, 1.f, color);
		App::EndDraw(0);
	}
	
	inline void Debug(const char* label)
	{
		static char buff[256];

		snprintf(buff, sizeof(buff), "%s: %s", label, "e0");
		e0 = App::DebugFloat(buff, e0);

		snprintf(buff, sizeof(buff), "%s: %s", label, "e1");
		e1 = App::DebugFloat(buff, e1);

		snprintf(buff, sizeof(buff), "%s: %s", label, "e2");
		e2 = App::DebugFloat(buff, e2);

		snprintf(buff, sizeof(buff), "%s: %s", label, "e12");
		e12 = App::DebugFloat(buff, e12);
	}

private:
	union
	{
		real data[16];
		struct
		{
			// Grade 0
			real e0 = 0;

			// Grade 1
			real e1 = 0, e2 = 0, ep = 0, en = 0;

			// Grade 2
			real e12 = 0, e1p = 0, e1n = 0, e2p = 0, e2n = 0, epn = 0;

			// Grade 3
			real e12p = 0, e12n = 0, e1pn = 0, e2pn = 0;

			// Grade 4
			real e12pn = 0;
		};
	};
};