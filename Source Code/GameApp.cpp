#include "GameApp.h"
#include "XUtility.h"


GameApp::GameApp(HINSTANCE hInstance)
	: D3DApp(hInstance), m_pFX(nullptr), m_pTech(nullptr), 
		m_pMKInput(nullptr)
{
	m_MainWndCaption = L"����ս��";
}

GameApp::~GameApp()
{
	ReleaseCOM(m_pFX);
	ReleaseStatic(Sprite2D);
	ReleaseStatic(D2DText);
	ReleaseStatic(MKInput);
	ReleaseStatic(PlaneManager);
}

bool GameApp::Init()
{
	if (!D3DApp::Init())
		return false;
	
	m_Mode = GameMode::MENU;
	InitFX();
	InitDInput();
	Init2D();
	InitText();
	LoadHighScore();
	CheckMaxStage();
	return true;
}

void GameApp::InitFX()
{
	m_pFX = nullptr;
	// ��ʼ��Effect
	BuildFX(m_pD3DDevice, L"HLSL/Basic.hlsl", &m_pFX);
	assert(m_pFX);
	m_pTech = m_pFX->GetTechniqueByName("Sprite2D");
}



void GameApp::Init2D()
{

	Sprite2D::Initialize(m_hMainWnd, m_pD3DImmediateContext, m_pSwapChain, m_pFX, "Sprite2D");
	// ����ս�������ʼ��
	m_Sprite2D["playerRed"].InitTexture("Texture/playerRed.png");
	m_Sprite2D["playerBlue"].InitTexture("Texture/playerBlue.png");
	PlaneManager::player.Initialize(1);
	// �����ӵ���ʼ��
	m_Sprite2D["fire"].InitTexture("Texture/fire.png");
	m_Sprite2D["flame1"].InitTexture("Texture/flame1.png");
	// �л��ӵ���ʼ��
	m_Sprite2D["bullet"].InitTexture("Texture/bullet.png");
	m_Sprite2D["laser"].InitTexture("Texture/laser.png");
	m_Sprite2D["flame2"].InitTexture("Texture/flame2.png");
	// Item��ʼ��
	m_Sprite2D["bomb"].InitTexture("Texture/bomb.png");
	m_Sprite2D["shield"].InitTexture("Texture/shield.png");
	m_Sprite2D["power"].InitTexture("Texture/power.png");
	// Ѫ��
	m_Sprite2D["health"].InitTexture("Texture/health.png");
	// ������ʼ��
	m_Sprite2D["background"].InitTexture("Texture/background.png");
	// �ɻ������ߺ͵л���ʼ��
	PlaneManager::Initialize(MainWnd(), m_pMKInput);
	PlaneManager::InitEnemyPlanes();
	// �л������ʼ��
	std::string key;
	std::string path = "Texture/";
	for (const EnemyPlane& plane : PlaneManager::enemyPlaneInfo)
	{
		key = plane.GetTexName();
		m_Sprite2D[key].InitTexture((path + key + ".png").c_str());
	}
}

void GameApp::InitDInput()
{
	MKInput::Initialize(m_hMainWnd, m_pSwapChain);
	m_pMKInput = MKInput::GetMKInput();
}

void GameApp::InitText()
{
	D2DText::Initialize(m_hMainWnd, m_pSwapChain);
	m_SmallFont.Reset(L"����", 20, D2D1::ColorF::White, DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
	m_MidFont.Reset(L"����", 30, D2D1::ColorF::White, DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
	m_LargeFont.Reset(L"����", 60, D2D1::ColorF::White, DWRITE_TEXT_ALIGNMENT_CENTER, DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
	m_Name[0][0] = m_Name[1][0] = m_Name[2][0] = L'A';
	m_Name[0][1] = m_Name[1][1] = m_Name[2][1] = 0;
}



void GameApp::OnResize()
{
	D2DText::Discard();
	D3DApp::OnResize();
	D2DText::OnReset();

	m_pSwapChain->GetFullscreenState(&m_FullScreen, nullptr);
	
}

void GameApp::UpdateScene(float dt)
{
	m_pMKInput->Update();
	
	if (m_Mode == GameMode::PLAYING || m_Mode == GameMode::COMPLETE)
		PlaneManager::UpdateScene();
}



void GameApp::DrawScene()
{
	assert(m_pD3DImmediateContext);
	assert(m_pSwapChain);
	// **************************************
	// ** TODO�����õ�ɫ                  ***
	// **************************************
	m_pD3DImmediateContext->ClearRenderTargetView(m_pRenderTargetView, reinterpret_cast<const float*>(&Colors::Silver));		//��ɫ
	m_pD3DImmediateContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// ******************************************
	// *** TODO�����л���                     ***
	// ******************************************
	// ��ͼ��˳�������
	DrawGameSence();
	DrawBackground();
	DrawUI();
	HR(m_pSwapChain->Present(0, 0));
}

void GameApp::DrawGameSence()
{
	// ��Ϸ�С���ɡ���ͣ�л���
	if (m_Mode == GameMode::PLAYING || m_Mode == GameMode::PAUSE || m_Mode == GameMode::COMPLETE)
	{
		RECT rc;
		std::vector<RECT> rcs;
		std::vector<float> radians;
		// �����bossģʽ���Ȼ���Ѫ��
		if (PlaneManager::bossFrameCnt <= PlaneManager::totalFrameCnt)
		{
			if (!PlaneManager::enemyPlanes.empty())
			{
				int nums = PlaneManager::enemyPlanes.front().GetHealthSlices();
				radians.resize(nums);
				rc = ToScreenRect({ 200, 100, 202, 130 });
				for (int i = 0; i < nums; ++i, rc.left += 4, rc.right += 4)
					rcs.push_back(rc);
				m_Sprite2D["health"].StretchRects(rcs, radians);
				m_Sprite2D["health"].Draw();
			}
		}

		// �����ӵ�
		rcs.clear();
		radians.clear();
		for (const Bullet& bullet : PlaneManager::enemyBullets)
		{
			rcs.push_back(ToScreenRect(bullet.GetRect()));
			radians.push_back(0.0f);
		}
		m_Sprite2D["bullet"].StretchRects(rcs, radians);
		m_Sprite2D["bullet"].Draw();

		rcs.clear();
		radians.clear();
		for (const Bullet& laser : PlaneManager::enemyLasers)
		{
			rcs.push_back(ToScreenRect(laser.GetRect()));
			radians.push_back(MathHelper::Pi / 2);
		}
		m_Sprite2D["laser"].StretchRects(rcs, radians);
		m_Sprite2D["laser"].Draw();

		rcs.clear();
		radians.clear();
		for (const Bullet& bullet : PlaneManager::enemyFlames)
		{
			rcs.push_back(ToScreenRect(bullet.GetRect()));
			radians.push_back(-bullet.GetRotation());
		}
		m_Sprite2D["flame2"].StretchRects(rcs, radians);
		m_Sprite2D["flame2"].Draw();

		rcs.clear();
		radians.clear();
		for (const Bullet& fire : PlaneManager::playerBullets)
		{
			rcs.push_back(ToScreenRect(fire.GetRect()));
			radians.push_back(-fire.GetRotation());
		}
		m_Sprite2D["fire"].StretchRects(rcs, radians);
		m_Sprite2D["fire"].Draw();

		rcs.clear();
		radians.clear();
		for (const Bullet& flame : PlaneManager::playerFlames)
		{
			rcs.push_back(ToScreenRect(flame.GetRect()));
			radians.push_back(-flame.GetRotation());
		}
		m_Sprite2D["flame1"].StretchRects(rcs, radians);
		m_Sprite2D["flame1"].Draw();
		// Ȼ�����Item
		for (const Item& item : PlaneManager::items)
		{
			rc = ToScreenRect(item.GetRect());
			std::string str;
			switch (item.GetType())
			{
			case 1: str = "bomb";
				break;
			case 2: str = "power";
				break;
			case 3: str = "shield";
				break;
			default: break;
			}
			m_Sprite2D[str].StretchRect(nullptr, &rc, nullptr, 0.0f);
			m_Sprite2D[str].Draw();
		}
		// Ȼ����Ƶл�
		for (const EnemyPlane& plane : PlaneManager::enemyPlanes)
		{
			rc = ToScreenRect(plane.GetRect());
			std::string str = plane.GetTexName();

			if (plane.IsRotating())
			{
				LONG width = rc.right - rc.left;
				LONG height = rc.bottom - rc.top;
				POINT center = { width / 2 + rc.left, height / 2 + rc.top };
				float rad = plane.GetCurrFrameCnt() / 10 % 2 * MathHelper::Pi;
				m_Sprite2D[str].StretchRect(nullptr, &rc, &center, rad);
			}
			else
				m_Sprite2D[str].StretchRect(nullptr, &rc, nullptr, 0.0f);
			m_Sprite2D[str].Draw();
		}
		// ���Ƽ���ս��
		if (PlaneManager::player.IsAlive())
		{
			rc = ToScreenRect(PlaneManager::player.GetRect());
			if (PlaneManager::player.GetTypeID() == 1)
			{
				m_Sprite2D["playerRed"].StretchRect(nullptr, &rc, nullptr, 0.0f);
				m_Sprite2D["playerRed"].Draw();
			}
			else if (PlaneManager::player.GetTypeID() == 2)
			{
				m_Sprite2D["playerBlue"].StretchRect(nullptr, &rc, nullptr, 0.0f);
				m_Sprite2D["playerBlue"].Draw();
			}
		}
	}
}

void GameApp::DrawBackground()
{
	
	static RECT backgroundRect = ToScreenRect({ 0, 900, 750, 1800 });
	RECT destRect = ToScreenRect({ 0, 0, 750, 900 });
	if (backgroundRect.bottom < 0)
	{
		backgroundRect.top = 900;
		backgroundRect.bottom = 1800;
	}
	// ��ͣʱ������
	if (m_Mode != GameMode::PAUSE)
	{
		backgroundRect.top -= 5;
		backgroundRect.bottom -= 5;
	}

	m_Sprite2D["background"].StretchRect(&backgroundRect, &destRect, nullptr, 0.0f);
	m_Sprite2D["background"].Draw();
}

void GameApp::DrawUI()
{
	std::wstring wstr;
	switch (m_Mode)
	{
	case GameMode::MENU: DrawMenu();
		break;
	case GameMode::CHOICE: DrawChoice();
		break;
	case GameMode::PLAYING: DrawPlaying();
		break;
	case GameMode::SETTING: DrawSetting();
		break;
	case GameMode::RANK: DrawRank();
		break;
	case GameMode::PAUSE: DrawPause();
		break;
	case GameMode::SCORE: DrawScore();
		break;
	case GameMode::COMPLETE: DrawComplete();
		break;
	}
}

void GameApp::DrawMenu()
{
	m_LargeFont.DrawString(L"����ս��", ToScreenRectF(D2D1_RECT_F{ 250.0f, 100.0f, 500.0f, 160.0f }));

	m_MidFont.SetTextAlign(DWRITE_TEXT_ALIGNMENT_CENTER);

	if (ClickBox(m_pMKInput, ToScreenRectF(D2D1_RECT_F{ 315.0f, 400.0f, 435.0f, 460.0f }), m_MidFont, L"��ʼ��Ϸ"))
	{
		m_Mode = GameMode::CHOICE;
		PlaneManager::snd["START"]->Play(0UL, 0UL, LevelToVolume(PlaneManager::volume));
	}
	if (ClickBox(m_pMKInput, ToScreenRectF(D2D1_RECT_F{ 345.0f, 480.0f, 405.0f, 540.0f }), m_MidFont, L"����"))
	{
		m_Mode = GameMode::SETTING;
		PlaneManager::snd["START"]->Play(0UL, 0UL, LevelToVolume(PlaneManager::volume));
	}
	if (ClickBox(m_pMKInput, ToScreenRectF(D2D1_RECT_F{ 330.0f, 560.0f, 420.0f, 620.0f }), m_MidFont, L"���а�"))
	{
		m_Mode = GameMode::RANK;
		PlaneManager::snd["START"]->Play(0UL, 0UL, LevelToVolume(PlaneManager::volume));
	}
	if (ClickBox(m_pMKInput, ToScreenRectF(D2D1_RECT_F{ 315.0f, 640.0f, 435.0f, 700.0f }), m_MidFont, L"¼��ط�"))
	{
		PlaneManager::snd["START"]->Play(0UL, 0UL, LevelToVolume(PlaneManager::volume));
		char cwd[MAX_PATH];
		char filePath[MAX_PATH];
		GetCurrentDirectoryA(MAX_PATH, cwd);	// ����Ĭ�Ϲ���·��
		//�����ļ�����
		OPENFILENAMEA ofn;
		ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
		ofn.lStructSize = sizeof(OPENFILENAMEA);
		ofn.hwndOwner = m_hMainWnd;
		ofn.lpstrFile = filePath;
		ofn.lpstrFile[0] = '\0';
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrFilter = "Dat�ļ�(*.dat)\0*.dat\0";
		ofn.nFilterIndex = 0;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = "\\Replay";
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

		BOOL prevFullScreen = m_FullScreen;
		if (prevFullScreen)
			m_pSwapChain->SetFullscreenState(false, nullptr);

		if (GetOpenFileNameA(&ofn))
		{
			SetCurrentDirectoryA(cwd);
			PlaneManager::InitReplay(filePath);
			m_Mode = GameMode::PLAYING;
			PlaneManager::replayMode = true;
			PlaneManager::score = 0;

		}
		else
			SetCurrentDirectoryA(cwd);

		if (prevFullScreen)
			m_pSwapChain->SetFullscreenState(true, nullptr);
	}
	if (ClickBox(m_pMKInput, ToScreenRectF(D2D1_RECT_F{ 315.0f, 720.0f, 435.0f, 780.0f }), m_MidFont, L"������Ϸ"))
	{
		// �˳�
		PostQuitMessage(0);
	}

	std::wstring wstr = L"����Ver1.0 X_Jun";
	m_MidFont.DrawString(wstr.c_str(), ToScreenRectF(D2D1_RECT_F{ 200.0f, 830.0f, 550.0f, 890.0f }));
	m_MidFont.SetTextAlign(DWRITE_TEXT_ALIGNMENT_LEADING);
}

void GameApp::DrawChoice()
{
	m_LargeFont.DrawString(L"ѡ��ս��", ToScreenRectF(D2D1_RECT_F{ 250.0f, 100.0f, 500.0f, 160.0f }));

	if (ClickBox(m_pMKInput, ToScreenRectF(D2D1_RECT_F{ 100.0f, 400.0f, 350.0f, 570.0f }), m_MidFont,
		L"ɢ����\n���٣�B\n������A\n������B\n����ʣ�12x\n"))
	{
		PlaneManager::player.Initialize(1);
		PlaneManager::InitStage(PlaneManager::stage);
		PlaneManager::score = 0;
		m_Mode = GameMode::PLAYING; 
		PlaneManager::snd["START"]->Play(0UL, 0UL, LevelToVolume(PlaneManager::volume));
		PlaneManager::replayMode = false;
		// �������������Ȼ��ʹ��
		PlaneManager::randomSeed = time(nullptr);	
		srand((unsigned)PlaneManager::randomSeed);
		
		// ��¼���������д���ʼ��Ϣ
		std::ostringstream oss;
		oss << "Replay/" << std::hex << PlaneManager::randomSeed << ".dat";
		std::string filePath = oss.str();
		std::ofstream& ofs = PlaneManager::replayOut;
		ofs.open(filePath, std::ios::out | std::ios::binary);
		ofs.write(reinterpret_cast<char *>(&PlaneManager::randomSeed), sizeof(time_t));
		ofs.write(reinterpret_cast<char *>(&PlaneManager::stage), sizeof(int));
		int bossMode = PlaneManager::bossMode;
		ofs.write(reinterpret_cast<char *>(&bossMode), sizeof(int));
		int planeType = 1;
		ofs.write(reinterpret_cast<char *>(&planeType), sizeof(int));
	}
	if (ClickBox(m_pMKInput, ToScreenRectF(D2D1_RECT_F{ 400.0f, 400.0f, 650.0f, 570.0f }), m_MidFont,
		L"������\n���٣�A\n������B\n������A\n����ʣ�16x\n"))
	{
		PlaneManager::player.Initialize(2);
		PlaneManager::InitStage(PlaneManager::stage);
		PlaneManager::score = 0;
		m_Mode = GameMode::PLAYING;
		PlaneManager::snd["START"]->Play(0UL, 0UL, LevelToVolume(PlaneManager::volume));
		PlaneManager::replayMode = false;
		// �������������Ȼ��ʹ��
		PlaneManager::randomSeed = time(nullptr);
		srand((unsigned)PlaneManager::randomSeed);

		// ��¼���������д���ʼ��Ϣ
		std::ostringstream oss;
		oss << "Replay/" << std::hex << PlaneManager::randomSeed << ".dat";
		std::string filePath = oss.str();
		std::ofstream& ofs = PlaneManager::replayOut;
		ofs.open(filePath, std::ios::out | std::ios::binary);
		ofs.write(reinterpret_cast<char *>(&PlaneManager::randomSeed), sizeof(time_t));
		ofs.write(reinterpret_cast<char *>(&PlaneManager::stage), sizeof(int));
		int bossMode = PlaneManager::bossMode;
		ofs.write(reinterpret_cast<char *>(&bossMode), sizeof(int));
		int planeType = 2;
		ofs.write(reinterpret_cast<char *>(&planeType), sizeof(int));
	}
}

void GameApp::DrawPlaying()
{
	// �����ı�UI
	// ���Ͻ�
	std::wstring wstr;
	wstr = L"Lives: " + std::to_wstring(PlaneManager::player.GetLives());
	m_MidFont.DrawString(wstr.c_str(), ToScreenRectF(D2D1_RECT_F{ 10.0f, 10.0f, 190.0f, 40.0f }));
	wstr = L"Credits: " + std::to_wstring(PlaneManager::player.GetCredits());
	m_MidFont.DrawString(wstr.c_str(), ToScreenRectF(D2D1_RECT_F{ 10.0f, 40.0f, 190.0f, 70.0f }));
	wstr = L"Shields: " + std::to_wstring(PlaneManager::player.GetShields());
	m_MidFont.DrawString(wstr.c_str(), ToScreenRectF(D2D1_RECT_F{ 10.0f, 70.0f, 190.0f, 100.0f }));
	wstr = L"Bombs: " + std::to_wstring(PlaneManager::player.GetBombs());
	m_MidFont.DrawString(wstr.c_str(), ToScreenRectF(D2D1_RECT_F{ 10.0f, 100.0f, 190.0f, 130.0f }));

	// ���½�
	if (PlaneManager::debugMode)
	{
		wstr = L"FrameCount: " + std::to_wstring(PlaneManager::totalFrameCnt);
		m_SmallFont.DrawString(wstr.c_str(), ToScreenRectF(D2D1_RECT_F{ 10.0f, 810.0f, 180.0f, 830.0f }));
		wstr = L"EnemyPlanes: " + std::to_wstring(PlaneManager::enemyPlanes.size());
		m_SmallFont.DrawString(wstr.c_str(), ToScreenRectF(D2D1_RECT_F{ 10.0f, 830.0f, 180.0f, 850.0f }));
		wstr = L"EnemyBullets: " + std::to_wstring(
			PlaneManager::enemyBullets.size() + PlaneManager::enemyLasers.size() + PlaneManager::enemyFlames.size());
		m_SmallFont.DrawString(wstr.c_str(), ToScreenRectF(D2D1_RECT_F{ 10.0f, 850.0f, 180.0f, 870.0f }));
		wstr = L"PlayerBullets: " + std::to_wstring(PlaneManager::playerBullets.size() +
			PlaneManager::playerFlames.size());
		m_SmallFont.DrawString(wstr.c_str(), ToScreenRectF(D2D1_RECT_F{ 10.0f, 870.0f, 180.0f, 890.0f }));
	}
	// ���Ͻ�
	wstr = L"Score:   " + std::to_wstring(PlaneManager::score);
	m_MidFont.SetTextAlign(DWRITE_TEXT_ALIGNMENT_TRAILING);
	m_MidFont.DrawString(wstr.c_str(), ToScreenRectF(D2D1_RECT_F{ 600.0f, 10.0f, 740.0f, 70.0f }));
	wstr = L"Combo: " + std::to_wstring(PlaneManager::player.GetCurrRate()) + L'x';
	m_MidFont.DrawString(wstr.c_str(), ToScreenRectF(D2D1_RECT_F{ 570.0f, 70.0f, 740.0f, 100.0f }));
	m_MidFont.SetTextAlign(DWRITE_TEXT_ALIGNMENT_LEADING);



	// ���½�
	wstr = L"Stage" + std::to_wstring(PlaneManager::stage);
	m_MidFont.DrawString(wstr.c_str(), ToScreenRectF(D2D1_RECT_F{ 640.0f, 860.0f, 740.0f, 890.0f }));
	
	// ����⵽ESC������
	if (m_pMKInput->Key_Down(DIK_ESCAPE))
	{
		m_Mode = GameMode::PAUSE;
		// �����з��ŵ�����ͣ��
		for (auto p : PlaneManager::snd)
			if (p.second->IsSoundPlaying())
				p.second->Stop();
	}

	// �ط�ģʽ
	if (PlaneManager::replayMode)
	{
		bool endReplay = PlaneManager::replayInfo[PlaneManager::stage].rbegin()->first < PlaneManager::totalFrameCnt;
		if (endReplay)
			m_MidFont.DrawString(L"Replay Finish", ToScreenRectF(D2D1_RECT_F{ 260.0f, 860.0f, 490.0f, 900.0f }));
		else
			m_MidFont.DrawString(L"Replay Mode", ToScreenRectF(D2D1_RECT_F{ 280.0f, 860.0f, 470.0f, 900.0f }));
	}
	

	// BOSS��������5s
	if (PlaneManager::bossFrameCnt - 400 <= PlaneManager::totalFrameCnt &&
		PlaneManager::totalFrameCnt < PlaneManager::bossFrameCnt - 100)
	{
		if (PlaneManager::totalFrameCnt == PlaneManager::bossFrameCnt - 400)
			PlaneManager::snd["WARNING"]->Play(0UL, 0UL, LevelToVolume(PlaneManager::volume));
		m_LargeFont.DrawString(L"����" , ToScreenRectF(D2D1_RECT_F{ 300.0f, 100.0f, 450.0f, 200.0f }));
		m_MidFont.DrawString(L"Boss��Ϯ����", ToScreenRectF(D2D1_RECT_F{ 285.0f, 200.0f, 465.0f, 300.0f }));
	}

	// ����2s�����LivesΪ0����ͣ��Ϸ
	if (PlaneManager::player.GetLives() == 0 && !PlaneManager::player.IsAlive()
		&& PlaneManager::totalFrameCnt - PlaneManager::player.GetCrashFrameCnt() >= 120)
	{
		// �õ��߷־�ȥ�Ǽ�
		if (PlaneManager::score > m_HighScore[7].second)
			m_Mode = GameMode::SCORE;
		else
			m_Mode = GameMode::PAUSE;
	}

	// ���boss�����������÷ֽ���
	if (PlaneManager::totalFrameCnt > PlaneManager::bossFrameCnt &&
		PlaneManager::enemyPlanes.empty() && !PlaneManager::bossKillFrameCnt)
	{
		PlaneManager::bossKillFrameCnt = PlaneManager::totalFrameCnt;
		PlaneManager::finalRate = PlaneManager::player.GetCurrRate();
		// ��յз��ӵ�
		PlaneManager::enemyBullets.clear();
		PlaneManager::enemyLasers.clear();
	}
	if (PlaneManager::bossKillFrameCnt && PlaneManager::totalFrameCnt - 120 > 
		PlaneManager::bossKillFrameCnt)
	{
		m_Mode = GameMode::COMPLETE;
	}
}

void GameApp::DrawSetting()
{
	m_LargeFont.DrawString(L"����", ToScreenRectF(D2D1_RECT_F{ 250.0f, 100.0f, 500.0f, 160.0f }));
	// ������ʼ�ؿ�
	std::wstring wstr = L"��ʼ�ؿ�: " + std::to_wstring(PlaneManager::stage);
	if (PlaneManager::bossMode)
		wstr += L"BOSS";
	if (ClickBox(m_pMKInput, ToScreenRectF(D2D1_RECT_F{ 250.0f, 400.0f, 500.0f, 460.0f }), m_MidFont, wstr.c_str()))
	{
		if (PlaneManager::stage < m_MaxStage)
			PlaneManager::stage++;
		else
		{
			PlaneManager::bossMode = !PlaneManager::bossMode;
			PlaneManager::stage = 1;
		}
		PlaneManager::snd["START"]->Play(0UL, 0UL, LevelToVolume(PlaneManager::volume));
	}
	// ��ʾ������Ϣ
	wstr = L"��ʾ������Ϣ: ";
	wstr += PlaneManager::debugMode ? L"��" : L"��";
	if (ClickBox(m_pMKInput, ToScreenRectF(D2D1_RECT_F{ 250.0f, 480.0f, 500.0f, 540.0f }), m_MidFont, wstr.c_str()))
	{
		PlaneManager::debugMode = !PlaneManager::debugMode;
		PlaneManager::snd["START"]->Play(0UL, 0UL, LevelToVolume(PlaneManager::volume));
	}

	// ������Ϸ����
	wstr = L"����: ";
	wstr += std::to_wstring(PlaneManager::volume);
	if (ClickBox(m_pMKInput, ToScreenRectF(D2D1_RECT_F{ 250.0f, 560.0f, 400.0f, 620.0f }), m_MidFont, wstr.c_str()))
	{
		if (PlaneManager::volume == 10)
			PlaneManager::volume = 0;
		else
			PlaneManager::volume++;
		PlaneManager::snd["START"]->Play(0UL, 0UL, LevelToVolume(PlaneManager::volume));
	}
	
	// ����������ToScreenRect(ToScreenRectF(D2D1_RECT_F{\1}))
	wstr = L"ȫ��:";
	wstr += (m_FullScreen ? L"��" : L"��");
	if (ClickBox(m_pMKInput, ToScreenRectF(D2D1_RECT_F{ 250.0f, 640.0f, 500.0f, 700.0f }), m_MidFont, wstr.c_str()))
	{
		m_FullScreen = !m_FullScreen;
		m_pSwapChain->SetFullscreenState(m_FullScreen, nullptr);
	}

	// ����������
	if (ClickBox(m_pMKInput, ToScreenRectF(D2D1_RECT_F{ 250.0f, 720.0f, 500.0f, 780.0f }), m_MidFont, L"����������"))
	{
		m_Mode = GameMode::MENU;
		PlaneManager::snd["START"]->Play(0UL, 0UL, LevelToVolume(PlaneManager::volume));
	}
}

void GameApp::DrawRank()
{
	m_LargeFont.DrawString(L"���а�", ToScreenRectF(D2D1_RECT_F{ 200.0f, 100.0f, 500.0f, 160.0f }));
	m_MidFont.DrawString(L"���� ����    �÷�", ToScreenRectF(D2D1_RECT_F{ 200.0f, 200.0f, 550.0f, 260.0f }));
	for (int i = 0; i < 8; ++i)
	{
		std::wstring wstr = std::to_wstring(i + 1) + L"     " + 
			m_HighScore[i].first + L"     " + std::to_wstring(m_HighScore[i].second);
		m_MidFont.DrawString(wstr.c_str(),
			ToScreenRectF(D2D1_RECT_F{ 200.0f, 300.0f + 60.0f * i, 550.0f, 360.0f + 60.0f * i }));
	}
	if (ClickBox(m_pMKInput, ToScreenRectF(D2D1_RECT_F{ 340.0f, 800.0f, 410.0f, 860.0f }), m_MidFont, L"����"))
	{
		m_Mode = GameMode::MENU;
		PlaneManager::snd["START"]->Play(0UL, 0UL, LevelToVolume(PlaneManager::volume));
	}
}

void GameApp::DrawPause()
{
	std::wstring wstr;
	m_MidFont.SetTextAlign(DWRITE_TEXT_ALIGNMENT_CENTER);
	// �����ҵ��������ù���
	if (PlaneManager::player.GetCredits() == 0 && PlaneManager::player.GetLives() == 0 &&
		!PlaneManager::player.IsAlive())
	{
		m_MidFont.DrawString(L"��Ϸ����", ToScreenRectF(D2D1_RECT_F{ 250.0f, 300.0f, 500.0f, 360.0f }));
		wstr = L"�÷֣�" + std::to_wstring(PlaneManager::score);
		m_MidFont.DrawString(wstr.c_str(), ToScreenRectF(D2D1_RECT_F{ 250.0f, 400.0f, 500.0f, 460.0f }));
		m_MidFont.DrawString(L"���س�������", ToScreenRectF(D2D1_RECT_F{ 250.0f, 500.0f, 500.0f, 560.0f }));

		// ����⵽�س�������
		if (m_pMKInput->Key_Down(DIK_RETURN))
		{
			m_Mode = GameMode::MENU;
			// ��¼��ģʽ�£������ļ����
			if (!PlaneManager::replayMode)
			{
				PlaneManager::replayOut.close();
				PlaneManager::replayInfo.clear();
			}
		}
			
	}
	// �����һ��б�
	else if (PlaneManager::player.GetLives() == 0 && !PlaneManager::player.IsAlive())
	{
		wstr = L"�÷֣�" + std::to_wstring(PlaneManager::score);
		m_MidFont.DrawString(wstr.c_str(), ToScreenRectF(D2D1_RECT_F{ 250.0f, 300.0f, 500.0f, 360.0f }));
		m_MidFont.DrawString(L"���Һ�÷�����", ToScreenRectF(D2D1_RECT_F{ 250.0f, 400.0f, 500.0f, 460.0f }));
		m_MidFont.DrawString(L"���س�����", ToScreenRectF(D2D1_RECT_F{ 250.0f, 500.0f, 500.0f, 560.0f }));
		m_MidFont.DrawString(L"��Esc����", ToScreenRectF(D2D1_RECT_F{ 250.0f, 600.0f, 500.0f, 660.0f }));

		// ����⵽�س�������
		if (m_pMKInput->Key_Down(DIK_RETURN))
		{
			m_Mode = GameMode::PLAYING;
			PlaneManager::player.Respawn();
			PlaneManager::score = 0;
		}
		// Esc������
		if (m_pMKInput->Key_Down(DIK_ESCAPE))
		{
			m_Mode = GameMode::MENU;
			// ��¼��ģʽ�£�д����ֹ��ǣ������ļ����
			if (!PlaneManager::replayMode)
			{
				PlaneManager::replayOut.close();
				PlaneManager::replayInfo.clear();
			}
		}
	}
	// ���඼����ͨ��ͣ
	else
	{
		m_MidFont.DrawString(L"��Ϸ����ͣ", ToScreenRectF(D2D1_RECT_F{ 250.0f, 300.0f, 500.0f, 360.0f }));
		m_MidFont.DrawString(L"���س�������", ToScreenRectF(D2D1_RECT_F{ 250.0f, 400.0f, 500.0f, 460.0f }));
		m_MidFont.DrawString(L"��F2�ص�������", ToScreenRectF(D2D1_RECT_F{ 250.0f, 500.0f, 500.0f, 560.0f }));
		// ����⵽�س�������
		if (m_pMKInput->Key_Down(DIK_RETURN))
			m_Mode = GameMode::PLAYING;
		if (m_pMKInput->Key_Down(DIK_F2))
		{
			m_Mode = GameMode::MENU;
			// ��¼��ģʽ�£�ɾ��¼��
			if (!PlaneManager::replayMode)
			{
				PlaneManager::replayOut.close();
				PlaneManager::replayInfo.clear();

				std::ostringstream oss;
				oss << "Replay/" << std::hex << PlaneManager::randomSeed << ".dat";
				std::string filePath = oss.str();
				remove(filePath.c_str());
			}
		}
			
	}
	m_MidFont.SetTextAlign(DWRITE_TEXT_ALIGNMENT_LEADING);
}

void GameApp::DrawScore()
{
	std::wstring wstr = L"�÷֣�" + std::to_wstring(PlaneManager::score);
	m_MidFont.DrawString(wstr.c_str(), ToScreenRectF(D2D1_RECT_F{ 250.0f, 300.0f, 500.0f, 360.0f }));
	m_MidFont.DrawString(L"ѡ��3�ֽ��ǳ�", ToScreenRectF(D2D1_RECT_F{ 250.0f, 400.0f, 500.0f, 460.0f }));
	for (int i = 0; i < 3; ++i)
	{
		if (ClickBox(m_pMKInput, ToScreenRectF(D2D1_RECT_F{ 250.0f + i * 30.0f, 500.0f, 280.0f + i * 30.0f, 560.0f }),
			m_MidFont, L"��") && m_Name[i][0] > 0x20)
		{
			m_Name[i][0]--;
			PlaneManager::snd["START"]->Play(0UL, 0UL, LevelToVolume(PlaneManager::volume));
		}
		m_MidFont.DrawString(m_Name[i], ToScreenRectF(D2D1_RECT_F{ 250.0f + i * 30.0f, 560.0f, 280.0f + i * 30.0f, 620.0f }));
		if (ClickBox(m_pMKInput, ToScreenRectF(D2D1_RECT_F{ 250.0f + i * 30.0f, 620.0f, 280.0f + i * 30.0f, 680.0f }),
			m_MidFont, L"��") && m_Name[i][0] < 0x7A)
		{
			m_Name[i][0]++;
			PlaneManager::snd["START"]->Play(0UL, 0UL, LevelToVolume(PlaneManager::volume));
		}
	}
	if (ClickBox(m_pMKInput, ToScreenRectF(D2D1_RECT_F{ 250.0f, 700.0f, 500.0f, 760.0f }), m_MidFont, L"ȷ��"))
	{
		wstr.resize(3);
		for (int i = 0; i < 3; ++i)
			wstr[i] = m_Name[i][0];
		StoreHighScore(wstr, PlaneManager::score);
		// ����ؿ�ȫ����ɣ��ص������棻��֮�ص���ͣ
		if (!PlaneManager::stageComplete)
			m_Mode = GameMode::MENU;
		else
			m_Mode = GameMode::PAUSE;

		m_Name[0][0] = m_Name[1][0] = m_Name[2][0] = L'A';
		PlaneManager::snd["START"]->Play(0UL, 0UL, LevelToVolume(PlaneManager::volume));
	}
}

void GameApp::DrawComplete()
{
	int leftBombs = PlaneManager::player.GetBombs();
	int leftLives = PlaneManager::player.GetLives();
	int leftShields = PlaneManager::player.GetShields();
	int noMissScore = PlaneManager::noMiss ? 10000 * PlaneManager::stage : 0;
	std::wstring wstr = L"ʣ��ը���÷�  " + std::to_wstring(leftBombs) + L" x 1000";
	m_MidFont.DrawString(wstr.c_str(), ToScreenRectF(D2D1_RECT_F{ 150.0f, 200.0f, 600.0f, 260.0f }));
	wstr = L"ʣ�������÷�  " + std::to_wstring(leftLives) + L" x 1000";
	m_MidFont.DrawString(wstr.c_str(), ToScreenRectF(D2D1_RECT_F{ 150.0f, 260.0f, 600.0f, 320.0f }));
	wstr = L"ʣ�໤�ܵ÷�  " + std::to_wstring(leftShields) + L" x 1000";
	m_MidFont.DrawString(wstr.c_str(), ToScreenRectF(D2D1_RECT_F{ 150.0f, 320.0f, 600.0f, 380.0f }));
	wstr = L"�������÷�    " + std::to_wstring(noMissScore);
	m_MidFont.DrawString(wstr.c_str(), ToScreenRectF(D2D1_RECT_F{ 150.0f, 380.0f, 600.0f, 440.0f }));
	wstr = L"����           x " + std::to_wstring(PlaneManager::finalRate);
	m_MidFont.DrawString(wstr.c_str(), ToScreenRectF(D2D1_RECT_F{ 150.0f, 440.0f, 600.0f, 500.0f }));
	int getScore = ((leftBombs + leftLives + leftShields) * 1000 + noMissScore) * 
		PlaneManager::finalRate;
	wstr = L"����÷�      " + std::to_wstring(getScore);
	m_MidFont.DrawString(wstr.c_str(), ToScreenRectF(D2D1_RECT_F{ 150.0f, 500.0f, 600.0f, 560.0f }));
	int totalScore = PlaneManager::score + getScore;
	wstr = L"��ǰ�ܷ�      " + std::to_wstring(totalScore);
	m_MidFont.DrawString(wstr.c_str(), ToScreenRectF(D2D1_RECT_F{ 150.0f, 600.0f, 600.0f, 760.0f }));

	if (PlaneManager::totalFrameCnt - 600 > PlaneManager::bossKillFrameCnt)
	{
		PlaneManager::score = totalScore;
		if (PlaneManager::stage == m_MaxStage)
		{
			// ¼��ģʽ�£�ֱ�ӷ���
			if (PlaneManager::replayMode)
			{
				m_Mode = GameMode::MENU;
				return;
			}
				
			PlaneManager::replayOut.close();
			PlaneManager::replayInfo.clear();

			// �õ��߷־�ȥ�Ǽ�
			if (PlaneManager::score > m_HighScore[7].second)
				m_Mode = GameMode::SCORE;
			else
				m_Mode = GameMode::MENU;
		}
		else
		{
			// ������һ��
			PlaneManager::bossMode = false;
			PlaneManager::InitStage(PlaneManager::stage + 1);
			PlaneManager::player.Refresh();
			m_Mode = GameMode::PLAYING;
			// ��¼��ģʽ�£�¼���ļ�д����һ�صı��
			if (!PlaneManager::replayMode)
			{
				unsigned short nextStage[2] = { 0xffff, (unsigned short)(PlaneManager::stage) };
				PlaneManager::replayOut.write(reinterpret_cast<char *>(&nextStage), sizeof(short) * 2);
			}
			
		}
	}
}

void GameApp::LoadHighScore()
{
	std::ifstream fin("Data/HighScore.dat", std::ios::in | std::ios::binary);
	if (!fin.is_open())
	{
		std::ofstream fout("Data/HighScore.dat", std::ios::out | std::ios::binary);
		wchar_t name[4] = L"PL";
		int score;
		for (int i = 0; i < 8; ++i)
		{
			name[2] = L'1' + i;
			score = 4000000 - 500000 * i;
			fout.write(reinterpret_cast<char*>(name), sizeof(name));
			fout.write(reinterpret_cast<char*>(&score), sizeof(score));
		}
		fout.close();
		fin.open("Data/HighScore.dat", std::ios::in | std::ios::binary);
	}
	wchar_t name[4];
	int score;
	m_HighScore.clear();
	for (int i = 0; i < 8; ++i)
	{
		fin.read(reinterpret_cast<char*>(name), sizeof(name));
		fin.read(reinterpret_cast<char*>(&score), sizeof(score));
		m_HighScore.push_back(std::make_pair(name, score));
	}
	fin.close();
}

void GameApp::StoreHighScore(const std::wstring & wstr, int score)
{
	m_HighScore[7].first = wstr;
	m_HighScore[7].second = score;
	std::sort(m_HighScore.begin(), m_HighScore.end(), [](
		const std::pair<std::wstring, int>& lft, 
		const std::pair<std::wstring, int>& rht) {
		return lft.second > rht.second;
	});
	std::ofstream fout("Data/HighScore.dat", std::ios::out | std::ios::binary);
	wchar_t name[4] = L"";
	for (int i = 0; i < 8; ++i)
	{
		memcpy_s(name, sizeof(name), m_HighScore[i].first.data(), 
			m_HighScore[i].first.size() * sizeof(wchar_t));
		fout.write(reinterpret_cast<char*>(name), sizeof(name));
		fout.write(reinterpret_cast<char*>(&m_HighScore[i].second), sizeof(int));
	}
	fout.close();
}

void GameApp::CheckMaxStage()
{
	char level[30];
	char stage[30];
	FILE *fp1, *fp2;
	int i = 0;
	while (true)
	{
		sprintf(level, "Data/EnemyLevel%d.txt", i + 1);
		sprintf(stage, "Data/Stage%d.txt", i + 1);
		fp1 = fopen(level, "r");
		fp2 = fopen(stage, "r");
		if (fp1 && fp2)
			++i;
		else
			break;
		fclose(fp1);
		fclose(fp2);
	}
	m_MaxStage = i;
}

RECT GameApp::ToScreenRect(const RECT & rect)
{
	if (!m_FullScreen)
		return rect;
	else
	{
		RECT rc;
		rc.left = m_ClientWidth / 2 - 375 + rect.left;
		rc.top = rect.top;
		rc.right = m_ClientWidth / 2 - 375 + rect.right;
		rc.bottom = rect.bottom;
		return rc;
	}

}

D2D1_RECT_F GameApp::ToScreenRectF(const D2D1_RECT_F & rectF)
{
	if (!m_FullScreen)
		return rectF;
	else
	{
		D2D1_RECT_F rcF;
		rcF.left = m_ClientWidth / 2.0f - 375.0f + rectF.left;
		rcF.top = rectF.top;
		rcF.right = m_ClientWidth / 2.0f - 375.0f + rectF.right;
		rcF.bottom = rectF.bottom;
		return rcF;
	}
}




