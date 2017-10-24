//=============================================================================
// Lighting.fx by Frank Luna (C) 2011 All Rights Reserved.
//
// Transforms and lights geometry.
//=============================================================================

#include "Basic.hlsli"

Vertex2DOut VS_2D(Vertex2DIn vin)
{
	// 2D��ͼ�����Ϊ0.0f����������ǰ������任���2D������
    Vertex2DOut vout;
    vout.PosH = mul(float4(vin.PosH, 0.0f, 1.0f), vin.World2D);
    vout.Tex = mul(float4(vin.Tex, 1.0f, 1.0f), gTexTransform).xy;
    return vout;
}

float4 PS_2D(Vertex2DOut pin) : SV_Target
{
    float4 texColor = gTexture.Sample(samAnisotropic, pin.Tex);

    // ����ͼƬ��alphaֵ�ʵ����вü�
    clip(texColor.a - 0.1f);

    return texColor;
}

TECH(Sprite2D)
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS_2D()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_2D()));
    }
}
