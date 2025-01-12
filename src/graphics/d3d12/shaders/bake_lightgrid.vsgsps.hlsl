cbuffer rootConstant : register(b0)
{
    float4x4 WorldMatrix;
    float4 ObjectColor_ProbeIndex;
};

cbuffer cbPerObject : register(b1)
{
    float4x4 ViewProjMatrix;
    float4x4 CubeProjMatrix;
    float4x4 CubeFaceViewMatrix[6];
    float4 LightPosition_Null;
    float4 GridPos_ElementScale;
    float4 Width_Height_Depth_Distance;
};
struct VertexOut
{
    float4 oPos : SV_POSITION;
    float3 posW : POSITION;
    float3 normalW : NORMAL;
    float2 texCoord : TEXCOORD;
};
struct GeometryOut
{
    float4 oPos : SV_POSITION;
    float3 posW : POSITION;
    float3 normalW : NORMAL;
    float2 texCoord : TEXCOORD;
    uint RTIndex : SV_RenderTargetArrayIndex;
};

VertexOut VS(float3 iPos : POSITION,
        float3 iNormal : NORMAL,
        float2 iTexCoord : TEXCOORD)
{
    VertexOut vout;
    float3 posW = mul(float4(iPos, 1), WorldMatrix).xyz;
    float3 normalW = mul(float4(iNormal, 0), WorldMatrix).xyz;
    
    vout.posW = posW;
    vout.normalW = normalW;
    vout.texCoord = iTexCoord;
    
    vout.oPos = float4(0, 0, 0, 1);
    return vout;
}

[maxvertexcount(18)]
void GS(triangle VertexOut input[3], 
        inout TriangleStream<GeometryOut> LightGridStream) 
{
    const float3 gridPos = GridPos_ElementScale.xyz;
    const uint gridWidth = Width_Height_Depth_Distance.x;
    const uint gridHeight = Width_Height_Depth_Distance.y;
    const uint gridDepth = Width_Height_Depth_Distance.z;
    const float distance = Width_Height_Depth_Distance.w;
    const uint probeIndex = ObjectColor_ProbeIndex.w;
    const uint numFaces = 6;

    const int xIndex = probeIndex % gridWidth;
    const int zIndex = (probeIndex / gridWidth) % gridDepth;
    const int yIndex = probeIndex / (gridWidth * gridDepth);

    float4 probeOffset = float4(-(gridPos + float3(xIndex * distance, yIndex * distance, zIndex * distance)), 1);
    //float4 probeOffset = float4(1,1,1, 1);
    // probeOffset.x = -probeOffset.x;
    float4x4 probeTranslateMatrix = float4x4(float4(1, 0, 0, 0), float4(0, 1, 0, 0), float4(0,0,1,0), probeOffset);
    probeOffset.w = 1;
    for (uint face = 0; face < numFaces; ++face) {
        float4x4 translatedCubeFaceViewMatrix = (mul((probeTranslateMatrix), (CubeFaceViewMatrix[face])));
        GeometryOut gout;
        gout.RTIndex = face + probeIndex * numFaces;
        for (int v = 0; v < 3; ++v) {
            gout.posW = input[v].posW;
            gout.normalW = input[v].normalW;
            gout.texCoord = input[v].texCoord;
            gout.oPos = mul(float4(gout.posW, 1), (translatedCubeFaceViewMatrix));
            gout.oPos = mul(gout.oPos, transpose(CubeProjMatrix));
            LightGridStream.Append(gout);
        }
        LightGridStream.RestartStrip();
    }
}
float4 PS(GeometryOut gout) : SV_Target
{
    float3 color = ObjectColor_ProbeIndex.xyz;
    float3 lightDir = normalize(LightPosition_Null.xyz - gout.posW);
    float3 normal = normalize(gout.normalW);
    
    float4 output;
    output = float4(color * (max(dot(lightDir, normal), 0)), 1);
    // output = float4(1, 0, 0, 1);
    return output;
}