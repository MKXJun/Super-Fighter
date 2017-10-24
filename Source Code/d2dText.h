#ifndef _D2DTEXT_H_
#define _D2DTEXT_H_

#include <intsafe.h>
#include <D2d1.h>
#include <D2D1Helper.h>
#include <DWrite.h>
#include <dxerr.h>
#include <cassert>


class D2DText 
{
public:
	D2DText();
	D2DText::D2DText(LPCWSTR fontName, FLOAT fontSize);
	D2DText(LPCWSTR fontName, FLOAT fontSize, D2D1::ColorF color,
		DWRITE_TEXT_ALIGNMENT textAlign, DWRITE_PARAGRAPH_ALIGNMENT paraAlign);
	~D2DText();

	HRESULT Reset(LPCWSTR fontName, FLOAT fontSize, D2D1::ColorF color,
		DWRITE_TEXT_ALIGNMENT textAlign, DWRITE_PARAGRAPH_ALIGNMENT paraAlign);
	HRESULT SetFont(LPCWSTR fontName, FLOAT fontSize);
	HRESULT SetTextRect(D2D1_RECT_F drawRect);
	HRESULT SetTextAlign(DWRITE_TEXT_ALIGNMENT textAlign);
	HRESULT SetParaAlign(DWRITE_PARAGRAPH_ALIGNMENT ParaAlign);
	HRESULT SetTextColor(D2D1::ColorF color);
	
	HRESULT DrawString(LPCWSTR wString);
	HRESULT DrawString(LPCWSTR wString, const D2D1_RECT_F& drawRect);
	HRESULT DrawString(LPCWSTR wString, const D2D1_RECT_F& drawRect, FLOAT rotateAngle);

	static HRESULT Initialize(HWND hWnd, IDXGISwapChain* pDXGISwapChain);	
	static void Discard();
	static void OnReset();
	static void Release();
private:
	

	ID2D1SolidColorBrush*				m_pD2DColorBrush;
	IDWriteTextFormat*					m_pTextFormat;
	D2D1_RECT_F							m_D2DRect;

	static HWND							m_hWnd;
	static ID2D1Factory*				m_pD2DFactory;
	static IDWriteFactory*				m_pDWriteFactory;
	static ID2D1RenderTarget*			m_pD2DRenderTarget;
	static IDXGISwapChain*				m_pSwapChain;
};
#endif
