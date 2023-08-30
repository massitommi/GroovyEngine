#pragma once

#include "generic.h"
#include "core/api.h"

#define VEC_TO_RAD(v) { math::DegToRad(v.x), math::DegToRad(v.y), math::DegToRad(v.z) }
#define VEC_TO_XVEC(v) { v.x, v.y, v.z, 1.0f }

struct Vec2
{
	float x, y;

	inline Vec2 operator-() { return { -x, -y }; }
};

struct Vec3
{
	float x, y, z;

	inline Vec3 operator-() { return { -x, -y, -z }; }

	inline operator Vec2() { return { x, y }; }
};

struct Vec4
{
	float x, y, z, w;

	inline Vec4 operator-() { return { -x, -y, -z, -w }; }

	inline operator Vec2() { return { x, y }; }
	inline operator Vec3() { return { x, y, z }; }
};

// vec2

inline Vec2 operator+(Vec2 v, float f)
{
	return { v.x + f, v.y + f };
}

inline Vec2 operator-(Vec2 v, float f)
{
	return { v.x - f, v.y - f };
}

inline Vec2 operator*(Vec2 v, float f)
{
	return { v.x * f, v.y * f };
}

inline Vec2 operator/(Vec2 v, float f)
{
	return { v.x / f, v.y / f };
}

inline Vec2 operator+(Vec2 v1, Vec2 v2)
{
	return { v1.x + v2.x, v1.y + v2.y };
}

inline Vec2 operator-(Vec2 v1, Vec2 v2)
{
	return { v1.x - v2.x, v1.y - v2.y };
}

inline Vec2 operator*(Vec2 v1, Vec2 v2)
{
	return { v1.x * v2.x, v1.y * v2.y };
}

inline Vec2 operator/(Vec2 v1, Vec2 v2)
{
	return { v1.x / v2.x, v1.y / v2.y };
}

inline Vec2& operator+=(Vec2& v, float f)
{
	v.x += f;
	v.y += f;
	return v;
}

inline Vec2& operator-=(Vec2& v, float f)
{
	v.x -= f;
	v.y -= f;
	return v;
}

inline Vec2& operator*=(Vec2& v, float f)
{
	v.x *= f;
	v.y *= f;
	return v;
}

inline Vec2& operator/=(Vec2& v, float f)
{
	v.x /= f;
	v.y /= f;
	return v;
}

inline Vec2& operator+=(Vec2& v1, Vec2 v2)
{
	v1.x += v2.x;
	v1.y += v2.y;
	return v1;
}

inline Vec2& operator-=(Vec2& v1, Vec2 v2)
{
	v1.x -= v2.x;
	v1.y -= v2.y;
	return v1;
}

inline Vec2& operator*=(Vec2& v1, Vec2 v2)
{
	v1.x *= v2.x;
	v1.y *= v2.y;
	return v1;
}

inline Vec2& operator/=(Vec2& v1, Vec2 v2)
{
	v1.x /= v2.x;
	v1.y /= v2.y;
	return v1;
}

// vec3

inline Vec3 operator+(Vec3 v, float f)
{
	return { v.x + f, v.y + f, v.z + f };
}

inline Vec3 operator-(Vec3 v, float f)
{
	return { v.x - f, v.y - f, v.z - f };
}

inline Vec3 operator*(Vec3 v, float f)
{
	return { v.x * f, v.y * f, v.z * f };
}

inline Vec3 operator/(Vec3 v, float f)
{
	return { v.x / f, v.y / f, v.z / f };
}

inline Vec3 operator+(Vec3 v1, Vec3 v2)
{
	return { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
}

inline Vec3 operator-(Vec3 v1, Vec3 v2)
{
	return { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
}

inline Vec3 operator*(Vec3 v1, Vec3 v2)
{
	return { v1.x * v2.x, v1.y * v2.y, v1.z * v2.z };
}

inline Vec3 operator/(Vec3 v1, Vec3 v2)
{
	return { v1.x / v2.x, v1.y / v2.y, v1.z / v2.z };
}

inline Vec3& operator+=(Vec3& v, float f)
{
	v.x += f;
	v.y += f;
	v.z += f;
	return v;
}

inline Vec3& operator-=(Vec3& v, float f)
{
	v.x -= f;
	v.y -= f;
	v.z -= f;
	return v;
}

inline Vec3& operator*=(Vec3& v, float f)
{
	v.x *= f;
	v.y *= f;
	v.z *= f;
	return v;
}

inline Vec3& operator/=(Vec3& v, float f)
{
	v.x /= f;
	v.y /= f;
	v.z /= f;
	return v;
}

inline Vec3& operator+=(Vec3& v1, Vec3 v2)
{
	v1.x += v2.x;
	v1.y += v2.y;
	v1.z += v2.z;
	return v1;
}

inline Vec3& operator-=(Vec3& v1, Vec3 v2)
{
	v1.x -= v2.x;
	v1.y -= v2.y;
	v1.z -= v2.z;
	return v1;
}

inline Vec3& operator*=(Vec3& v1, Vec3 v2)
{
	v1.x *= v2.x;
	v1.y *= v2.y;
	v1.z *= v2.z;
	return v1;
}

inline Vec3& operator/=(Vec3& v1, Vec3 v2)
{
	v1.x /= v2.x;
	v1.y /= v2.y;
	v1.z /= v2.z;
	return v1;
}

struct Transform
{
	Vec3 location;
	Vec3 rotation;
	Vec3 scale;
};

namespace math
{
	CORE_API Vec3 GetForwardVector(Vec3 rotation);
	CORE_API Vec3 GetRightVector(Vec3 rotation);
	CORE_API Vec3 GetUpVector(Vec3 rotation);

	CORE_API float Magnitude(Vec3 v);
	CORE_API Vec3 Normalize(Vec3 v);
}