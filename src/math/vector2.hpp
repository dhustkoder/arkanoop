#ifndef GPROJ_VECTOR2_HPP_
#define GPROJ_VECTOR2_HPP_
#include "math_types.hpp"

namespace gp {


constexpr Vec2 operator+(const Vec2& a, const Vec2& b)
{
	return { a.x + b.x, a.y + b.y };
}


constexpr Vec2 operator-(const Vec2& a, const Vec2& b)
{
	return { a.x - b.x, a.y - b.y };
}


constexpr Vec2 operator*(const Vec2& a, const float scalar)
{
	return { a.x * scalar, a.y * scalar };
}


inline Vec2& operator+=(Vec2& a, const Vec2& b)
{
	a = a + b;
	return a;
}


inline Vec2& operator-=(Vec2& a, const Vec2& b)
{
	a = a - b;
	return a;
}


inline Vec2& operator*=(Vec2& a, const float scalar)
{
	a = a * scalar;
	return a;
}



inline bool operator==(const Vec2& a, const Vec2& b)
{
	if (fabs(a.x - b.x) > 0.00001)
		return false;
	else if (fabs(a.y - b.y) > 0.00001)
		return false;
	return true;
}


inline bool operator!=(const Vec2& a, const Vec2& b)
{
	return !(a == b);
}



} // namespace gp
#endif
