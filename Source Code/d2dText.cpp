#include "d2dText.h"

//---------------------------------------------------------------------------------------
// Simple d3d error checker for book demos.
//---------------------------------------------------------------------------------------
#if defined(DEBUG) | defined(_DEBUG)
#ifndef HR
#define HR(x)                                              \
	{                                                          \
		HRESULT hr = (x);                                      \
		if(FAILED(hr))                                         \
		{                                                      \
			DXTrace(__FILE__, (DWORD)__LINE__, hr, L#x, true); \
		}                                                      \
	}
#endif

#else
#ifndef HR
#define HR(x) (x)
#endif
#endif 

//---------------------------------------------------------------------------------------
// Convenience macro for releasing COM objects.
//---------------------------------------------------------------------------------------

#define ReleaseCOM(x) { if(x){ x->Release(); x = 0; } }


ID2D1Factory*			D2DText::m_pD2DFactory = nullptr;
IDWriteFactory*			D2DText::m_pDWriteFactory = nullptr;
ID2D1RenderTarget*		D2DText::m_pD2DRenderTarget = nullptr;
IDXGISwapChain*			D2DText::m_pSwapChain = nullptr;
HWND					D2DText::m_hWnd = nullptr;

D2DText::D2DText()
	: m_pD2DColorBrush(nullptr), m_D2DRect(), m_pTextFormat(nullptr)
{
}


D2DText::D2DText(LPCWSTR fontName, FLOAT fontSize, D2D1::ColorF color,
	DWRITE_TEXT_ALIGNMENT textAlign, DWRITE_PARAGRAPH_ALIGNMENT paraAlign)
{
	Reset(fontName, fontSize, color, textAlign, paraAlign);
}


D2DText::D2DText(LPCWSTR fontName, FLOAT fontSize)
{
	Reset(fontName, fontSize, D2D1::ColorF::Black, 
		DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
}


D2DText::~D2DText()
{
	ReleaseCOM(m_pD2DColorBrush);
	ReleaseCOM(m_pTextFormat);
}

// 在使用D2DText对象之前必须提供交换链进行初始化
HRESULT D2DText::Initialize(HWND hWnd, IDXGISwapChain* pDXGISwapChain)
{
	if (!m_hWnd)
		m_hWnd = hWnd;
	if (!m_pSwapChain)
		m_pSwapChain = pDXGISwapChain;
	if (!m_pD2DFactory)
	{
		// 创建D2D1工厂
		D2D1_FACTORY_OPTIONS options;
		options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
		HR(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, 
			options, &m_pD2DFactory));
		// 这里创建渲染目标
		Discard();
		OnReset();
	}
	if (!m_pDWriteFactory)
	{
		// 创建DWrite工厂
		HR(DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(IDWriteFactory),
			reinterpret_cast<IUnknown**>(&m_pDWriteFactory)));
	}

	return S_OK;
}


// 重新设置文本参数
HRESULT D2DText::Reset(LPCWSTR fontName, FLOAT fontSize, D2D1::ColorF color, 
	DWRITE_TEXT_ALIGNMENT textAlign, DWRITE_PARAGRAPH_ALIGNMENT paraAlign)
{
	assert(m_pSwapChain);
	ReleaseCOM(m_pD2DColorBrush);
	ReleaseCOM(m_pTextFormat);
	// 设置默认的文本格式
	HR(m_pDWriteFactory->CreateTextFormat(fontName, 
		nullptr, 
		DWRITE_FONT_WEIGHT_REGULAR,
		DWRITE_FONT_STYLE_NORMAL, 
		DWRITE_FONT_STRETCH_NORMAL, 
		fontSize, L"zh-cn", 
		&m_pTextFormat));
	// 设置文本水平对齐
	HR(m_pTextFormat->SetTextAlignment(textAlign));
	// 设置文本段落对齐
	HR(m_pTextFormat->SetParagraphAlignment(paraAlign));
	// 设置颜色刷
	HR(m_pD2DRenderTarget->CreateSolidColorBrush(D2D1::ColorF(color), 
		&m_pD2DColorBrush));
	return S_OK;
}

// 设置文本字体类型和大小
HRESULT D2DText::SetFont(LPCWSTR fontName, FLOAT fontSize)
{
	// 取出之前设置的格式，然后释放
	DWRITE_TEXT_ALIGNMENT textAlign = m_pTextFormat->GetTextAlignment();
	DWRITE_PARAGRAPH_ALIGNMENT ParaAlign = m_pTextFormat->GetParagraphAlignment();
	ReleaseCOM(m_pTextFormat);

	// 设置默认的文本格式
	HR(m_pDWriteFactory->CreateTextFormat(fontName, 
		nullptr, 
		DWRITE_FONT_WEIGHT_REGULAR,
		DWRITE_FONT_STYLE_NORMAL, 
		DWRITE_FONT_STRETCH_NORMAL, 
		fontSize, 
		L"zh-cn", 
		&m_pTextFormat));
	// 设置文本水平对齐
	HR(m_pTextFormat->SetTextAlignment(textAlign));
	// 设置文本段落对齐
	HR(m_pTextFormat->SetParagraphAlignment(ParaAlign));

	return S_OK;
}

// 设置文本框
HRESULT D2DText::SetTextRect(D2D1_RECT_F drawRect)
{
	m_D2DRect = drawRect;
	return S_OK;
}

// 设置文本水平对齐
HRESULT D2DText::SetTextAlign(DWRITE_TEXT_ALIGNMENT textAlign)
{
	HR(m_pTextFormat->SetTextAlignment(textAlign));
	return S_OK;
}

// 设置文本段落对齐
HRESULT D2DText::SetParaAlign(DWRITE_PARAGRAPH_ALIGNMENT ParaAlign)
{
	HR(m_pTextFormat->SetParagraphAlignment(ParaAlign));
	return S_OK;
}

// 设置文本颜色
HRESULT D2DText::SetTextColor(D2D1::ColorF color)
{
	ReleaseCOM(m_pD2DColorBrush);
	HR(m_pD2DRenderTarget->CreateSolidColorBrush(D2D1::ColorF(color), &m_pD2DColorBrush));
	return S_OK;
}

// 三个重载函数= = 然而我想写默认参数的。迷之Google规范
// 直接绘制文本
HRESULT D2DText::DrawString(LPCWSTR wString)
{
	return DrawString(wString, m_D2DRect);
}

// 根据给定矩形区域绘制文本
HRESULT D2DText::DrawString(LPCWSTR wString,const D2D1_RECT_F& drawRect)
{
	return DrawString(wString, drawRect, 0.0f);
}

// 根据给定矩形区域，并设置字体旋转角度来绘制文本
HRESULT D2DText::DrawString(
	LPCWSTR wString, const D2D1_RECT_F& drawRect, FLOAT rotateAngle)
{
	m_pD2DRenderTarget->BeginDraw();

	D2D1_POINT_2F center = D2D1::Point2F((drawRect.right - drawRect.left) / 2, 
		(drawRect.bottom - drawRect.top) / 2);
	m_pD2DRenderTarget->SetTransform(D2D1::Matrix3x2F::Rotation(rotateAngle, center));
	m_pD2DRenderTarget->DrawText(wString, 
		wcslen(wString), 
		m_pTextFormat, 
		drawRect, 
		m_pD2DColorBrush);

	HR(m_pD2DRenderTarget->EndDraw());
	return S_OK;
}





// 在RenderTargetView被释放的时候必须调用该函数
void D2DText::Discard()
{
	ReleaseCOM(m_pD2DRenderTarget);
}

// 在构造好RenderTargetView后必须调用该函数
void D2DText::OnReset()
{
	if (m_pSwapChain)
	{

		IDXGISurface1 *m_pBackBuffer = nullptr;
		HR(m_pSwapChain->GetBuffer(0, __uuidof(IDXGISurface1), reinterpret_cast<void**>(&m_pBackBuffer)));
		D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT,
			D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED));
		HR(m_pD2DFactory->CreateDxgiSurfaceRenderTarget(m_pBackBuffer, props, &m_pD2DRenderTarget));
		ReleaseCOM(m_pBackBuffer);
	}
	
}

void D2DText::Release()
{
	ReleaseCOM(m_pD2DRenderTarget);
	ReleaseCOM(m_pD2DFactory);
	ReleaseCOM(m_pDWriteFactory);
}

