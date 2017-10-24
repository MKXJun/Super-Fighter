#include "XUtility.h"
#include "PlaneManager.h"

void GetPictureInfo(LPCSTR fileName, LONG * pOutWidth, LONG * pOutHeight)
{
	//获取位图尺寸
	FILE* fp = fopen(fileName, "rb");
	if (!fp)
	{
		MessageBoxA(0, (std::string(fileName) + "打开失败!").c_str(), "错误", MB_OK);
		assert(fp != NULL);
	}

	unsigned char buf[4];
	unsigned char BMPHeader[] = { 0x42,0x4d,0xf6,0xb8,0x59 };				//BMP头文件
	unsigned char PNGHeader[] = {0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A};  //PNG头文件

	unsigned seek_pos = 0;
	fread_s(buf, 4, sizeof(char), 4, fp);
	if (!memcmp(buf, PNGHeader, 4))
	{

		fseek(fp, 16, SEEK_SET);
		fread_s(buf, 4, sizeof(char), 4, fp);
		*pOutWidth = BYTE_TO_UINT(buf[0], buf[1], buf[2], buf[3]);
		fread_s(buf, 4, sizeof(char), 4, fp);
		*pOutHeight = BYTE_TO_UINT(buf[0], buf[1], buf[2], buf[3]);
	}
	else if (!memcmp(buf, BMPHeader, 4))
	{
		fseek(fp, 18, SEEK_SET);
		fread_s(buf, 4, sizeof(char), 4, fp);
		*pOutWidth = BYTE_TO_UINT(buf[3], buf[2], buf[1], buf[0]);
		fread_s(buf, 4, sizeof(char), 4, fp);
		*pOutHeight = BYTE_TO_UINT(buf[3], buf[2], buf[1], buf[0]);
	}

	fclose(fp);
}




void BuildFX(ID3D11Device* dev, LPCWSTR fileName, ID3DX11Effect** ppEffect)
{
	DWORD shaderFlags = 0;
#if defined( DEBUG ) || defined( _DEBUG )
	shaderFlags |= D3D10_SHADER_DEBUG;
	shaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif

	ID3D10Blob* compiledShader = 0;
	ID3D10Blob* compilationMsgs = 0;

	HRESULT hr = D3DX11CompileFromFile(fileName, 0, 0, 0, "fx_5_0", shaderFlags,
		0, 0, &compiledShader, &compilationMsgs, 0);

	// compilationMsgs可以获取警告或者错误信息
	if (compilationMsgs != 0)
	{
		MessageBoxA(0, (char*)compilationMsgs->GetBufferPointer(), 0, 0);
		ReleaseCOM(compilationMsgs);
	}

	// 如果没有编译信息，确保这里没有错误
	if (FAILED(hr))
	{
		DXTrace(__FILE__, (DWORD)__LINE__, hr, L"D3DX11CompileFromFile", true);
	}

	HR(D3DX11CreateEffectFromMemory(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(),
		0, dev, ppEffect));

	// 编译着色器完成，可以释放了
	ReleaseCOM(compiledShader);
}

bool RectCollision(const RECT& r1, const RECT& r2)
{
	// 矩形r1宽高的一半
	float halfWidth1 = (r1.right - r1.left) / 2.0f;
	float halfHeight1 = (r1.bottom - r1.top) / 2.0f;
	// 矩形r2宽高的一半
	float halfWidth2 = (r2.right - r2.left) / 2.0f;
	float halfHeight2 = (r2.bottom - r2.top) / 2.0f;
	// 中心r1坐标
	float centerX1 = (r1.right + r1.left) / 2.0f;
	float centerY1 = (r1.bottom + r1.top) / 2.0f;
	// 中心r2坐标
	float centerX2 = (r2.right + r2.left) / 2.0f;
	float centerY2 = (r2.bottom + r2.top) / 2.0f;

	// 判断两点XY间距是否分别小于等于两个矩形宽高的一半。
	return (fabs(centerX1 - centerX2) <= halfWidth1 + halfWidth2) &&
		(fabs(centerY1 - centerY2) <= halfHeight1 + halfHeight2);
}

float DegToRad(float degree)
{
	return degree * MathHelper::Pi / 180.0f;
}

bool ClickBox(const MKInput* pIn, const D2D1_RECT_F& rect, D2DText& d2dText, LPCWSTR wstr)
{
	POINT point = pIn->GetMousePos();
	// 判断鼠标是否在按钮内
	if (rect.left <= (float)point.x && rect.right >= (float)point.x &&
		rect.top <= (float)point.y && rect.bottom >= (float)point.y)
	{
		d2dText.SetTextColor(D2D1::ColorF::Gray);
		d2dText.DrawString(wstr, rect);
		d2dText.SetTextColor(D2D1::ColorF::White);
	}
	else
		d2dText.DrawString(wstr, rect);
	
	

	// 判断鼠标是否点击了按钮
	point = pIn->GetMouseClickPos(MKMouse::LEFT);
	if (rect.left <= (float)point.x && rect.right >= (float)point.x &&
		rect.top <= (float)point.y && rect.bottom >= (float)point.y &&
		pIn->Mouse_SingleClick(MKMouse::LEFT))
		return true;
	else
		return false;
}

LONG LevelToVolume(int level)
{
	if (level <= 0)
		return -10000;
	return (LONG)(2000 * log10f(level / 10.0f));
}

