#include "dxmath_helper.h"
#include "stdafx.h"


XMFLOAT3 Vector3::XMVectorToFloat3(XMVECTOR& vector)
{
	XMFLOAT3 xmf3Result;
	XMStoreFloat3(&xmf3Result, vector);
	return(xmf3Result);
}
XMFLOAT3 Vector3::ScalarProduct(XMFLOAT3& xmf3Vector, float fScalar, bool bNormalize)
{
	XMFLOAT3 xmf3Result;
	if (bNormalize)
		XMStoreFloat3(&xmf3Result, XMVector3Normalize(XMLoadFloat3(&xmf3Vector)) *
			fScalar);
	else
		XMStoreFloat3(&xmf3Result, XMLoadFloat3(&xmf3Vector) * fScalar);
	return(xmf3Result);
}
XMFLOAT3 Vector3::Add(const XMFLOAT3& xmf3Vector1, const XMFLOAT3& xmf3Vector2)
{
	XMFLOAT3 xmf3Result;
	XMStoreFloat3(&xmf3Result, XMLoadFloat3(&xmf3Vector1) +
		XMLoadFloat3(&xmf3Vector2));
	return(xmf3Result);
}
XMFLOAT3 Vector3::Add(XMFLOAT3& xmf3Vector1, XMFLOAT3& xmf3Vector2, float fScalar)
{
	XMFLOAT3 xmf3Result;
	XMStoreFloat3(&xmf3Result, XMLoadFloat3(&xmf3Vector1) + (XMLoadFloat3(&xmf3Vector2)
		* fScalar));
	return(xmf3Result);
}
XMFLOAT3 Vector3::Subtract(XMFLOAT3& xmf3Vector1, XMFLOAT3& xmf3Vector2)
{
	XMFLOAT3 xmf3Result;
	XMStoreFloat3(&xmf3Result, XMLoadFloat3(&xmf3Vector1) -
		XMLoadFloat3(&xmf3Vector2));
	return(xmf3Result);
}
float Vector3::DotProduct(XMFLOAT3& xmf3Vector1, XMFLOAT3& xmf3Vector2)
{
	XMFLOAT3 xmf3Result;
	XMStoreFloat3(&xmf3Result, XMVector3Dot(XMLoadFloat3(&xmf3Vector1),
		XMLoadFloat3(&xmf3Vector2)));
	return(xmf3Result.x);
}
XMFLOAT3 Vector3::CrossProduct(XMFLOAT3& xmf3Vector1, XMFLOAT3& xmf3Vector2, bool bNormalize)
{
	XMFLOAT3 xmf3Result;
	if (bNormalize)
		XMStoreFloat3(&xmf3Result,
			XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&xmf3Vector1),
				XMLoadFloat3(&xmf3Vector2))));
	else
		XMStoreFloat3(&xmf3Result, XMVector3Cross(XMLoadFloat3(&xmf3Vector1),
			XMLoadFloat3(&xmf3Vector2)));
	return(xmf3Result);
}
XMFLOAT3 Vector3::Normalize(XMFLOAT3& xmf3Vector)
{
	XMFLOAT3 m_xmf3Normal;
	XMStoreFloat3(&m_xmf3Normal, XMVector3Normalize(XMLoadFloat3(&xmf3Vector)));
	return(m_xmf3Normal);
}
float Vector3::Length(XMFLOAT3& xmf3Vector)
{
	XMFLOAT3 xmf3Result;
	XMStoreFloat3(&xmf3Result, XMVector3Length(XMLoadFloat3(&xmf3Vector)));
	return(xmf3Result.x);
}
float Vector3::Angle(XMVECTOR& xmvVector1, XMVECTOR& xmvVector2)
{
	XMVECTOR xmvAngle = XMVector3AngleBetweenNormals(xmvVector1, xmvVector2);
	return(XMConvertToDegrees(acosf(XMVectorGetX(xmvAngle))));
}
float Vector3::Angle(XMFLOAT3& xmf3Vector1, XMFLOAT3& xmf3Vector2)
{
	XMVECTOR xmvAngle = XMVector3AngleBetweenNormals(XMLoadFloat3(&xmf3Vector1), XMLoadFloat3(&xmf3Vector2));
	return(XMConvertToDegrees(acosf(XMVectorGetX(xmvAngle))));
}
XMFLOAT3 Vector3::TransformNormal(XMFLOAT3& xmf3Vector, XMMATRIX& xmmtxTransform)
{
	XMFLOAT3 xmf3Result;
	XMStoreFloat3(&xmf3Result, XMVector3TransformNormal(XMLoadFloat3(&xmf3Vector),
		xmmtxTransform));
	return(xmf3Result);
}
XMFLOAT3 Vector3::TransformCoord(XMFLOAT3& xmf3Vector, XMMATRIX& xmmtxTransform)
{
	XMFLOAT3 xmf3Result;
	XMStoreFloat3(&xmf3Result, XMVector3TransformCoord(XMLoadFloat3(&xmf3Vector),
		xmmtxTransform));
	return(xmf3Result);
}
XMFLOAT3 Vector3::TransformCoord(XMFLOAT3& xmf3Vector, XMFLOAT4X4& xmmtx4x4Matrix)
{
	XMFLOAT3 xmf3Result;
	XMStoreFloat3(&xmf3Result, XMVector3TransformCoord(XMLoadFloat3(&xmf3Vector),
		XMLoadFloat4x4(&xmmtx4x4Matrix)));
	return(xmf3Result);
}

//4차원 벡터의 연산
XMFLOAT4 Vector4::Add(XMFLOAT4& xmf4Vector1, XMFLOAT4& xmf4Vector2)
{
	XMFLOAT4 xmf4Result;
	XMStoreFloat4(&xmf4Result, XMLoadFloat4(&xmf4Vector1) +
		XMLoadFloat4(&xmf4Vector2));
	return(xmf4Result);
}
XMFLOAT4 Vector4::Multiply(XMFLOAT4& xmf4Vector1, XMFLOAT4& xmf4Vector2)
{
	XMFLOAT4 xmf4Result;
	XMStoreFloat4(&xmf4Result, XMLoadFloat4(&xmf4Vector1) *
		XMLoadFloat4(&xmf4Vector2));
	return(xmf4Result);
}
XMFLOAT4 Vector4::Multiply(float fScalar, XMFLOAT4& xmf4Vector)
{
	XMFLOAT4 xmf4Result;
	XMStoreFloat4(&xmf4Result, fScalar * XMLoadFloat4(&xmf4Vector));
	return(xmf4Result);
}

//행렬의 연산
XMFLOAT4X4 Matrix4x4::Identity()
{
	XMFLOAT4X4 xmmtx4x4Result;
	XMStoreFloat4x4(&xmmtx4x4Result, XMMatrixIdentity());
	return(xmmtx4x4Result);
}
XMFLOAT4X4 Matrix4x4::Multiply(XMFLOAT4X4& xmmtx4x4Matrix1, XMFLOAT4X4& xmmtx4x4Matrix2)
{
	XMFLOAT4X4 xmmtx4x4Result;
	XMStoreFloat4x4(&xmmtx4x4Result, XMLoadFloat4x4(&xmmtx4x4Matrix1) *
		XMLoadFloat4x4(&xmmtx4x4Matrix2));
	return(xmmtx4x4Result);
}
XMFLOAT4X4 Matrix4x4::Multiply(XMFLOAT4X4& xmmtx4x4Matrix1, XMMATRIX& xmmtxMatrix2)
{
	XMFLOAT4X4 xmmtx4x4Result;
	XMStoreFloat4x4(&xmmtx4x4Result, XMLoadFloat4x4(&xmmtx4x4Matrix1) * xmmtxMatrix2);
	return(xmmtx4x4Result);
}
XMFLOAT4X4 Matrix4x4::Multiply(XMMATRIX& xmmtxMatrix1, XMFLOAT4X4& xmmtx4x4Matrix2)
{
	XMFLOAT4X4 xmmtx4x4Result;
	XMStoreFloat4x4(&xmmtx4x4Result, xmmtxMatrix1 * XMLoadFloat4x4(&xmmtx4x4Matrix2));
	return(xmmtx4x4Result);
}
XMFLOAT4X4 Matrix4x4::Inverse(XMFLOAT4X4& xmmtx4x4Matrix)
{
	XMFLOAT4X4 xmmtx4x4Result;
	XMStoreFloat4x4(&xmmtx4x4Result, XMMatrixInverse(NULL,
		XMLoadFloat4x4(&xmmtx4x4Matrix)));
	return(xmmtx4x4Result);
}
XMFLOAT4X4 Matrix4x4::Transpose(XMFLOAT4X4& xmmtx4x4Matrix)
{
	XMFLOAT4X4 xmmtx4x4Result;
	XMStoreFloat4x4(&xmmtx4x4Result,
		XMMatrixTranspose(XMLoadFloat4x4(&xmmtx4x4Matrix)));
	return(xmmtx4x4Result);
}
XMFLOAT4X4 Matrix4x4::PerspectiveFovLH(float FovAngleY, float AspectRatio, float NearZ,
	float FarZ)
{
	XMFLOAT4X4 xmmtx4x4Result;
	XMStoreFloat4x4(&xmmtx4x4Result, XMMatrixPerspectiveFovLH(FovAngleY, AspectRatio,
		NearZ, FarZ));
	return(xmmtx4x4Result);
}
XMFLOAT4X4 Matrix4x4::LookAtLH(XMFLOAT3& xmf3EyePosition, XMFLOAT3& xmf3LookAtPosition,
	XMFLOAT3& xmf3UpDirection)
{
	XMFLOAT4X4 xmmtx4x4Result;
	XMStoreFloat4x4(&xmmtx4x4Result, XMMatrixLookAtLH(XMLoadFloat3(&xmf3EyePosition),
		XMLoadFloat3(&xmf3LookAtPosition), XMLoadFloat3(&xmf3UpDirection)));
	return(xmmtx4x4Result);
}