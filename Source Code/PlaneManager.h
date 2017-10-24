#pragma once
#include "XUtility.h"
#include "GameTimer.h"
#include "MKInput.h"
#include <map>
#include <queue>
#include <functional>
#include <list>
#include "DirectSound.h"
enum {
	REPLAY_UP = 1,		// �ط��ϰ���
	REPLAY_DOWN = 2,	// �ط��°���
	REPLAY_LEFT = 4,	// �ط��󰴼�
	REPLAY_RIGHT = 8,	// �ط��Ұ���
	REPLAY_Z = 16,		// �ط�Z����
	REPLAY_X = 32,		// �ط�X����
	REPLAY_EXIT = 65535	// �˳�
};


struct Command
{
	std::string funcName;	// ������
	std::string funcArgs;	// ��������
};

struct CommandSet {
	int typeID;											// �л�ID
	std::multimap<int, Command> mainCmd;				// ��ָ�
	std::vector<std::multimap<int, Command>> Cmds;		// ��ָ�
};

class EnemyPlane {
public:
	EnemyPlane(int typeID, int HP, int score, int penalty, int minScore,
		float velX, float velY, float posY, float width, float height,
		bool follow, bool inside, bool isRotating, std::string deathSnd, std::string texName);
	// ��ȡID��Ϣ
	int GetTypeID() const { return m_TypeID; }
	// ���ؿ�����ʼ���л�
	void InitAI(CommandSet* commandSet);
	// ���ó�ʼλ��
	void SetPosX(int minPosX, int maxPosX);
	// ���õ���
	void SetItem(int item) { m_Item = (m_TypeID == 3 && item > 0 && item < 4) ? item : 0; }
	// ���µл�
	void Update();
	// ��ȡ֡��
	int GetCurrFrameCnt() const { return m_CurrFrameCnt; }
	// ��ȡ��Ҫ���Ƶ�Ѫ����
	int GetHealthSlices() const { return 100 * m_HP / m_MaxHP; }
	// ��ȡ����/�����ж�����
	RECT GetRect() const;
	// ��ȡ������
	std::string GetTexName() const { return m_TexName; }
	// �ж��Ƿ���
	bool IsAlive() const { return m_Alive; }
	// �ж��Ƿ����ת
	bool IsRotating() const { return m_IsRotating; }
	// �ܵ��˺�
	void Hurt(int val);
private:
	// ��һ�������Ƕȷ����ӵ���angleΪ����Ƕȣ�velΪ�ٶȣ�offsetXY��������λ��
	void Shoot(int offsetX, int offsetY, float vel, const std::vector<int>& angles);
	// ����ҷ���һ�������ӵ���angleΪƫ�ƽǶȣ�velΪ�ٶȣ�offsetXY��������λ��
	void ShootToPlayer(int offsetX, int offsetY, float vel, const std::vector<int>& angles);
	// ���ų���Ƕ�angleƫ��+-offsetAngle�Ƕ����nums���ӵ���velΪ�ٶȣ�offsetXY��������λ��
	void ShootRand(int offsetX, int offsetY, float vel, int angle, int offsetAngle, int nums);
	// �����ƫ��+-offsetAngle�Ƕ����nums���ӵ���velΪ�ٶȣ�offsetXY��������λ��
	void ShootToPlayerRand(int offsetX, int offsetY, float vel, int offsetAngle, int nums);
	// ���伤��
	void ShootLaser(int offsetX, int offsetY, float vel);
	// ���켤��
	void MakeLaserRand(int minX, int maxX, float vel, int nums);
	// ��һ�������Ƕȷ�����棬angleΪ����Ƕȣ�velΪ�ٶȣ�offsetXY��������λ��
	void ShootFlame(int offsetX, int offsetY, float vel, const std::vector<int>& angles);
	// ���ų���Ƕ�angleƫ��+-offsetAngle�Ƕ����nums�����棬velΪ�ٶȣ�offsetXY��������λ��
	void ShootFlameRand(int offsetX, int offsetY, float vel, int angle, int offsetAngle, int nums);
	// �����ƫ��+-offsetAngle�Ƕ����nums�����棬velΪ�ٶȣ�offsetXY��������λ��
	void ShootFlameToPlayerRand(int offsetX, int offsetY, float vel, int offsetAngle, int nums);
	// �������
	void MakeFlameRand(int minX, int maxX, float vel, int nums);
	// �޸��ٶ�X
	void SetVelX(float velX) { m_VelX = velX; }
	// �޸��ٶ�Y
	void SetVelY(float velY) { m_VelY = velY; }
	// �޸��ٶ�X����velX > 0��ʱ������ʱ�ɻ�����Ļ��࣬�ٶ����󣻷�֮����
	void SetAwayVelX(float velX) { m_VelX = (m_PosX <= 375.0 ? -1 : 1) * velX; }
private:
	int m_TypeID;			// ��ʶID
	int m_HP;				// Ѫ��
	int m_MaxHP;			// ���Ѫ��
	int m_Score;			// �ܵ÷�
	int m_Penalty;			// ֡�ͷ�
	int m_MinScore;			// ��͵÷�
	float m_VelX;			// �����ٶ�
	float m_VelY;			// �����ٶ�
	float m_PosX;			// �ɻ����ϽǶ�ӦXλ��
	float m_PosY;			// �ɻ����ϽǶ�ӦYλ��
	int m_Width;			// �ɻ����
	int m_Height;			// �ɻ��߶�
	int m_Item;				// ��ǿ�����ͣ�0Ϊ�ޣ�1ΪBomb��2ΪShield��3ΪPower
	bool m_Follow;			// �Ƿ�׷�����
	bool m_Inside;			// �Ƿ��ܳ��߽�
	bool m_Alive;			// �Ƿ���
	bool m_IsRotating;		// �˶�ʱ�Ƿ���ת
	std::string m_DeathSnd;	// ������Ч
	std::string m_TexName;	// �����ļ���������Ҫ������׺.png��

	CommandSet*	m_pCmd;		// ָ�		
	int m_CurrFrameCnt;		// ��ǰ֡��
	int m_CurrCmd;			// ��ǰִ�еķ�ָ��
};

class UserPlane {
public:
	// ��ʼ���û��ɻ�
	void Initialize(int typeID);
	// ����
	void Respawn();
	// �����ƶ�
	void Move(int X, int Y);
	// ���
	void Shoot();
	// ʹ�ò��ߵ�
	void UseWipe() { m_WipeCnt++; }
	// ��ȡtypeID
	int GetTypeID() const { return m_TypeID; }
	// ��ȡ����/�����ж�����
	RECT GetRect() const;
	// ��ȡ�����ж�����
	RECT GetHitRect() const;
	// ��ȡʣ��������
	int GetLives() const { return m_Lives; }
	// ��ȡʣ��Ӳ����
	int GetCredits() const { return m_Credits; }
	// ��ȡʣ�໤����
	int GetShields() const { return m_Shields; }
	// ��ȡʣ��ը����
	int GetBombs() const { return m_Bombs; }
	// ��ȡ�����е�֡
	int GetHitFrameCnt() const { return m_HitFrameCnt; }
	// ��ȡ�����ٵ�֡
	int GetCrashFrameCnt() const { return m_CrashFrameCnt; }
	// ��ȡ����
	int GetCurrRate() const { return m_CurrRate; }
	//  �õ�Item
	void RecvItem(int typeID);
	// ���·ɻ�
	void Update(MKInput* pIn);
	// ����ը��ʹ��
	void UpdateBomb();
	// ˢ�£����һ�غ�ʹ��
	void Refresh();
	// ��������
	void LiftRate();
	// ����
	void Hurt();

	// �Ƿ���в��ߵ�
	bool HasWipe() const { return m_WipeCnt < m_MaxWipeCnt; }
	// �Ƿ���
	bool IsAlive() const { return m_Alive; }
	// �Ƿ��޵�״̬
	bool IsUndead() const { return m_IsUndead; }
	// �Ƿ��ڷ�ը��
	bool IsUsingBomb() const { return m_UseBomb; }
private:
	int m_TypeID;			// ��ʶID
	int m_Lives;			// ʣ��������
	int m_Credits;			// ʣ��Ӳ��
	int m_FireLevel;		// �����ȼ�
	int m_Bombs;			// ը����
	int m_Shields;			// ������
	float m_Vel;			// �ƶ��ٶ�
	
	bool m_IsUndead;		// ����״̬
	bool m_Alive;			// ���
	bool m_UseBomb;			// ʹ��ը����

	int m_MaxRate;			// �����
	int m_CurrRate;			// ��ǰ����

	int m_WipeCnt;			// ���ߴ���
	int m_MaxWipeCnt;		// �����ߴ���

	int m_HitFrameCnt;		// �����е�ʱ��֡
	int m_BombFrameCnt;		// X�����µ�ʱ��֡
	int m_CrashFrameCnt;	// ��׹��ʱ��֡
	int m_ComboFrameCnt;	// ׷�ٵ�����ʱ��֡
	int m_ShootFrameCnt;	// �����ӵ���ʱ��֡
	int m_ShootSpeed;		// ���֡���
	

	float m_PosX;			// �ɻ����ϽǶ�ӦXλ��
	float m_PosY;			// �ɻ����ϽǶ�ӦYλ��

};

class Bullet
{
public:
	Bullet(float posX, float posY, float vel, float radian, float width, float height);
	void Update();
	void Follow(const RECT& rect);
	RECT GetRect() const;
	RECT GetHitRect() const;
	float GetRotation() const { return m_Rotation; }
	bool IsAlive() const { return m_Alive; }
	bool IsSideWipe() const { return m_SideWipe; }
	void Destroy() { m_Alive = false; }
	void SetWipe() { m_SideWipe = true; }
private:
	float m_PosX;		// �ӵ����Ķ�ӦXλ��		
	float m_PosY;		// �ӵ����Ķ�ӦYλ��
	float m_Vel;		// �ӵ��ٶ�
	float m_Rotation;	// �ӵ���ת
	float m_Width;		// �ӵ����
	float m_Height;		// �ӵ��߶�
	bool m_Alive;		// �ӵ����
	bool m_SideWipe;	// �ӵ�����
};

class Item
{
public:
	Item(float posX, float posY, int type)
		: m_PosX(posX), m_PosY(posY), m_Type(type), m_Alive(true)
	{
	}
	void Update();
	int  GetType() const { return m_Type; }
	RECT GetRect() const;
	bool IsAlive() const { return m_Alive; }
	void Destroy() { m_Alive = false; }
private:
	float m_PosX;	// λ��X
	float m_PosY;	// λ��Y
	int   m_Type;	// ��ǿ������
	bool  m_Alive;	// ���
};

struct PlaneManager {

	static UserPlane player;									// ����ս��				
	static std::list<EnemyPlane> enemyPlanes;					// �з�ս�����������Ļ�ϵģ�
	static std::list<EnemyPlane> enemyPlaneInfo;				// �з�ս����Ϣ
	static std::list<Bullet> enemyBullets;						// �з��ӵ����������Ļ�ϵģ�
	static std::list<Bullet> enemyLasers;						// �з����⣨�������Ļ�ϵģ�
	static std::list<Bullet> enemyFlames;						// �з����棨�������Ļ�ϵģ�
	static std::list<Bullet> playerBullets;						// ����ӵ����������Ļ�ϵģ�
	static std::list<Bullet> playerFlames;						// ��Ҹ��ٵ����������Ļ�ϵģ�
	static std::list<Item> items;								// ����
	static std::vector<CommandSet> enemyCommands;				// �з�ս��ָ�
	static std::multimap<int, std::string> stageInfo;			// ��ǰ�ؿ��Ļһ�����
	
	static int score;											// �÷�
	static int stage;											// ��ǰ�ؿ�

	static bool replayMode;										// �ط�ģʽ
	static bool bossMode;										// bossģʽ
	static bool debugMode;										// debugģʽ

	static bool noMiss;											// �ⳡս��û������
	static bool oneMillion;										// һ����ֽ�һ����
	static bool twoMillion;										// ������ֽ�һ����
	static bool threeMillion;									// ������ֽ�һ����
	static bool fiveMillion;									// �����ֽ�һ����
	static bool eightMillion;									// �˰���ֽ�һ����
	static bool stageComplete;									// ������йؿ�
	static int finalRate;										// ���ձ���
	static int totalFrameCnt;									// ��ǰ��֡
	static int bossFrameCnt;									// ��ǰ�ؿ���boss����ǰռ�õ�֡��
	static int bossKillFrameCnt;								// boss����ɱ��֡��
	static MKInput* pIn;										// ���������豸

	static CSoundManager csound;								// ����������
	static std::map<std::string, CSound*> snd;					// ���
	static int volume;											// ����

	static std::map<int, std::map<unsigned short, unsigned short>> replayInfo;	// �ط�����
	static time_t randomSeed;									// ���������					
	static std::ofstream replayOut;								// ���¼���ļ���

	static void Initialize(HWND hWnd, MKInput* pIn);			// ��ʼ�����������豸
	static void Release();										// �ͷ�����
	static void InitEnemyPlanes();								// ��ʼ���л�
	static void InitStage(int stage);							// ��ʼ���ؿ��л�AI
	static void UpdateScene();									// ���³���������Ǵ����

	static void InitReplay(LPCSTR filePath);					// ��ʼ���ط�									
};