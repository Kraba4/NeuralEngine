cbuffer rootConstant : register(b0)
{
    float4x4 WorldMatrix;
    float4 ObjectColor;
};
sampler Sampler : register(s0);

TextureCubeArray<float4> cubeMapArray : register(t2);

cbuffer cbPerObject : register(b1)
{
    float4x4 ViewProjMatrix;
    float4x4 CubeProjMatrix;
    float4x4 CubeFaceViewMatrix[6];
    float4 LightPosition_Null;
    float4 GridPos_ElementScale;
    float4 Width_Height_Depth_Distance;
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
    
    float3 posW = mul(float4(iPos, 1), WorldMatrix).xyz;
    float3 normalW = mul(float4(iNormal, 0), WorldMatrix).xyz;
    
    oSurface.posW = posW;
    oSurface.normalW = normalW;
    oSurface.texCoord = iTexCoord;
    
    oPos = mul(float4(posW, 1), ViewProjMatrix);
}

uint toIndex(float3 pos, float gridWidth, float gridDepth) {
    return pos.x + pos.z * gridWidth + pos.y * (gridWidth * gridDepth);
}
float4 PS(float4 oPos : SV_POSITION, Surface oSurface) : SV_Target
{
    const float3 gridPos = GridPos_ElementScale.xyz;
    const int gridWidth = Width_Height_Depth_Distance.x;
    const int gridHeight = Width_Height_Depth_Distance.y;
    const int gridDepth = Width_Height_Depth_Distance.z;
    const float distance = Width_Height_Depth_Distance.w;

    float3 normal = normalize(oSurface.normalW);
    float3 reverseNormal = -normal;
    // float3 normal = float3(0, -1, 0);
    float3 indexPos = (oSurface.posW - gridPos) / distance;
    float3 posW = oSurface.posW - gridPos;
    float3 ambient;
    if (indexPos.x < 0 || indexPos.x >= gridWidth || 
        indexPos.z < 0 || indexPos.z >= gridDepth ||
        indexPos.y < 0 || indexPos.y > gridHeight) {
        ambient = 0;
    } else {
        float3 minIndexPos = float3((int)indexPos.x, (int)indexPos.y, (int)indexPos.z);

        float3 posProb000 = (minIndexPos + float3(0, 0, 0));
        float3 posProb100 = (minIndexPos + float3(1, 0, 0));
        float3 posProb010 = (minIndexPos + float3(0, 1, 0));
        float3 posProb110 = (minIndexPos + float3(1, 1, 0));
        float3 posProb001 = (minIndexPos + float3(0, 0, 1));
        float3 posProb101 = (minIndexPos + float3(1, 0, 1));
        float3 posProb011 = (minIndexPos + float3(0, 1, 1));
        float3 posProb111 = (minIndexPos + float3(1, 1, 1));
        
        float3 dir000 = normalize(posProb000 * distance - posW);
        float3 dir100 = normalize(posProb100 * distance - posW);
        float3 dir010 = normalize(posProb010 * distance - posW);
        float3 dir110 = normalize(posProb110 * distance - posW);
        float3 dir001 = normalize(posProb001 * distance - posW);
        float3 dir101 = normalize(posProb101 * distance - posW);
        float3 dir011 = normalize(posProb011 * distance - posW);
        float3 dir111 = normalize(posProb111 * distance - posW);

        float3 p000 = cubeMapArray.Sample(Sampler, float4(-dir000, toIndex(posProb000, gridWidth, gridDepth))).xyz;
        float3 p100 = cubeMapArray.Sample(Sampler, float4(-dir100, toIndex(posProb100, gridWidth, gridDepth))).xyz;
        float3 p010 = cubeMapArray.Sample(Sampler, float4(-dir010, toIndex(posProb010, gridWidth, gridDepth))).xyz;
        float3 p110 = cubeMapArray.Sample(Sampler, float4(-dir110, toIndex(posProb110, gridWidth, gridDepth))).xyz;
        float3 p001 = cubeMapArray.Sample(Sampler, float4(-dir001, toIndex(posProb001, gridWidth, gridDepth))).xyz;
        float3 p101 = cubeMapArray.Sample(Sampler, float4(-dir101, toIndex(posProb101, gridWidth, gridDepth))).xyz;
        float3 p011 = cubeMapArray.Sample(Sampler, float4(-dir011, toIndex(posProb011, gridWidth, gridDepth))).xyz;
        float3 p111 = cubeMapArray.Sample(Sampler, float4(-dir111, toIndex(posProb111, gridWidth, gridDepth))).xyz;

        float3 xInterp00 = lerp(p000, p100, frac(indexPos.x));
        float3 xInterp10 = lerp(p010, p110, frac(indexPos.x));
        float3 xInterp01 = lerp(p001, p101, frac(indexPos.x));
        float3 xInterp11 = lerp(p011, p111, frac(indexPos.x));

        float3 yInterp0 = lerp(xInterp00, xInterp10, frac(indexPos.y));
        float3 yInterp1 = lerp(xInterp01, xInterp11, frac(indexPos.y));

        ambient = lerp(yInterp0, yInterp1, frac(indexPos.z));
    }
    // float3 ambient = p000;

    float3 color = ObjectColor.xyz;
    float3 lightDir = normalize(LightPosition_Null.xyz - oSurface.posW);
    
    float4 output;
    // dot(lightDir, normal) +
    output = float4(color * ambient, 1) * 0.8 + float4(color * (max(dot(lightDir, normal), 0) * 0.6), 1);

    return output;
}