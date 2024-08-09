#include "stdafx.h"
#include "math_helper.h"

const float MathHelper::Infinity = FLT_MAX;
const float MathHelper::Pi = 3.1415926535f;

float MathHelper::AngleFromXY(float x, float y)
{
	float theta = 0.0f;

	// 1, 4 사분면
	if (x >= 0.0f)
	{
		theta = atanf(y / x); // [-pi/2, +pi/2]

		if (theta < 0.0f)
			theta += 2.0f * Pi; // [0, 2*pi)
	}

	// 2, 3 사분면
	else
		theta = atanf(y / x) + Pi; // [0, 2*pi)

	return theta;
}

XMVECTOR MathHelper::SphericalToCartesian(float radius, float theta, float phi)
{
	return XMVectorSet(
		radius * sinf(phi) * cosf(theta),
		radius * cosf(phi),
		radius * sinf(phi) * sinf(theta),
		1.0f);
}

XMMATRIX MathHelper::InverseTranspose(CXMMATRIX M)
{
	// 역-전치 행렬을 노멀에만 적용하기 위해 이동 제외
	XMMATRIX A = M;
	A.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

	XMVECTOR det = XMMatrixDeterminant(A);
	return XMMatrixTranspose(XMMatrixInverse(&det, A));
}

XMFLOAT4X4 MathHelper::Identity4x4()
{
	static XMFLOAT4X4 I(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	return I;
}

XMVECTOR MathHelper::RandUnitVec3()
{
	XMVECTOR One = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
	XMVECTOR Zero = XMVectorZero();

	while (true)
	{
		// [-1,1]^3 큐브 위의 랜덤 점 생성.
		XMVECTOR v = XMVectorSet(MathHelper::RandF(-1.0f, 1.0f), MathHelper::RandF(-1.0f, 1.0f), MathHelper::RandF(-1.0f, 1.0f), 0.0f);

		// 단위 구에 균일한 분포를 얻기 위해 단위 구 외부의 점을 무시(큐브 모서리 쪽 점이 더 많이 나오는 현상 방지)
		if (XMVector3Greater(XMVector3LengthSq(v), One))
			continue;

		return XMVector3Normalize(v);
	}
}

XMVECTOR MathHelper::RandHemisphereUnitVec3(XMVECTOR n)
{
	XMVECTOR One = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
	XMVECTOR Zero = XMVectorZero();

	while (true)
	{
		// [-1,1]^3 큐브 위의 랜덤 점 생성.
		XMVECTOR v = XMVectorSet(MathHelper::RandF(-1.0f, 1.0f), MathHelper::RandF(-1.0f, 1.0f), MathHelper::RandF(-1.0f, 1.0f), 0.0f);

		// 단위 구에 균일한 분포를 얻기 위해 단위 구 외부의 점을 무시(큐브 모서리 쪽 점이 더 많이 나오는 현상 방지)
		if (XMVector3Greater(XMVector3LengthSq(v), One))
			continue;

		// 아래쪽 반구의 점 무시
		if (XMVector3Less(XMVector3Dot(n, v), Zero))
			continue;

		return XMVector3Normalize(v);
	}
}