#ifndef _GAMEAPP_H_
#define _GAMEAPP_H_
#include "d3dApp.h"
#include "Sprite.h"
#include "MKInput.h"
#include "PlaneManager.h"




enum class GameMode { 
	MENU,		// 主菜单
	CHOICE,		// 选择飞机界面
	PLAYING,	// 游戏中
	SETTING,	// 设置页面
	RANK,		// 排行榜
	PAUSE,		// 暂停
	SCORE,		// 结算页面
	COMPLETE	// 完成一关页面
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
	MKInput*									m_pMKInput;				// 键鼠输入
	
	D2DText										m_SmallFont;			// 小字体
	D2DText										m_MidFont;				// 中等字体
	D2DText										m_LargeFont;			// 大字体

	ID3DX11Effect*								m_pFX;					// Effects
	ID3DX11EffectTechnique*						m_pTech;				// Technique
	GameMode									m_Mode;					// 游戏模式
	std::vector<std::pair<std::wstring, int>>	m_HighScore;			// 高分
	wchar_t										m_Name[3][2];			// 用于设置名字

	int											m_MaxStage;				// 当前支持的关卡数

	

	std::map<std::string, Sprite2D>				m_Sprite2D;				// 精灵集合

};

#endif