#include "MKInput.h"
#include "d3dUtil.h"
MKInput* MKInput::m_MKInput = nullptr;
LPDIRECTINPUT8 MKInput::m_DInput = nullptr;
LPDIRECTINPUTDEVICE8 MKInput::m_pKeyboard = nullptr;
LPDIRECTINPUTDEVICE8 MKInput::m_pMouse = nullptr;
HWND MKInput::m_hWnd = nullptr;
IDXGISwapChain*	MKInput::m_pSwapChain = nullptr;

void MKInput::Initialize(HWND hWnd, IDXGISwapChain*	pSwapChain)
{
	if (hWnd && !m_hWnd)
	{
		m_hWnd = hWnd;

		HR(DirectInput8Create(GetModuleHandle(nullptr), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_DInput, nullptr));

		//初始化键盘
		HR(m_DInput->CreateDevice(GUID_SysKeyboard, &m_pKeyboard, nullptr));
		HR(m_pKeyboard->SetDataFormat(&c_dfDIKeyboard));
		HR(m_pKeyboard->SetCooperativeLevel(m_hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND));
		m_pKeyboard->Acquire();

		//初始化鼠标
		HR(m_DInput->CreateDevice(GUID_SysMouse, &m_pMouse, nullptr));
		HR(m_pMouse->SetDataFormat(&c_dfDIMouse));
		HR(m_pMouse->SetCooperativeLevel(m_hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND));
		m_pMouse->Acquire();
	}
	if (pSwapChain && !m_pSwapChain)
		m_pSwapChain = pSwapChain;
	
}

void MKInput::Release()
{
	if (m_MKInput)
	{
		delete m_MKInput;
		if (m_pKeyboard)
		{
			HR(m_pKeyboard->Unacquire());
			ReleaseCOM(m_pKeyboard);
		}
		if (m_pMouse)
		{
			HR(m_pMouse->Unacquire());
			ReleaseCOM(m_pMouse);
		}
		ReleaseCOM(m_DInput);
	}
	
}

MKInput * MKInput::GetMKInput()
{
	if (!m_MKInput)
		m_MKInput = new MKInput;
	return m_MKInput;
}

void MKInput::Update()
{
	// 更新键盘
	m_pKeyboard->Poll();
	ZeroMemory(m_keys, 256);
	if (m_pKeyboard->GetDeviceState(256, (LPVOID)&m_keys) < 0)
	{
		// 键盘设备丢失，尝试重新获取
		m_pKeyboard->Acquire();
	}

	m_pMouse->Poll();
	ZeroMemory(&m_MouseState, sizeof(DIMOUSESTATE));
	if (m_pMouse->GetDeviceState(sizeof(DIMOUSESTATE), &m_MouseState) < 0)
	{
		//鼠标设备丢失，尝试重新获取
		m_pMouse->Acquire();
	}

	/*	
		更新鼠标位置和按键
		由于实际的视口是指窗口内部不含边框的部分，窗口的上边框高度为38，下左右边框宽度均为9
	如果做成无边框，则需要删去被减掉的边框宽度值。
	*/
	GetCursorPos(&m_MousePos);
	GetWindowRect(m_hWnd, &m_WndRect);
	BOOL fullscreen;
	m_pSwapChain->GetFullscreenState(&fullscreen, nullptr);
	if (!fullscreen)
	{
		m_MousePos.x -= (m_WndRect.left + 9);
		m_MousePos.y -= (m_WndRect.top + 38);
		m_MousePos.x = MathHelper::Clamp(m_MousePos.x, 0L, m_WndRect.right - m_WndRect.left - 18);
		m_MousePos.y = MathHelper::Clamp(m_MousePos.y, 0L, m_WndRect.bottom - m_WndRect.top - 47);
	}
	
	

	// 只有第一次点击有效
	if (m_SingleClickLeft && Mouse_LongClick(MKMouse::LEFT))
		m_SingleClickLeft = false;
	if (m_SingleClickRight && Mouse_LongClick(MKMouse::RIGHT))
		m_SingleClickRight = false;

	// 判断鼠标左键长按并更新鼠标左键按下的位置
	if ((m_MouseState.rgbButtons[0] & 0x80) && !Mouse_LongClick(MKMouse::LEFT))
	{
		m_OnClickLeftPos = m_MousePos;
		m_LongClickLeft = m_SingleClickLeft = true;
	}
	else if (!(m_MouseState.rgbButtons[0] & 0x80) && Mouse_LongClick(MKMouse::LEFT))
	{
		m_LongClickLeft = false;
		m_OnClickLeftPos = POINT{ -1, -1 };
	}
		

	// 判断鼠标右键长按并更新鼠标右键按下的位置
	if ((m_MouseState.rgbButtons[1] & 0x80) && !Mouse_LongClick(MKMouse::RIGHT))
	{
		m_OnClickRightPos = m_MousePos;
		m_LongClickRight = m_SingleClickRight = true;
	}
	else if (!(m_MouseState.rgbButtons[1] & 0x80) && Mouse_LongClick(MKMouse::RIGHT))
	{
		m_LongClickRight = false;
		m_OnClickRightPos = POINT{ -1, -1 };
	}
		
	
}

bool MKInput::Mouse_SingleClick(MKMouse mouseEvent) const
{
	if (mouseEvent == MKMouse::LEFT)
		return m_SingleClickLeft;
	else if (mouseEvent == MKMouse::RIGHT)
		return m_SingleClickRight;
	return false;
}

bool MKInput::Mouse_LongClick(MKMouse mouseEvent) const
{
	if (mouseEvent == MKMouse::LEFT)
		return m_LongClickLeft;
	else if (mouseEvent == MKMouse::RIGHT)
		return m_LongClickRight;
	return false;
}

POINT MKInput::GetMouseClickPos(MKMouse mouseEvent) const
{
	if (mouseEvent == MKMouse::LEFT)
		return m_OnClickLeftPos;
	else if (mouseEvent == MKMouse::RIGHT)
		return m_OnClickRightPos;
	return POINT();
}


