//***************************************************************************************
// d3dApp.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "d3dApp.h"
#include <WindowsX.h>
#include <sstream>

namespace
{
	// This is just used to forward Windows messages from a global window
	// procedure to our member function window procedure because we cannot
	// assign a member function to WNDCLASS::lpfnWndProc.
	D3DApp* gd3dApp = 0;
}

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Forward hwnd on because we can get messages (e.g., WM_CREATE)
	// before CreateWindow returns, and thus before m_hMainWnd is valid.
	return gd3dApp->MsgProc(hwnd, msg, wParam, lParam);
}

D3DApp::D3DApp(HINSTANCE hInstance)
:	m_hAppInst(hInstance),
	m_MainWndCaption(L"D3D11 Application"),
	m_d3dDriverType(D3D_DRIVER_TYPE_HARDWARE),
	m_ClientWidth(750 + 18),
	m_ClientHeight(900 + 47),
	m_Enable4xMsaa(false),
	m_hMainWnd(0),
	m_AppPaused(false),
	m_Minimized(false),
	m_Maximized(false),
	m_Resizing(false),
	m_4xMsaaQuality(0),
	m_FullScreen(false),

	m_pD3DDevice(0),
	m_pD3DImmediateContext(0),
	m_pSwapChain(0),
	m_pDepthStencilBuffer(0),
	m_pRenderTargetView(0),
	m_pDepthStencilView(0)
{
	ZeroMemory(&m_ScreenViewport, sizeof(D3D11_VIEWPORT));

	// Get a pointer to the application object so we can forward 
	// Windows messages to the object's window procedure through
	// the global window procedure.
	gd3dApp = this;
}

D3DApp::~D3DApp()
{
	ReleaseCOM(m_pRenderTargetView);
	ReleaseCOM(m_pDepthStencilView);
	ReleaseCOM(m_pSwapChain);
	ReleaseCOM(m_pDepthStencilBuffer);

	// Restore all default settings.
	if( m_pD3DImmediateContext )
		m_pD3DImmediateContext->ClearState();

	ReleaseCOM(m_pD3DImmediateContext);

	// 此处用于内存泄漏检测
	 ReportLeakMemory();
	
	ReleaseCOM(m_pD3DDevice);
}

HINSTANCE D3DApp::AppInst()const
{
	return m_hAppInst;
}

HWND D3DApp::MainWnd()const
{
	return m_hMainWnd;
}

float D3DApp::AspectRatio()const
{
	return static_cast<float>(m_ClientWidth) / m_ClientHeight;
}

int D3DApp::Run()
{
	MSG msg = {0};
 
	m_Timer.Reset();

	while(msg.message != WM_QUIT)
	{
		// 有窗口消息则先处理它们
		if(PeekMessage( &msg, 0, 0, 0, PM_REMOVE ))
		{
            TranslateMessage( &msg );
            DispatchMessage( &msg );
		}
		// 否则，进行游戏部分
		else
        {	
			m_Timer.Tick();

			if( !m_AppPaused )
			{		
				// 限帧到60fps
				static float currTime = m_Timer.TotalTime();
				if (m_Timer.TotalTime() - currTime > 1.0f / 60)
				{
					CalculateFrameStats();
					currTime = m_Timer.TotalTime();
					UpdateScene(m_Timer.DeltaTime());
					DrawScene();
				}
				
			}
			else
			{
				Sleep(100);
			}
        }
    }

	return (int)msg.wParam;
}

bool D3DApp::Init()
{
	if(!InitMainWindow())
		return false;

	if(!InitDirect3D())
		return false;

	return true;
}
 
void D3DApp::OnResize()
{
	assert(m_pD3DImmediateContext);
	assert(m_pD3DDevice);
	assert(m_pSwapChain);


	// Release the old views, as they hold references to the buffers we
	// will be destroying.  Also release the old depth/stencil buffer.

	ReleaseCOM(m_pRenderTargetView);		// 释放渲染目标视图
	ReleaseCOM(m_pDepthStencilView);		// 释放深度/模板视图
	ReleaseCOM(m_pDepthStencilBuffer);	// 释放深度/模板缓冲区

	// 重设交换链并且重新创建渲染目标视图

	HR(m_pSwapChain->ResizeBuffers(1, m_ClientWidth, m_ClientHeight, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE));
	
	HR(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pBackBuffer)));
	HR(m_pD3DDevice->CreateRenderTargetView(m_pBackBuffer, 0, &m_pRenderTargetView));
	ReleaseCOM(m_pBackBuffer);


	// 创建深度/目标缓冲和视图

	D3D11_TEXTURE2D_DESC depthStencilDesc;
	
	depthStencilDesc.Width     = m_ClientWidth;
	depthStencilDesc.Height    = m_ClientHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format    = DXGI_FORMAT_D24_UNORM_S8_UINT;

	// Use 4X MSAA? --需要给交换链设置MASS参数
	if( m_Enable4xMsaa )
	{
		depthStencilDesc.SampleDesc.Count   = 4;
		depthStencilDesc.SampleDesc.Quality = m_4xMsaaQuality-1;
	}
	// No MSAA
	else
	{
		depthStencilDesc.SampleDesc.Count   = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
	}

	depthStencilDesc.Usage          = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags      = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0; 
	depthStencilDesc.MiscFlags      = 0;

	HR(m_pD3DDevice->CreateTexture2D(&depthStencilDesc, 0, &m_pDepthStencilBuffer));
	HR(m_pD3DDevice->CreateDepthStencilView(m_pDepthStencilBuffer, 0, &m_pDepthStencilView));


	// 将渲染目标视图和深度/模板缓冲区结合到管线

	m_pD3DImmediateContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);
	

	// 设置视口变换
	m_ScreenViewport.TopLeftX = 0;
	m_ScreenViewport.TopLeftY = 0;
	m_ScreenViewport.Width = static_cast<float>(m_ClientWidth);
	m_ScreenViewport.Height = static_cast<float>(m_ClientHeight);
	m_ScreenViewport.MinDepth = 0.0f;
	m_ScreenViewport.MaxDepth = 1.0f;

	

	m_pD3DImmediateContext->RSSetViewports(1, &m_ScreenViewport);
}
 
LRESULT D3DApp::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch( msg )
	{
	// WM_ACTIVATE is sent when the window is activated or deactivated.  
	// We pause the game when the window is deactivated and unpause it 
	// when it becomes active.  
	case WM_ACTIVATE:
		if( LOWORD(wParam) == WA_INACTIVE )
		{
			m_AppPaused = true;
			m_Timer.Stop();
		}
		else
		{
			m_AppPaused = false;
			m_Timer.Start();
		}
		return 0;

	// WM_SIZE is sent when the user resizes the window.  
	case WM_SIZE:
		// Save the new client area dimensions.
		m_ClientWidth  = LOWORD(lParam);
		m_ClientHeight = HIWORD(lParam);
		if( m_pD3DDevice )
		{
			if( wParam == SIZE_MINIMIZED )
			{
				m_AppPaused = true;
				m_Minimized = true;
				m_Maximized = false;
			}
			else if( wParam == SIZE_MAXIMIZED )
			{
				m_AppPaused = false;
				m_Minimized = false;
				m_Maximized = true;
				OnResize();
			}
			else if( wParam == SIZE_RESTORED )
			{
				
				// Restoring from minimized state?
				if( m_Minimized )
				{
					m_AppPaused = false;
					m_Minimized = false;
					OnResize();
				}

				// Restoring from maximized state?
				else if( m_Maximized )
				{
					m_AppPaused = false;
					m_Maximized = false;
					OnResize();
				}
				else if( m_Resizing )
				{
					// If user is dragging the resize bars, we do not resize 
					// the buffers here because as the user continuously 
					// drags the resize bars, a stream of WM_SIZE messages are
					// sent to the window, and it would be pointless (and slow)
					// to resize for each WM_SIZE message received from dragging
					// the resize bars.  So instead, we reset after the user is 
					// done resizing the window and releases the resize bars, which 
					// sends a WM_EXITSIZEMOVE message.
				}
				else // API call such as SetWindowPos or m_pSwapChain->SetFullscreenState.
				{
					OnResize();
				}
			}
		}
		return 0;

	// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
	case WM_ENTERSIZEMOVE:
		m_AppPaused = true;
		m_Resizing  = true;
		m_Timer.Stop();
		return 0;

	// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
	// Here we reset everything based on the new window dimensions.
	case WM_EXITSIZEMOVE:
		m_AppPaused = false;
		m_Resizing  = false;
		m_Timer.Start();
		OnResize();
		return 0;
 
	// WM_DESTROY is sent when the window is being destroyed.
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	// The WM_MENUCHAR message is sent when a menu is active and the user presses 
	// a key that does not correspond to any mnemonic or accelerator key. 
	case WM_MENUCHAR:
        // Don't beep when we alt-enter.
        return MAKELRESULT(0, MNC_CLOSE);
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void D3DApp::ReportLeakMemory()
{
#if defined(DEBUG) || defined(_DEBUG)  
	ID3D11Debug *d3dDebug;
	HRESULT hr = m_pD3DDevice->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&d3dDebug));
	if (SUCCEEDED(hr))
	{
		hr = d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
	}
	ReleaseCOM(d3dDebug);
#endif
}

bool D3DApp::InitMainWindow()
{
	WNDCLASS wc;
	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = MainWndProc; 
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = m_hAppInst;
	wc.hIcon         = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor       = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName  = 0;
	wc.lpszClassName = L"D3DWndClassName";

	if( !RegisterClass(&wc) )
	{
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
		return false;
	}

	// Compute window rectangle dimensions based on requested client area dimensions.
	RECT R = { 0, 0, m_ClientWidth, m_ClientHeight };
    AdjustWindowRect(&R, WS_OVERLAPPED | WS_SYSMENU, false);
	int width  = R.right - R.left;
	int height = R.bottom - R.top;

	m_hMainWnd = CreateWindow(L"D3DWndClassName", m_MainWndCaption.c_str(), 
		WS_OVERLAPPED | WS_SYSMENU, 600, 50, width, height, 0, 0, m_hAppInst, 0);
	if( !m_hMainWnd )
	{
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return false;
	}

	ShowWindow(m_hMainWnd, SW_SHOW);
	UpdateWindow(m_hMainWnd);

	return true;
}

bool D3DApp::InitDirect3D()
{
	// 创建D3D设备 和 D3D设备上下文

	UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0
	};
	D3D_FEATURE_LEVEL featureLevel;

	HRESULT hr = D3D11CreateDevice(
			0,                 // 默认适配器
			m_d3dDriverType,
			0,                 // 无软件设备
			createDeviceFlags | D3D11_CREATE_DEVICE_BGRA_SUPPORT, 
			featureLevels, ARRAYSIZE(featureLevels),
			D3D11_SDK_VERSION,
			&m_pD3DDevice,
			&featureLevel,
			&m_pD3DImmediateContext);

	if( FAILED(hr) )
	{
		MessageBox(0, L"D3D11CreateDevice Failed.", 0, 0);
		return false;
	}

	// 填充DXGI_SWAP_CHAIN_DESC用以描述交换链

	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width  = m_ClientWidth;
	sd.BufferDesc.Height = m_ClientHeight;
	sd.BufferDesc.RefreshRate.Numerator = 60;				// 分子
	sd.BufferDesc.RefreshRate.Denominator = 1;				// 分母
	sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;		// (r, g, b, a)
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	// Use 4X MSAA? 
	if( m_Enable4xMsaa )
	{
		sd.SampleDesc.Count   = 4;
		sd.SampleDesc.Quality = m_4xMsaaQuality-1;
	}
	// No MSAA
	else
	{
		sd.SampleDesc.Count   = 1;
		sd.SampleDesc.Quality = 0;
	}

	sd.BufferUsage  = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount  = 1;
	sd.OutputWindow = m_hMainWnd;
	sd.Windowed     = true;
	sd.SwapEffect   = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags        = 0;

	// To correctly create the swap chain, we must use the IDXGIFactory that was
	// used to create the device.  If we tried to use a different IDXGIFactory instance
	// (by calling CreateDXGIFactory), we get an error: "IDXGIFactory::CreateSwapChain: 
	// This function is being called with a device from a different IDXGIFactory."

	IDXGIDevice* dxgiDevice = 0;
	HR(m_pD3DDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice));
	      
	IDXGIAdapter* dxgiAdapter = 0;
	HR(dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter));

	IDXGIFactory* dxgiFactory = 0;
	HR(dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory));

	HR(dxgiFactory->CreateSwapChain(m_pD3DDevice, &sd, &m_pSwapChain));


	ReleaseCOM(dxgiDevice);
	ReleaseCOM(dxgiAdapter);
	ReleaseCOM(dxgiFactory);

	// The remaining steps that need to be carried out for d3d creation
	// also need to be executed every time the window is resized.  So
	// just call the OnResize method here to avoid code duplication.
	
	OnResize();

	

	return true;
}

void D3DApp::CalculateFrameStats()
{
	// 该代码计算每秒帧速，并计算每一帧渲染需要的时间，显示在窗口标题

	static int frameCnt = 0;
	static float timeElapsed = 0.0f;

	frameCnt++;

	// Compute averages over one second period.
	if( (m_Timer.TotalTime() - timeElapsed) >= 1.0f )
	{
		float fps = (float)frameCnt; // fps = frameCnt / 1
		float mspf = 1000.0f / fps;

		std::wostringstream outs;   
		outs.precision(6);
		outs << m_MainWndCaption << L"    "
			 << L"FPS: " << fps << L"    " 
			 << L"Frame Time: " << mspf << L" (ms)";
		SetWindowText(m_hMainWnd, outs.str().c_str());
		
		// Reset for next average.
		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}


