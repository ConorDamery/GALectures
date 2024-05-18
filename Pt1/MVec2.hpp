#pragma once

#include <App.hpp>

#include <cmath>
#include <stdio.h>

class MVec2
{
public:
	MVec2(f32 e0, f32 e1, f32 e2, f32 e12)
		: e0(e0), e1(e1), e2(e2), e12(e12)
	{}

	// Members
	inline f32 GetE0() const { return e0; }
	inline f32 GetE1() const { return e1; }
	inline f32 GetE2() const { return e2; }
	inline f32 GetE12() const { return e12; }
	inline void SetE0(f32 v) { e0 = v; }
	inline void SetE1(f32 v) { e1 = v; }
	inline void SetE2(f32 v) { e2 = v; }
	inline void SetE12(f32 v) { e12 = v; }

	// Geometric product
	inline MVec2 operator*(const MVec2& z) const
	{
		return MVec2
		{
			e0 * z.e0 + e1 * z.e1 + e2 * z.e2 - e12 * z.e12,
			e0 * z.e1 + e1 * z.e0 - e2 * z.e12 + e12 * z.e2,
			e0 * z.e2 + e1 * z.e12 + e2 * z.e0 - e12 * z.e1,
			e0 * z.e12 + e1 * z.e2 - e2 * z.e1 + e12 * z.e0
		};
	}

	// Inner product
	inline MVec2 operator%(const MVec2& z) const
	{
		return MVec2
		{
			e0 * z.e0 + e1 * z.e1 + e2 * z.e2 - e12 * z.e12,
			0,
			0,
			0
		};
	}

	// Outer product
	inline MVec2 operator^(const MVec2& z) const
	{
		return MVec2
		{
			0,
			e0 * z.e1 + e1 * z.e0 - e2 * z.e12 + e12 * z.e2,
			e0 * z.e2 + e1 * z.e12 + e2 * z.e0 - e12 * z.e1,
			e0 * z.e12 + e1 * z.e2 - e2 * z.e1 + e12 * z.e0
		};
	}
	
	// Dual
	inline MVec2 GetDual() const { return MVec2(e12, e2, -e1, e0); }

	// Negate operator
	inline MVec2 operator-() const { return MVec2(e0, -e1, -e2, -e12); }

	// Reverse operator
	inline MVec2 operator~() const { return MVec2(e0, e1, e2, -e12); }

	// Meet operator
	inline MVec2 operator|(const MVec2& z) const { return (GetDual() ^ z.GetDual()).GetDual(); }

	// Grade
	inline MVec2 GetGrade(i32 i) const
	{
		switch (i)
		{
		case 0: return MVec2(e0, 0, 0, 0);
		case 1: return MVec2(0, e1, e2, 0);
		case 2: return MVec2(0, 0, 0, e12);

		default: return MVec2(0, 0, 0, 0);
		}
	}
	
	// Utils
	inline void Draw(u32 color) const
	{
		f32 a = (std::signbit(e12) ? -1 : 1)* std::sqrt(std::abs(e12));
		App::BeginDraw(true, false, 1.f, 1.f);
		App::AddVertex(0.f, 0.f, 0.f, color);
		App::AddVertex(a, 0.f, 0.f, color);
		App::AddVertex(0, a, 0.f, color);
		App::AddVertex(a, a, 0.f, color);
		App::AddVertex(0, a, 0.f, color);
		App::AddVertex(a, 0.f, 0.f, color);
		App::EndDraw(2);
		App::BeginDraw(true, false, 1.f, 5.f);
		App::AddVertex(0.f, 0.f, 0.f, color);
		App::AddVertex(e1, e2, 0.f, color);
		App::EndDraw(1);
		App::BeginDraw(true, false, 10.f, 1.f);
		App::AddVertex(e1, e2, 0.f, color);
		App::AddVertex(e0, 0.f, 0.f, color);
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
	f32 e0 = 0, e1 = 0, e2 = 0, e12 = 0;
};