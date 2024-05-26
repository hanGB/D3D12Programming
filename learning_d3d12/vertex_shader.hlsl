float4 main(uint nVertexID : SV_VertexID) : SV_Position
{
    float4 output;
    
    if (nVertexID == 0)         output = float4(0.0, 0.5, 0.5, 1.0);
    else if (nVertexID == 1)    output = float4(0.5, -0.5, 0.5, 1.0);
    else if (nVertexID == 2)    output = float4(-0.5, -0.5, 0.5, 1.0);
    
    return output;
}
