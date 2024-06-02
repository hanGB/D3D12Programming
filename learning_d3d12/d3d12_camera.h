#pragma once

#define FIRST_PERSON_CAMERA 0x01
#define THIRD_PERSON_CAMERA 0x02
#define SPACE_SHIP_CAMERA	0x03

struct VS_CB_CAMERA_INFO
{
	XMFLOAT4X4 view;
	XMFLOAT4X4 projection;
};

class PERObject;

class D3D12Camera
{
public:
	D3D12Camera();
	D3D12Camera(D3D12Camera* camera);
	virtual ~D3D12Camera();

	virtual void CreateShaderVariables(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);
	virtual void ReleaseShderVariables();
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* commandList);

	void GenerateViewMatrix();
	void GenerateViewMatrix(XMFLOAT3 position, XMFLOAT3 lookAt, XMFLOAT3 up);
	// 실수 계산 오차 해결을 위한 재계산
	void RegenerateViewMatrix();
	void GenerateProjectionMatrix(float fovAngle, float aspectRatio, float nearPlaneDistance, float farPlaneDistance);

	void SetViewport(int xTopLeft, int yTopLeft, int width, int height, float minZ = 0.0f, float maxZ = 1.0f);
	void SetScissorRect(LONG left, LONG top, LONG right, LONG bottom);

	virtual void SetViewportsAndScissorRect(ID3D12GraphicsCommandList* commandList);

	// getter
	PERObject* GetPlayer();
	DWORD GetMode() const;

	XMFLOAT3& GetLookAt();
	
	XMFLOAT3& GetRightVector();
	XMFLOAT3& GetUpVector();
	XMFLOAT3& GetLookVector();

	float& GetPitch();
	float& GetYaw();
	float& GetRoll();

	XMFLOAT3& GetOffSet();
	
	float GetTimeLag() const;

	XMFLOAT4X4 GetViewMatrix() const;
	XMFLOAT4X4 GetProjectionMatrix() const;

	D3D12_VIEWPORT GetViewport() const;
	D3D12_RECT GetScissorRect() const;

	// setter
	void SetPlayer(PERObject* player);
	void SetMode(DWORD mode);

	void SetOffSet(XMFLOAT3 offSet);
	void SetTimeLag(float timeLag);
 
	virtual void Move(XMFLOAT3& shift);
	virtual void Rotate(float pitch = 0.0f, float yaw = 0.0f, float roll = 0.0f);
	virtual void Update(XMFLOAT3& lookAt, float timeElapsed);
	virtual void SetLookAt(XMFLOAT3& lookAt);

protected:
	// 위치
	XMFLOAT3 m_position;
	// 카메라 로컬 벡터
	XMFLOAT3 m_right;
	XMFLOAT3 m_up;
	XMFLOAT3 m_look;

	// 카메라 각도
	float m_pitch, m_yaw, m_roll;

	// 카메라 설정
	DWORD m_viewMode;

	// 3인칭에서 사용
	// 플레이어가 바라볼 위치 벡터
	XMFLOAT3 m_lookAt;
	// 플레이어와 카메라의 보프셋
	XMFLOAT3 m_offSet;

	// 카메라 랙
	float m_timeLag;

	PERObject* m_player;
	
	// 카메라 변환 행렬
	XMFLOAT4X4 m_view;
	// 투영 변환 행렬
	XMFLOAT4X4 m_projection;

	// 뷰포트, 씨저 사각형
	D3D12_VIEWPORT m_viewport;
	D3D12_RECT m_scissorRect;

	float m_aspectRatio;
};