#include "Sprite.h"
#include "XUtility.h"

ID3D11Device* SpriteManager::m_pDevice = nullptr;
ID3D11DeviceContext* SpriteManager::m_pDeviceContext = nullptr;
ID3DX11Effect* SpriteManager::m_pFX = nullptr;
IDXGISwapChain* SpriteManager::m_pSwapChain = nullptr;
HWND SpriteManager::m_hWnd = nullptr;


ID3DX11EffectTechnique* Sprite2D::m_pTech = nullptr;
ID3D11InputLayout* Sprite2D::m_pInputLayout = nullptr;
ID3D11Buffer* Sprite2D::m_pVertexBuffer = nullptr;
ID3D11Buffer* Sprite2D::m_pIndexBuffer = nullptr;
ID3D11Buffer* Sprite2D::m_pInstancedBuffer = nullptr;
ID3DX11EffectShaderResourceVariable* Sprite2D::m_pFXTexture = nullptr;	
ID3DX11EffectMatrixVariable* Sprite2D::m_pFXTexTransform = nullptr;	






SpriteManager::SpriteManager()
{
}

SpriteManager::~SpriteManager()
{
}

void SpriteManager::InitSprite(HWND hWnd, ID3D11DeviceContext * devContext, IDXGISwapChain* pSwapChain, ID3DX11Effect* pFX)
{
	if (!m_hWnd)
		m_hWnd = hWnd;
	if (!m_pDeviceContext)
	{
		m_pDeviceContext = devContext;
		m_pDeviceContext->GetDevice(&m_pDevice);
	}
	if (!m_pFX)
		m_pFX = pFX;
	if (!m_pSwapChain)
		m_pSwapChain = pSwapChain;
}




Sprite2D::Sprite2D()
	:  m_pTexture(nullptr), m_TexHeight(256), m_TexWidth(256)
{	
}

Sprite2D::~Sprite2D()
{
	ReleaseCOM(m_pTexture);
}

void Sprite2D::InitTexture(LPCSTR texFile)
{
	if (!m_pTexture)
	{
		// 获取图片信息
		GetPictureInfo(texFile, &m_TexWidth, &m_TexHeight);

		// 初始化纹理
		HR(D3DX11CreateShaderResourceViewFromFileA(GetDevice(),
			texFile, 0, 0, &m_pTexture, 0));

		// 初始化矩阵
		m_Instances.resize(1);
		XMStoreFloat4x4(&m_Instances[0], XMMatrixIdentity());
		XMStoreFloat4x4(&m_TexTransform, XMMatrixIdentity());
	}
}



void Sprite2D::Draw()
{
	if (m_Instances.empty())
		return;

	GetDeviceContext()->IASetInputLayout(m_pInputLayout);
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);	//图元类型

	UINT stride[2] = { sizeof(Vertex), sizeof(XMFLOAT4X4) };
	UINT offset[2] = { 0, 0 };
	ID3D11Buffer* vbs[2] = { m_pVertexBuffer, m_pInstancedBuffer };

	GetDeviceContext()->IASetVertexBuffers(0, 2, vbs, stride, offset);
	GetDeviceContext()->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	m_pFXTexture->SetResource(m_pTexture);
	m_pFXTexTransform->SetMatrix((const float *)&m_TexTransform);

	D3D11_MAPPED_SUBRESOURCE mappedData;
	GetDeviceContext()->Map(m_pInstancedBuffer, 0,
		D3D11_MAP_WRITE_DISCARD, 0, &mappedData);

	XMFLOAT4X4* dataView = reinterpret_cast<XMFLOAT4X4*>(mappedData.pData);
	memcpy(dataView, m_Instances.data(), sizeof(XMFLOAT4X4) * m_Instances.size());
	GetDeviceContext()->Unmap(m_pInstancedBuffer, 0);

	m_pTech->GetPassByIndex(0)->Apply(0, GetDeviceContext());
	GetDeviceContext()->DrawIndexedInstanced(4, m_Instances.size(), 0, 0, 0);
	
}

// 选取绘制区域和目标区域
void Sprite2D::StretchRect(const RECT* pSrcRect, const RECT* pDestRect, const POINT* pCenter, float radian)
{
	m_Instances.resize(1);
	XMStoreFloat4x4(&m_TexTransform, XMMatrixIdentity());
	if (pSrcRect)
	{
		// 算出源矩形的纹理坐标范围
		float texLeft = static_cast<float>(pSrcRect->left) / m_TexWidth;
		float texRight = static_cast<float>(pSrcRect->right) / m_TexWidth;
		float texTop = static_cast<float>(pSrcRect->top) / m_TexHeight;
		float texBottom = static_cast<float>(pSrcRect->bottom) / m_TexHeight;
		// 设置映射矩阵
		m_TexTransform(0, 0) = texRight - texLeft;
		m_TexTransform(1, 1) = texBottom - texTop;
		m_TexTransform(2, 0) = texLeft;
		m_TexTransform(2, 1) = texTop;
	}
	
	if (pDestRect)
	{
		// 目标矩形的宽高
		LONG destRectWidth = pDestRect->right - pDestRect->left;
		LONG destRectHeight = pDestRect->bottom - pDestRect->top;
		RECT wndRect;
		GetWindowRect(GetWindow(), &wndRect);
		// 视口宽高
		LONG vpWidth = wndRect.right - wndRect.left;
		LONG vpHeight = wndRect.bottom - wndRect.top;
		if (!IsFullScreen())
		{
			vpWidth -= 18;
			vpHeight -= 47;
		}
		// 目标矩形中心位置
		POINT center;
		if (pCenter)
			center = *pCenter;
		else
			center = { (pDestRect->right + pDestRect->left) / 2, (pDestRect->bottom + pDestRect->top) / 2 };
		// 缩放比例
		float scaleX = static_cast<float>(destRectWidth) / vpWidth;
		float scaleY = static_cast<float>(destRectHeight) / vpHeight;
		// 旋转矩阵
		XMMATRIX xmRotate;
		ZeroMemory(&xmRotate, sizeof(XMMATRIX));
		xmRotate(0, 0) = cos(radian);
		xmRotate(0, 1) = (float)vpWidth / vpHeight * sin(radian);
		xmRotate(1, 0) = -(float)vpHeight / vpWidth * sin(radian);
		xmRotate(1, 1) = cos(radian);
		xmRotate(2, 2) = 1.0f;
		xmRotate(3, 3) = 1.0f;
		// 平移偏移量
		float offsetX = static_cast<float>(center.x - vpWidth / 2) / vpWidth * 2;
		float offsetY = -(static_cast<float>(center.y - vpHeight / 2) / vpHeight * 2);

		XMStoreFloat4x4(&m_Instances[0], XMMatrixScaling(scaleX, scaleY, 1.0f) *
			xmRotate * XMMatrixTranslation(offsetX, offsetY, 0.0f));
	}
	else
		XMStoreFloat4x4(&m_Instances[0], XMMatrixIdentity());
}

void Sprite2D::StretchRects(const std::vector<RECT>& DestRects,const std::vector<float>& radians)
{
	XMStoreFloat4x4(&m_TexTransform, XMMatrixIdentity());
	int sz = DestRects.size();
	m_Instances.resize(sz);
	for (int i = 0; i < sz; ++i)
	{
		// 目标矩形的宽高
		LONG destRectWidth = DestRects[i].right - DestRects[i].left;
		LONG destRectHeight = DestRects[i].bottom - DestRects[i].top;
		RECT wndRect;
		GetWindowRect(GetWindow(), &wndRect);
		// 视口宽高
		LONG vpWidth = wndRect.right - wndRect.left;
		LONG vpHeight = wndRect.bottom - wndRect.top;
		if (!IsFullScreen())
		{
			vpWidth -= 18;
			vpHeight -= 47;
		}
		// 目标矩形中心位置
		POINT center = { (DestRects[i].right + DestRects[i].left) / 2,
			(DestRects[i].bottom + DestRects[i].top) / 2 };
		// 缩放比例
		float scaleX = static_cast<float>(destRectWidth) / vpWidth;
		float scaleY = static_cast<float>(destRectHeight) / vpHeight;
		// 旋转矩阵
		XMMATRIX xmRotate;
		ZeroMemory(&xmRotate, sizeof(XMMATRIX));
		xmRotate(0, 0) = cos(radians[i]);
		xmRotate(0, 1) = (float)vpWidth / vpHeight * sin(radians[i]);
		xmRotate(1, 0) = -(float)vpHeight / vpWidth * sin(radians[i]);
		xmRotate(1, 1) = cos(radians[i]);
		xmRotate(2, 2) = 1.0f;
		xmRotate(3, 3) = 1.0f;
		// 平移偏移量
		float offsetX = static_cast<float>(center.x - vpWidth / 2) / vpWidth * 2;
		float offsetY = -(static_cast<float>(center.y - vpHeight / 2) / vpHeight * 2);

		XMStoreFloat4x4(&m_Instances[i], XMMatrixScaling(scaleX, scaleY, 1.0f) *
			xmRotate * XMMatrixTranslation(offsetX, offsetY, 0.0f));
	}
}




// 初始化2D
void Sprite2D::Initialize(HWND hWnd, ID3D11DeviceContext* devContext, IDXGISwapChain* pSwapChain, ID3DX11Effect* pFX, LPCSTR techName)
{
	InitSprite(hWnd, devContext, pSwapChain, pFX);
	if (!m_pTech)
	{
		m_pTech = GetEffect()->GetTechniqueByName(techName);
		// 初始化Effects变量
		m_pFXTexture = GetEffect()->GetVariableByName("gTexture")->AsShaderResource();
		m_pFXTexTransform = GetEffect()->GetVariableByName("gTexTransform")->AsMatrix();
	}
	if (!m_pInputLayout)
	{
		D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
		};
		D3DX11_PASS_DESC passDesc;
		m_pTech->GetPassByIndex(0)->GetDesc(&passDesc);
		HR(GetDevice()->CreateInputLayout(inputLayoutDesc, 6, passDesc.pIAInputSignature,
			passDesc.IAInputSignatureSize, &m_pInputLayout));
	}

	// 顶点数组初始化
	std::vector<Vertex>	Vertexes{
		{ XMFLOAT3(-1.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) }
	};

	// 初始化顶点描述
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex) * Vertexes.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	// 创建顶点缓冲区
	D3D11_SUBRESOURCE_DATA vInitData;
	vInitData.pSysMem = Vertexes.data();
	HR(GetDevice()->CreateBuffer(&vbd, &vInitData, &m_pVertexBuffer));

	std::vector<DWORD> Indices{ 0,1,2,3 };


	// 初始化索引描述
	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * Indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;

	// 初始化顶点
	D3D11_SUBRESOURCE_DATA iInitData;
	iInitData.pSysMem = Indices.data();
	HR(GetDevice()->CreateBuffer(&ibd, &iInitData, &m_pIndexBuffer));

	// 初始化实例描述
	D3D11_BUFFER_DESC inbd;
	inbd.Usage = D3D11_USAGE_DYNAMIC;
	inbd.ByteWidth = sizeof(XMFLOAT4X4) * 500;
	inbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	inbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	inbd.MiscFlags = 0;
	inbd.StructureByteStride = 0;

	HR(GetDevice()->CreateBuffer(&inbd, nullptr, &m_pInstancedBuffer));
}
// 释放
void Sprite2D::Release()
{
	ReleaseCOM(m_pInputLayout);
	ReleaseCOM(m_pInstancedBuffer);
	ReleaseCOM(m_pIndexBuffer);
	ReleaseCOM(m_pVertexBuffer);
	ReleaseStatic(SpriteManager);
}

Sprite2DBackGround::~Sprite2DBackGround()
{
}
