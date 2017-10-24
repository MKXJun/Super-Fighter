#pragma once
#define DIRECTINPUT_VERSION 0x0800

#include <d3d11.h>
#include <dinput.h>
#pragma comment(lib, "dinput8.lib")

enum class MKMouse { 
	LEFT,	//����Ҽ�
	RIGHT	//������
};

class MKInput {
public:

	// ��ʼ�����̺����
	static void Initialize(HWND hWnd, IDXGISwapChain* pSwapChain);
	// �ͷż��̺����
	static void Release();
	// ��ȡ��������ָ��
	static MKInput* GetMKInput();
	// ���¼����¼�
	void Update();

	// ��굥��
	bool Mouse_SingleClick(MKMouse mouseEvent) const;
	// �����갴���Ƿ�ס
	bool Mouse_LongClick(MKMouse mouseEvent) const;
	// �����갴������
	POINT GetMouseClickPos(MKMouse mouseEvent) const;
	// ����������
	POINT GetMousePos() const { return m_MousePos; }
	// ��ȡ����������λ��XY
	POINT GetMouseLXY() const 
	{ 
		return POINT{ m_MouseState.lX, m_MouseState.lY }; 
	}
	// ��ȡ�����������Ϸ���Ϊ����
	int GetMouseLRoll() const { return m_MouseState.lZ / 120; }
	// �����̰���
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