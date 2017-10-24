

#define TECH(name) technique11 name


cbuffer cbPerObject
{
    float4x4 gWorld2D;           // 对裁剪空间坐标点的变换矩阵
    float4x4 gTexTransform;      // 对纹理坐标点的世界变换矩阵
};
Texture2D gTexture;

SamplerState samAnisotropic
{
    Filter = ANISOTROPIC;
    MaxAnisotropy = 4;

    AddressU = WRAP;
    AddressV = WRAP;
};


struct Vertex2DIn
{
    float2 PosH : POSITION;
    float2 Tex : TEXCOORD;
    row_major float4x4 World2D : WORLD;
    uint InstanceId : SV_InstanceID;
};

struct Vertex2DOut
{
    float4 PosH : SV_POSITION;
    float2 Tex : TEXCOORD;
};