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
    float3 fromCenterRay : TEXCOORD0;
    float2 texCoord : TEXCOORD1;
    float pixelInstanceID : TEXCOORD2;
};
void VS(float3 iPos : POSITION,
        float3 iNormal : NORMAL,
        float2 iTexCoord : TEXCOORD,
        uint InstanceId : SV_InstanceID,
        out float4 oPos : SV_POSITION,
        out Surface oSurface)
{
    const float elementScale = GridPos_ElementScale.w;
    const float3 gridPos = GridPos_ElementScale.xyz;
    const int gridWidth = Width_Height_Depth_Distance.x;
    const int gridDepth = Width_Height_Depth_Distance.z;
    const float distance = Width_Height_Depth_Distance.w;

    float3 posW    = (float4(iPos, 1) * elementScale).xyz;
    float3 probeCenter = gridPos;

    const int xIndex = InstanceId % gridWidth;
    const int zIndex = (InstanceId / gridWidth) % gridDepth;
    const int yIndex = InstanceId / (gridWidth * gridDepth);

    probeCenter.x += xIndex * distance;
    probeCenter.z += zIndex * distance;
    probeCenter.y += yIndex * distance;
    posW += probeCenter;

    oSurface.posW = posW;
    oSurface.fromCenterRay = iPos;
    // oSurface.normalW = iNormal.xyz;
    oSurface.texCoord = iTexCoord;
    
    // oPos = mul(float4(posW, 1), ViewProjMatrix);
    oPos = mul(float4(posW, 1), ViewProjMatrix);
    //oPos.w = -oPos.w;
    //oPos.y = -oPos.y;
    oSurface.pixelInstanceID = InstanceId + 0.2;
}

float4 PS(float4 oPos : SV_POSITION, Surface oSurface) : SV_Target
{
    uint pixelInstanceId = oSurface.pixelInstanceID;
    float3 color = cubeMapArray.Sample(Sampler, float4(oSurface.fromCenterRay, pixelInstanceId)).xyz;   
    float4 output = float4(color, 1);
    return output;
}