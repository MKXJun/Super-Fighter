#pragma once

#include "d3dUtil.h"

// 2D顶点信息
struct Vertex {
	XMFLOAT3 Pos;
	XMFLOAT2 Tex;
};


// Sprite管理者，负责全局初始化
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
	static ID3D11Device*					m_pDevice;			// D3D设备
	static ID3D11DeviceContext*				m_pDeviceContext;	// D3D设备上下文
	static IDXGISwapChain*					m_pSwapChain;		// 交换链
	static ID3DX11Effect*					m_pFX;				// Effects(单文件)
	static HWND								m_hWnd;				// 窗口句柄
};

// 精灵2D
class Sprite2D : public SpriteManager
{	

public:
	Sprite2D();
	~Sprite2D() override;
	// 初始化纹理
	void InitTexture(LPCSTR texFile);
	
	// 进行绘制，当使用StretchRect的时候只绘制一个
	void Draw();
	/*
	 * 设置需要绘制的贴图区域(按像素)和到窗口上的矩形区域。
	 * .png和.bmp的纹理可以通过GetTexture系列函数获取正常的宽高
	 * 对于其余类型的纹理默认宽高都将为256
	 * 设置radian可以决定旋转弧度。
	 * 若pSrcRect为空，绘制完整贴图。
	 * 若pDestRect为空，在整个游戏屏幕绘制贴图。
	 * 若pCenter为空，使用默认纹理的中心进行旋转。
	 */
	void StretchRect(const RECT* pSrcRect, const RECT* pDestRect, const POINT* pCenter, float radian);
	// 对同一种纹理绘制多个不同的目标
	void StretchRects(const std::vector<RECT>& DestRects, const std::vector<float>& radians);
	// 获取纹理宽度
	LONG GetTexWidth() const { return m_TexWidth; }
	// 获取纹理高度
	LONG GetTexHeight() const { return m_TexHeight; }

	// 初始化Sprite2D必要组件
	static void Initialize(HWND hWnd, ID3D11DeviceContext* devContext, IDXGISwapChain* pSwapChain, ID3DX11Effect* pFX, LPCSTR techName);

	// 释放Sprite2D资源
	static void Release();

private:
	static ID3D11Buffer*						m_pVertexBuffer;			// 顶点缓冲区
	static ID3D11Buffer*						m_pIndexBuffer;				// 索引缓冲区	
	static ID3D11Buffer*						m_pInstancedBuffer;			// 实例缓冲区

	static ID3DX11EffectShaderResourceVariable*	m_pFXTexture;				// Effects纹理变量
	static ID3DX11EffectMatrixVariable*			m_pFXTexTransform;			// Effects纹理坐标点变换矩阵



	static ID3D11InputLayout*					m_pInputLayout;				// 输入布局
	static ID3DX11EffectTechnique*				m_pTech;					// Effect技术

	std::vector<XMFLOAT4X4>						m_Instances;				// 实例数据
	XMFLOAT4X4									m_TexTransform;				// 纹理变换

	ID3D11ShaderResourceView*					m_pTexture;					// 纹理
	LONG										m_TexWidth, m_TexHeight;	// 纹理宽度、高度
};

class Sprite2DBackGround : SpriteManager
{
private:
	Sprite2DBackGround();
	~Sprite2DBackGround() override;

	ID3D11Buffer*						m_pVertexBuffer;			// 顶点缓冲区
	ID3D11Buffer*						m_pIndexBuffer;				// 索引缓冲区	


};