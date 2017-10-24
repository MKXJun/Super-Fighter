#ifndef _GAMEAPP_H_
#define _GAMEAPP_H_
#include "d3dApp.h"
#include "Sprite.h"
#include "MKInput.h"
#include "PlaneManager.h"




enum class GameMode { 
	MENU,		// ���˵�
	CHOICE,		// ѡ��ɻ�����
	PLAYING,	// ��Ϸ��
	SETTING,	// ����ҳ��
	RANK,		// ���а�
	PAUSE,		// ��ͣ
	SCORE,		// ����ҳ��
	COMPLETE	// ���һ��ҳ��
};

class GameApp : public D3DApp
{
public:
	GameApp(HINSTANCE hInstance);
	~GameApp();

	bool Init();

	void InitFX();
	void Init2D();
	void InitDInput();
	void InitText();

	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void DrawGameSence();
	void DrawBackground();
	void DrawUI();

	void DrawMenu();
	void DrawChoice();
	void DrawPlaying();
	void DrawSetting();
	void DrawRank();
	void DrawPause();
	void DrawScore();
	void DrawComplete();

	void LoadHighScore();
	void StoreHighScore(const std::wstring& name, int score);

	void CheckMaxStage();
private:
	RECT ToScreenRect(const RECT& rect);
	D2D1_RECT_F ToScreenRectF(const D2D1_RECT_F& rectF);
	MKInput*									m_pMKInput;				// ��������
	
	D2DText										m_SmallFont;			// С����
	D2DText										m_MidFont;				// �е�����
	D2DText										m_LargeFont;			// ������

	ID3DX11Effect*								m_pFX;					// Effects
	ID3DX11EffectTechnique*						m_pTech;				// Technique
	GameMode									m_Mode;					// ��Ϸģʽ
	std::vector<std::pair<std::wstring, int>>	m_HighScore;			// �߷�
	wchar_t										m_Name[3][2];			// ������������

	int											m_MaxStage;				// ��ǰ֧�ֵĹؿ���

	

	std::map<std::string, Sprite2D>				m_Sprite2D;				// ���鼯��

};

#endif