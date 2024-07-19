cbuffer cbPerObject : register(b0)
{
    float4x4 WorldMatrix;
    float4x4 ViewProjMatrix;
    float3 LightPosition;
};
struct Surface
{
    float3 posW : POSITION;
    float3 normalW : NORMAL;
    float3 colorW : COLOR;
};
void VS(float3 iPos : POSITION,
        float3 iNormal : NORMAL,
        float3 iColor : COLOR,
        out float4 oPos : SV_POSITION,
        out Surface oSurface)
{
    float3 posW = mul(float4(iPos, 1), WorldMatrix).xyz;
    float3 normalW = mul(float4(iNormal, 0), WorldMatrix).xyz;
    
    oSurface.posW = posW;
    oSurface.normalW = normalW;
    oSurface.colorW = iColor;
    
    oPos = mul(float4(posW, 1), ViewProjMatrix);
    oPos.y = -oPos.y;
}

float4 PS(float4 oPos : SV_POSITION, Surface oSurface) : SV_Target
{
    float3 color = oSurface.colorW;
    float3 lightDir = normalize(LightPosition - oSurface.posW);
    return float4(color * dot(lightDir, normalize(oSurface.normalW)), 1);
}