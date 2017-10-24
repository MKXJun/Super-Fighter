#pragma once
#define DIRECTINPUT_VERSION 0x0800

#include <d3d11.h>
#include <dinput.h>
#pragma comment(lib, "dinput8.lib")

enum class MKMouse { 
	LEFT,	//鼠标右键
	RIGHT	//鼠标左键
};

class MKInput {
public:

	// 初始化键盘和鼠标
	static void Initialize(HWND hWnd, IDXGISwapChain* pSwapChain);
	// 释放键盘和鼠标
	static void Release();
	// 获取键鼠对象的指针
	static MKInput* GetMKInput();
	// 更新键鼠事件
	void Update();

	// 鼠标单击
	bool Mouse_SingleClick(MKMouse mouseEvent) const;
	// 检测鼠标按键是否按住
	bool Mouse_LongClick(MKMouse mouseEvent) const;
	// 输出鼠标按键坐标
	POINT GetMouseClickPos(MKMouse mouseEvent) const;
	// 输出鼠标坐标
	POINT GetMousePos() const { return m_MousePos; }
	// 获取鼠标相对坐标位移XY
	POINT GetMouseLXY() const 
	{ 
		return POINT{ m_MouseState.lX, m_MouseState.lY }; 
	}
	// 获取鼠标滚轮量（上方向为正）
	int GetMouseLRoll() const { return m_MouseState.lZ / 120; }
	// 检测键盘按键
	bool Key_Down(int key) const { return (bool)(m_keys[key] & 0x80); }

private:
	MKInput() {}
	~MKInput() {}

	static MKInput*					m_MKInput;
	static LPDIRECTINPUT8			m_DInput;
	static HWND						m_hWnd;
	static LPDIRECTINPUTDEVICE8		m_pKeyboard;
	static LPDIRECTINPUTDEVICE8		m_pMouse;
	static IDXGISwapChain*			m_pSwapChain;


	char							m_keys[256];

	RECT							m_WndRect;
	DIMOUSESTATE					m_MouseState;
	POINT							m_MousePos;
	POINT							m_OnClickLeftPos;
	POINT							m_OnClickRightPos;
	bool							m_SingleClickLeft;
	bool							m_SingleClickRight;
	bool							m_LongClickLeft;
	bool							m_LongClickRight;

};