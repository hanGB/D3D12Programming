#pragma once


class Waves
{
public:
    Waves(int m, int n, float dx, float dt, float speed, float damping);
    Waves(const Waves& rhs) = delete;
    Waves& operator=(const Waves& rhs) = delete;
    ~Waves();

    int RowCount()const;
    int ColumnCount()const;
    int VertexCount()const;
    int TriangleCount()const;
    float Width()const;
    float Depth()const;

    // Returns the solution at the ith grid point.
    const XMFLOAT3& Position(int i)const { return m_currSolution[i]; }

    // Returns the solution normal at the ith grid point.
    const XMFLOAT3& Normal(int i)const { return m_normals[i]; }

    // Returns the unit tangent vector at the ith grid point in the local x-axis direction.
    const XMFLOAT3& TangentX(int i)const { return m_tangentX[i]; }

    void Update(float dt);
    void Disturb(int i, int j, float magnitude);

private:
    int m_numRows = 0;
    int m_numCols = 0;

    int m_vertexCount = 0;
    int m_triangleCount = 0;

    // Simulation constants we can precompute.
    float m_k1 = 0.0f;
    float m_k2 = 0.0f;
    float m_k3 = 0.0f;

    float m_timeStep = 0.0f;
    float m_spatialStep = 0.0f;

    std::vector<XMFLOAT3> m_prevSolution;
    std::vector<XMFLOAT3> m_currSolution;
    std::vector<XMFLOAT3> m_normals;
    std::vector<XMFLOAT3> m_tangentX;
};