#pragma once

class MathHelper
{
public:
	// [0, 1)
	static float RandF()
	{
		return (float)(rand()) / (float)RAND_MAX;
	}

	// [a, b)
	static float RandF(float a, float b)
	{
		return a + RandF() * (b - a);
	}
	static int Rand(int a, int b)
	{
		return a + rand() % ((b - a) + 1);
	}

	template<typename T>
	static T Min(const T& a, const T& b)
	{
		return a < b ? a : b;
	}

	template<typename T>
	static T Max(const T& a, const T& b)
	{
		return a > b ? a : b;
	}

	template<typename T>
	static T Lerp(const T& a, const T& b, float t)
	{
		return a + (b - a) * t;
	}

	template<typename T>
	static T Clamp(const T& x, const T& low, const T& high)
	{
		return x < low ? low : (x > high ? high : x);
	}

	// [0, 2*PI) 극 좌표계 x, y
	static float AngleFromXY(float x, float y);

	static XMVECTOR SphericalToCartesian(float radius, float theta, float phi);

	static XMMATRIX InverseTranspose(CXMMATRIX M);
	static XMFLOAT4X4 Identity4x4();

	static XMVECTOR RandUnitVec3();
	static XMVECTOR RandHemisphereUnitVec3(XMVECTOR n);

	static const float Infinity;
	static const float Pi;
};