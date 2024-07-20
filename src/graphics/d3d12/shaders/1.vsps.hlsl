cbuffer rootConstant : register(b0)
{
    int objectId;
};

cbuffer cbPerObject : register(b1)
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
    
    float3 posW = objectId == 0 ? mul(float4(iPos, 1), WorldMatrix).xyz : float4(iPos, 1);
    float3 normalW = objectId == 0 ? mul(float4(iNormal, 0), WorldMatrix).xyz : float4(iNormal, 0);
    
    oSurface.posW = posW;
    oSurface.normalW = normalW;
    oSurface.colorW = iColor;
    
    oPos = mul(float4(posW, 1), ViewProjMatrix);
    //oPos.w = -oPos.w;
    //oPos.y = -oPos.y;
}

float4 PS(float4 oPos : SV_POSITION, Surface oSurface) : SV_Target
{
    float3 color = oSurface.colorW;
    float3 lightDir = normalize(LightPosition - oSurface.posW);
    return float4(color * (max(dot(lightDir, normalize(oSurface.normalW)), 0)), 1);
}