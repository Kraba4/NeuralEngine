cbuffer rootConstant : register(b0)
{
    float4x4 WorldMatrix;
    float4 ObjectColor_ProbeIndex;
};

sampler Sampler : register(s0);

TextureCubeArray<float4> cubeMapArray : register(t2);

cbuffer cbPerObject : register(b1)
{
    float4x4 ViewProjMatrix;
    float4x4 CubeProjMatrix;
    float4x4 CubeFaceViewMatrix[6];
    float4 LightPosition_CubeFaceSize;
    float4 GridPos_ElementScale;
    float4 Width_Height_Depth_Distance;
};
struct GeometryOut
{
    float4 oPos : SV_POSITION;
    float2 texCoord : TEXCOORD0;
    uint face : TEXCOORD1;
    uint RTIndex : SV_RenderTargetArrayIndex;
};

struct VertexOut {
    float4 oPos : SV_POSITION;
    float2 texCoord : TEXCOORD0;
};
VertexOut VS(uint vertexId : SV_VertexID)
{
    VertexOut vout;
    float2 xy = vertexId == 0 ? float2(-1, -1) : (vertexId == 1 ? float2(3, -1) : float2(-1, 3));
    vout.oPos = float4(xy*float2(1, -1), 0, 1);
    vout.texCoord = xy * 0.5 + 0.5;
    return vout;
}

[maxvertexcount(18)]
void GS(triangle VertexOut input[3], 
        inout TriangleStream<GeometryOut> LightGridStream) 
{
    const uint probeIndex = ObjectColor_ProbeIndex.w;
    const uint numFaces = 6;

    for (uint face = 0; face < numFaces; ++face) {
        GeometryOut gout;
        gout.RTIndex = face + probeIndex * numFaces;
         gout.face = face;
        for (int v = 0; v < 3; ++v) {
            gout.texCoord = input[v].texCoord;
            gout.oPos = input[v].oPos;
            LightGridStream.Append(gout);
        }
        LightGridStream.RestartStrip();
    }
}

static const uint numFaces = 6;
static const float3 uDir[numFaces] =    {float3(0, 0, 1), float3(0, 0, -1), float3(-1, 0, 0), float3(-1, 0, 0), float3(-1, 0, 0), float3(1, 0, 0)};
static const float3 vDir[numFaces] =    {float3(0, 1, 0), float3(0, 1, 0), float3(0, 0, -1), float3(0, 0, 1), float3(0, 1, 0), float3(0, 1, 0)};
static const float3 faceDir[numFaces] = {float3(1, 0, 0), float3(-1, 0, 0), float3(0, 1, 0), float3(0, -1, 0), float3(0, 0, 1), float3(0, 0, -1)};

float3 getDir(uint face, float i, float j, float cubeFaceSize) {
    i = (i / cubeFaceSize) * 2.0f - 1.0f + (0.5f / cubeFaceSize);
    j = (j / cubeFaceSize) * 2.0f - 1.0f + (0.5f / cubeFaceSize);
    const float3 dir = j * uDir[face] + i * vDir[face] + faceDir[face];
    return dir;
}
float4 PS(GeometryOut gout) : SV_Target
{
    const uint cubeFaceSize = LightPosition_CubeFaceSize.w;
    uint probeIndex = ObjectColor_ProbeIndex.w;
    gout.texCoord.x = (1.0 - gout.texCoord.x);
    gout.texCoord.y = (1.0 - gout.texCoord.y);
    float3 normalDir = getDir(gout.face, gout.texCoord.y * cubeFaceSize, gout.texCoord.x * cubeFaceSize, cubeFaceSize);
    normalDir = normalize(normalDir);

    float3 sumColor = float3(0, 0, 0);
    for (uint face = 0; face < numFaces; ++face) {
        for (uint i = 0; i < cubeFaceSize; ++i) {
            for (uint j = 0; j < cubeFaceSize; ++j) {
                float3 texelDir = getDir(face, i, j, cubeFaceSize);
                float3 texelColor = cubeMapArray.Sample(Sampler, float4(texelDir, probeIndex)).xyz;
                sumColor += texelColor * max(0, dot(normalize(texelDir), normalDir));
            }
        }
    }
    const float PiMultiply4 = 12.56637;
    float4 output = float4(PiMultiply4 * sumColor / float(numFaces * cubeFaceSize * cubeFaceSize), 1);
    return output;
}