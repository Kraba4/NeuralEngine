#pragma once

enum MeshNames {
    M_Cat,
    M_Bird,
    M_Plane,
    M_Wall,
    M_Sphere,
    M_SIZE
};
enum GraphicsPipelinesNames {
    GP_Basic,
    GP_Final,
    GP_BakeLightGrid,
    GP_DrawLightGrid,
    GP_CalcIrradiance,
    GP_SIZE
};
enum RootSignaturesNames {
    RS_Basic,
    RS_SIZE
};
enum FrameTexturesNames {
    FT_Main,
    FT_MainDepth,
    FT_GBufferColor,
    FT_GBufferNormal,
    FT_GBufferToCamera,
    FT_SIZE
};
enum UniqueTexturesNames {
    UT_LightGridCubemaps,
    UT_LightGridDepthCubemaps,
    UT_LightGridIrradianceCubemaps,
    UT_SIZE
};
enum FrameConstantBuffersNames {
    FCB_ProjViewLight,
    FCB_SIZE
};
enum UniqueBuffersNames {
    UB_Vertex,
    UB_Index,
    UB_VertexUpload,
    UB_IndexUpload,
    UB_LightGridCubemapsReadback,
    UB_SIZE
};