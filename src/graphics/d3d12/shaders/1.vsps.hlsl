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
    float2 texCoord : TEXCOORD;
};
void VS(float3 iPos : POSITION,
        float3 iNormal : NORMAL,
        float2 iTexCoord : TEXCOORD,
        out float4 oPos : SV_POSITION,
        out Surface oSurface)
{
    
    float3 posW = objectId == 0 ? mul(float4(iPos, 1), WorldMatrix).xyz : iPos;
    float3 normalW = objectId == 0 ? mul(float4(iNormal, 0), WorldMatrix).xyz : iNormal;
    
    oSurface.posW = posW;
    oSurface.normalW = normalW;
    oSurface.texCoord = iTexCoord;
    
    oPos = mul(float4(posW, 1), ViewProjMatrix);
    //oPos.w = -oPos.w;
    //oPos.y = -oPos.y;
}

struct PS_OUTPUT
{
    float4 Screen: SV_Target0;
    float4 Color: SV_Target1;
    float4 Normal: SV_Target2;
    float4 ToCamera: SV_Target3;
};

PS_OUTPUT PS(float4 oPos : SV_POSITION, Surface oSurface)
{
    float3 color = 1;
    float3 lightDir = normalize(LightPosition - oSurface.posW);
    float3 normal = normalize(oSurface.normalW);
    
    PS_OUTPUT output;
    output.Screen = float4(color * (max(dot(lightDir, normal), 0)) + 0.2f, 1);
    output.Color = float4(color * (max(dot(lightDir, normal), 0)) + 0.2f, 1);
    output.Normal = float4(normal, 1);
    output.ToCamera = float4(lightDir, 1);

    return output;
    // return float4(lightDir, 1);
}