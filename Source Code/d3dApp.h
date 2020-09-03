//***************************************************************************************
// d3dApp.h by Frank Luna (C) 2011 All Rights Reserved.
//
// Simple Direct3D demo application class.  
// Make sure you link: d3d11.lib d3dx11d.lib D3DCompiler.lib D3DX11EffectsD.lib 
//                     dxerr.lib dxgi.lib dxguid.lib.
// Link d3dx11.lib and D3DX11Effects.lib for release mode builds instead
//   of d3dx11d.lib and D3DX11EffectsD.lib.
//***************************************************************************************

#ifndef D3DAPP_H
#define D3DAPP_H

#include <string>
#include "d2dText.h"
#include "d3dUtil.h"
#include "GameTimer.h"

#if defined(_DEBUG) | defined(DEBUG)
#pragma comment(lib, "d3dx11d.lib")
#else
#pragma comment(lib, "d3dx11.lib")
#endif

#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")
#pragma comment(lib, "Effects11.lib")
#pragma comment(lib, "dxerr.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "legacy_stdio_definitions.lib")

class D3DApp
{
public:
	D3DApp(HINSTANCE hInstance);
	virtual ~D3DApp();
	
	HINSTANCE AppInst()const;
	HWND      MainWnd()const;
	float     AspectRatio()const;
	
	int Run();
 
	// 框架方法。派生类应该重载这些方法来实现特定的应用需求

	virtual bool Init();
	virtual void OnResize(); 
	virtual void UpdateScene(float dt)=0;
	virtual void DrawScene()=0; 
	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	// 必要时可以调用以检测D3D设备的内存泄漏
	void ReportLeakMemory();

protected:
	bool InitMainWindow();
	bool InitDirect3D();

	void CalculateFrameStats();

protected:

	HINSTANCE m_hAppInst;
	HWND      m_hMainWnd;
	bool      m_AppPaused;
	bool      m_Minimized;
	bool      m_Maximized;
	bool      m_Resizing;
	UINT      m_4xMsaaQuality;
	BOOL	  m_FullScreen;			// 是否全屏
	GameTimer m_Timer;


	// Direct3D
	ID3D11Device*					m_pD3DDevice;				// D3D设备
	ID3D11DeviceContext*			m_pD3DImmediateContext;		// D3D设备上下文
	IDXGISwapChain*					m_pSwapChain;				// 交换链
	ID3D11Texture2D*				m_pBackBuffer;				// 后备缓冲区
	ID3D11Texture2D*				m_pDepthStencilBuffer;		// 深度/模板缓冲区
	ID3D11RenderTargetView*			m_pRenderTargetView;		// 渲染目标视图
	ID3D11DepthStencilView*			m_pDepthStencilView;		// 深度/模板视图
	D3D11_VIEWPORT					m_ScreenViewport;			// 屏幕视口
	
						
	

	// 派生类应该在构造函数内设置好这些自定义的初值
	std::wstring			m_MainWndCaption;
	D3D_DRIVER_TYPE			m_d3dDriverType;
	int						m_ClientWidth;
	int						m_ClientHeight;
	bool					m_Enable4xMsaa;
};

#endif // D3DAPP_H