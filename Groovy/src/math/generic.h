#pragma once

namespace math
{
	constexpr double PI = 3.14159265359;

	template<typename T>
	T Clamp(T value, T min, T max)
	{
		if (value < min)
			return min;
		if (value > max)
			return max;
		return value;
	}

	inline constexpr float DegToRad(float degrees)
	{
		return (float)(degrees * PI / 180.0);
	}
}