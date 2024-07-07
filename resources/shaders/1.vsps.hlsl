cbuffer cbPerObject : register(b0)
{
    float4x4 gWorldViewProj;
};
void VS(float2 iPos : POSITION,
        out float4 oPos : SV_POSITION)
{
    oPos = mul(float4(iPos, 0, 1), gWorldViewProj);
}

float4 PS(float4 oPos : SV_POSITION) : SV_Target
{
    return float4(0, 0.8, 0.3, 1);
}