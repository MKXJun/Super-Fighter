#pragma once

#include "d3dUtil.h"

// 2D������Ϣ
struct Vertex {
	XMFLOAT3 Pos;
	XMFLOAT2 Tex;
};


// Sprite�����ߣ�����ȫ�ֳ�ʼ��
class SpriteManager
{
public:
	SpriteManager();
	virtual ~SpriteManager();

protected:
	static ID3D11Device* GetDevice() { return m_pDevice; }
	static ID3D11DeviceContext* GetDeviceContext() { return m_pDeviceContext; }
	static ID3DX11Effect* GetEffect() { return m_pFX; }
	static IDXGISwapChain* GetSwapChain() { return m_pSwapChain; }
	static HWND GetWindow() { return m_hWnd; }
	static void InitSprite(HWND hWnd, ID3D11DeviceContext* devContext, IDXGISwapChain* pSwapChain, ID3DX11Effect* pFX);
	static void Release() { m_pDevice = nullptr; m_pDeviceContext = nullptr; }
	static bool IsFullScreen() { BOOL fullscreen; m_pSwapChain->GetFullscreenState(&fullscreen, nullptr); return fullscreen; }
private:
	static ID3D11Device*					m_pDevice;			// D3D�豸
	static ID3D11DeviceContext*				m_pDeviceContext;	// D3D�豸������
	static IDXGISwapChain*					m_pSwapChain;		// ������
	static ID3DX11Effect*					m_pFX;				// Effects(���ļ�)
	static HWND								m_hWnd;				// ���ھ��
};

// ����2D
class Sprite2D : public SpriteManager
{	

public:
	Sprite2D();
	~Sprite2D() override;
	// ��ʼ������
	void InitTexture(LPCSTR texFile);
	
	// ���л��ƣ���ʹ��StretchRect��ʱ��ֻ����һ��
	void Draw();
	/*
	 * ������Ҫ���Ƶ���ͼ����(������)�͵������ϵľ�������
	 * .png��.bmp���������ͨ��GetTextureϵ�к�����ȡ�����Ŀ��
	 * �����������͵�����Ĭ�Ͽ�߶���Ϊ256
	 * ����radian���Ծ�����ת���ȡ�
	 * ��pSrcRectΪ�գ�����������ͼ��
	 * ��pDestRectΪ�գ���������Ϸ��Ļ������ͼ��
	 * ��pCenterΪ�գ�ʹ��Ĭ����������Ľ�����ת��
	 */
	void StretchRect(const RECT* pSrcRect, const RECT* pDestRect, const POINT* pCenter, float radian);
	// ��ͬһ��������ƶ����ͬ��Ŀ��
	void StretchRects(const std::vector<RECT>& DestRects, const std::vector<float>& radians);
	// ��ȡ������
	LONG GetTexWidth() const { return m_TexWidth; }
	// ��ȡ����߶�
	LONG GetTexHeight() const { return m_TexHeight; }

	// ��ʼ��Sprite2D��Ҫ���
	static void Initialize(HWND hWnd, ID3D11DeviceContext* devContext, IDXGISwapChain* pSwapChain, ID3DX11Effect* pFX, LPCSTR techName);

	// �ͷ�Sprite2D��Դ
	static void Release();

private:
	static ID3D11Buffer*						m_pVertexBuffer;			// ���㻺����
	static ID3D11Buffer*						m_pIndexBuffer;				// ����������	
	static ID3D11Buffer*						m_pInstancedBuffer;			// ʵ��������

	static ID3DX11EffectShaderResourceVariable*	m_pFXTexture;				// Effects�������
	static ID3DX11EffectMatrixVariable*			m_pFXTexTransform;			// Effects���������任����



	static ID3D11InputLayout*					m_pInputLayout;				// ���벼��
	static ID3DX11EffectTechnique*				m_pTech;					// Effect����

	std::vector<XMFLOAT4X4>						m_Instances;				// ʵ������
	XMFLOAT4X4									m_TexTransform;				// ����任

	ID3D11ShaderResourceView*					m_pTexture;					// ����
	LONG										m_TexWidth, m_TexHeight;	// �����ȡ��߶�
};

class Sprite2DBackGround : SpriteManager
{
private:
	Sprite2DBackGround();
	~Sprite2DBackGround() override;

	ID3D11Buffer*						m_pVertexBuffer;			// ���㻺����
	ID3D11Buffer*						m_pIndexBuffer;				// ����������	


};