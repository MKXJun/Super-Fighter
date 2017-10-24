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

// ��ʹ��D2DText����֮ǰ�����ṩ���������г�ʼ��
HRESULT D2DText::Initialize(HWND hWnd, IDXGISwapChain* pDXGISwapChain)
{
	if (!m_hWnd)
		m_hWnd = hWnd;
	if (!m_pSwapChain)
		m_pSwapChain = pDXGISwapChain;
	if (!m_pD2DFactory)
	{
		// ����D2D1����
		D2D1_FACTORY_OPTIONS options;
		options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
		HR(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, 
			options, &m_pD2DFactory));
		// ���ﴴ����ȾĿ��
		Discard();
		OnReset();
	}
	if (!m_pDWriteFactory)
	{
		// ����DWrite����
		HR(DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(IDWriteFactory),
			reinterpret_cast<IUnknown**>(&m_pDWriteFactory)));
	}

	return S_OK;
}


// ���������ı�����
HRESULT D2DText::Reset(LPCWSTR fontName, FLOAT fontSize, D2D1::ColorF color, 
	DWRITE_TEXT_ALIGNMENT textAlign, DWRITE_PARAGRAPH_ALIGNMENT paraAlign)
{
	assert(m_pSwapChain);
	ReleaseCOM(m_pD2DColorBrush);
	ReleaseCOM(m_pTextFormat);
	// ����Ĭ�ϵ��ı���ʽ
	HR(m_pDWriteFactory->CreateTextFormat(fontName, 
		nullptr, 
		DWRITE_FONT_WEIGHT_REGULAR,
		DWRITE_FONT_STYLE_NORMAL, 
		DWRITE_FONT_STRETCH_NORMAL, 
		fontSize, L"zh-cn", 
		&m_pTextFormat));
	// �����ı�ˮƽ����
	HR(m_pTextFormat->SetTextAlignment(textAlign));
	// �����ı��������
	HR(m_pTextFormat->SetParagraphAlignment(paraAlign));
	// ������ɫˢ
	HR(m_pD2DRenderTarget->CreateSolidColorBrush(D2D1::ColorF(color), 
		&m_pD2DColorBrush));
	return S_OK;
}

// �����ı��������ͺʹ�С
HRESULT D2DText::SetFont(LPCWSTR fontName, FLOAT fontSize)
{
	// ȡ��֮ǰ���õĸ�ʽ��Ȼ���ͷ�
	DWRITE_TEXT_ALIGNMENT textAlign = m_pTextFormat->GetTextAlignment();
	DWRITE_PARAGRAPH_ALIGNMENT ParaAlign = m_pTextFormat->GetParagraphAlignment();
	ReleaseCOM(m_pTextFormat);

	// ����Ĭ�ϵ��ı���ʽ
	HR(m_pDWriteFactory->CreateTextFormat(fontName, 
		nullptr, 
		DWRITE_FONT_WEIGHT_REGULAR,
		DWRITE_FONT_STYLE_NORMAL, 
		DWRITE_FONT_STRETCH_NORMAL, 
		fontSize, 
		L"zh-cn", 
		&m_pTextFormat));
	// �����ı�ˮƽ����
	HR(m_pTextFormat->SetTextAlignment(textAlign));
	// �����ı��������
	HR(m_pTextFormat->SetParagraphAlignment(ParaAlign));

	return S_OK;
}

// �����ı���
HRESULT D2DText::SetTextRect(D2D1_RECT_F drawRect)
{
	m_D2DRect = drawRect;
	return S_OK;
}

// �����ı�ˮƽ����
HRESULT D2DText::SetTextAlign(DWRITE_TEXT_ALIGNMENT textAlign)
{
	HR(m_pTextFormat->SetTextAlignment(textAlign));
	return S_OK;
}

// �����ı��������
HRESULT D2DText::SetParaAlign(DWRITE_PARAGRAPH_ALIGNMENT ParaAlign)
{
	HR(m_pTextFormat->SetParagraphAlignment(ParaAlign));
	return S_OK;
}

// �����ı���ɫ
HRESULT D2DText::SetTextColor(D2D1::ColorF color)
{
	ReleaseCOM(m_pD2DColorBrush);
	HR(m_pD2DRenderTarget->CreateSolidColorBrush(D2D1::ColorF(color), &m_pD2DColorBrush));
	return S_OK;
}

// �������غ���= = Ȼ������дĬ�ϲ����ġ���֮Google�淶
// ֱ�ӻ����ı�
HRESULT D2DText::DrawString(LPCWSTR wString)
{
	return DrawString(wString, m_D2DRect);
}

// ���ݸ���������������ı�
HRESULT D2DText::DrawString(LPCWSTR wString,const D2D1_RECT_F& drawRect)
{
	return DrawString(wString, drawRect, 0.0f);
}

// ���ݸ����������򣬲�����������ת�Ƕ��������ı�
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





// ��RenderTargetView���ͷŵ�ʱ�������øú���
void D2DText::Discard()
{
	ReleaseCOM(m_pD2DRenderTarget);
}

// �ڹ����RenderTargetView�������øú���
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

