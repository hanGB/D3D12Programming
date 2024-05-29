#pragma once
#include <DirectXMath.h>
#include <DirectXPackedVector.h>

using namespace DirectX;
using namespace DirectX::PackedVector;

//3차원 벡터의 연산 
namespace Vector3
{
	inline XMFLOAT3 XMVectorToFloat3(XMVECTOR& vector);
	inline XMFLOAT3 ScalarProduct(XMFLOAT3& xmf3Vector, float fScalar, bool bNormalize = true);
	inline XMFLOAT3 Add(const XMFLOAT3& xmf3Vector1, const XMFLOAT3& xmf3Vector2);
	inline XMFLOAT3 Add(XMFLOAT3& xmf3Vector1, XMFLOAT3& xmf3Vector2, float fScalar);
	inline XMFLOAT3 Subtract(XMFLOAT3& xmf3Vector1, XMFLOAT3& xmf3Vector2);
	inline float DotProduct(XMFLOAT3& xmf3Vector1, XMFLOAT3& xmf3Vector2);
	inline XMFLOAT3 CrossProduct(XMFLOAT3& xmf3Vector1, XMFLOAT3& xmf3Vector2, bool bNormalize = true);
	inline XMFLOAT3 Normalize(XMFLOAT3& xmf3Vector);
	inline float Length(XMFLOAT3& xmf3Vector);
	inline float Angle(XMVECTOR& xmvVector1, XMVECTOR& xmvVector2);
	inline float Angle(XMFLOAT3& xmf3Vector1, XMFLOAT3& xmf3Vector2);
	inline XMFLOAT3 TransformNormal(XMFLOAT3& xmf3Vector, XMMATRIX& xmmtxTransform);
	inline XMFLOAT3 TransformCoord(XMFLOAT3& xmf3Vector, XMMATRIX& xmmtxTransform);
	inline XMFLOAT3 TransformCoord(XMFLOAT3& xmf3Vector, XMFLOAT4X4& xmmtx4x4Matrix);
}
//4차원 벡터의 연산
namespace Vector4
{
	inline XMFLOAT4 Add(XMFLOAT4& xmf4Vector1, XMFLOAT4& xmf4Vector2);
	inline XMFLOAT4 Multiply(XMFLOAT4& xmf4Vector1, XMFLOAT4& xmf4Vector2);
	inline XMFLOAT4 Multiply(float fScalar, XMFLOAT4& xmf4Vector);
}
//행렬의 연산
namespace Matrix4x4
{
	inline XMFLOAT4X4 Identity();
	inline XMFLOAT4X4 Multiply(XMFLOAT4X4& xmmtx4x4Matrix1, XMFLOAT4X4& xmmtx4x4Matrix2);
	inline XMFLOAT4X4 Multiply(XMFLOAT4X4& xmmtx4x4Matrix1, XMMATRIX& xmmtxMatrix2);
	inline XMFLOAT4X4 Multiply(XMMATRIX& xmmtxMatrix1, XMFLOAT4X4& xmmtx4x4Matrix2);
	inline XMFLOAT4X4 Inverse(XMFLOAT4X4& xmmtx4x4Matrix);
	inline XMFLOAT4X4 Transpose(XMFLOAT4X4& xmmtx4x4Matrix);
	inline XMFLOAT4X4 PerspectiveFovLH(float FovAngleY, float AspectRatio, float NearZ,
		float FarZ);
	inline XMFLOAT4X4 LookAtLH(XMFLOAT3& xmf3EyePosition, XMFLOAT3& xmf3LookAtPosition,
		XMFLOAT3& xmf3UpDirection);
}
	
