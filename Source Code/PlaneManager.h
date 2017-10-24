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
	REPLAY_UP = 1,		// 回放上按键
	REPLAY_DOWN = 2,	// 回放下按键
	REPLAY_LEFT = 4,	// 回放左按键
	REPLAY_RIGHT = 8,	// 回放右按键
	REPLAY_Z = 16,		// 回放Z按键
	REPLAY_X = 32,		// 回放X按键
	REPLAY_EXIT = 65535	// 退出
};


struct Command
{
	std::string funcName;	// 函数名
	std::string funcArgs;	// 函数参数
};

struct CommandSet {
	int typeID;											// 敌机ID
	std::multimap<int, Command> mainCmd;				// 主指令集
	std::vector<std::multimap<int, Command>> Cmds;		// 分指令集
};

class EnemyPlane {
public:
	EnemyPlane(int typeID, int HP, int score, int penalty, int minScore,
		float velX, float velY, float posY, float width, float height,
		bool follow, bool inside, bool isRotating, std::string deathSnd, std::string texName);
	// 获取ID信息
	int GetTypeID() const { return m_TypeID; }
	// 按关卡来初始化敌机
	void InitAI(CommandSet* commandSet);
	// 设置初始位置
	void SetPosX(int minPosX, int maxPosX);
	// 设置道具
	void SetItem(int item) { m_Item = (m_TypeID == 3 && item > 0 && item < 4) ? item : 0; }
	// 更新敌机
	void Update();
	// 获取帧数
	int GetCurrFrameCnt() const { return m_CurrFrameCnt; }
	// 获取需要绘制的血条数
	int GetHealthSlices() const { return 100 * m_HP / m_MaxHP; }
	// 获取击中/绘制判定矩形
	RECT GetRect() const;
	// 获取纹理名
	std::string GetTexName() const { return m_TexName; }
	// 判断是否存活
	bool IsAlive() const { return m_Alive; }
	// 判断是否会旋转
	bool IsRotating() const { return m_IsRotating; }
	// 受到伤害
	void Hurt(int val);
private:
	// 朝一个或多个角度发射子弹，angle为朝向角度，vel为速度，offsetXY决定初射位置
	void Shoot(int offsetX, int offsetY, float vel, const std::vector<int>& angles);
	// 向玩家发射一个或多个子弹，angle为偏移角度，vel为速度，offsetXY决定初射位置
	void ShootToPlayer(int offsetX, int offsetY, float vel, const std::vector<int>& angles);
	// 沿着朝向角度angle偏移+-offsetAngle角度射出nums发子弹，vel为速度，offsetXY决定初射位置
	void ShootRand(int offsetX, int offsetY, float vel, int angle, int offsetAngle, int nums);
	// 向玩家偏移+-offsetAngle角度射出nums发子弹，vel为速度，offsetXY决定初射位置
	void ShootToPlayerRand(int offsetX, int offsetY, float vel, int offsetAngle, int nums);
	// 发射激光
	void ShootLaser(int offsetX, int offsetY, float vel);
	// 制造激光
	void MakeLaserRand(int minX, int maxX, float vel, int nums);
	// 朝一个或多个角度发射火焰，angle为朝向角度，vel为速度，offsetXY决定初射位置
	void ShootFlame(int offsetX, int offsetY, float vel, const std::vector<int>& angles);
	// 沿着朝向角度angle偏移+-offsetAngle角度射出nums发火焰，vel为速度，offsetXY决定初射位置
	void ShootFlameRand(int offsetX, int offsetY, float vel, int angle, int offsetAngle, int nums);
	// 向玩家偏移+-offsetAngle角度射出nums发火焰，vel为速度，offsetXY决定初射位置
	void ShootFlameToPlayerRand(int offsetX, int offsetY, float vel, int offsetAngle, int nums);
	// 制造火焰
	void MakeFlameRand(int minX, int maxX, float vel, int nums);
	// 修改速度X
	void SetVelX(float velX) { m_VelX = velX; }
	// 修改速度Y
	void SetVelY(float velY) { m_VelY = velY; }
	// 修改速度X，当velX > 0的时候，若此时飞机在屏幕左侧，速度向左；反之向右
	void SetAwayVelX(float velX) { m_VelX = (m_PosX <= 375.0 ? -1 : 1) * velX; }
private:
	int m_TypeID;			// 标识ID
	int m_HP;				// 血量
	int m_MaxHP;			// 最大血量
	int m_Score;			// 总得分
	int m_Penalty;			// 帧惩罚
	int m_MinScore;			// 最低得分
	float m_VelX;			// 横向速度
	float m_VelY;			// 纵向速度
	float m_PosX;			// 飞机左上角对应X位置
	float m_PosY;			// 飞机左上角对应Y位置
	int m_Width;			// 飞机宽度
	int m_Height;			// 飞机高度
	int m_Item;				// 加强包类型：0为无，1为Bomb，2为Shield，3为Power
	bool m_Follow;			// 是否追随玩家
	bool m_Inside;			// 是否不跑出边界
	bool m_Alive;			// 是否存活
	bool m_IsRotating;		// 运动时是否旋转
	std::string m_DeathSnd;	// 死亡音效
	std::string m_TexName;	// 纹理文件名（不需要包含后缀.png）

	CommandSet*	m_pCmd;		// 指令集		
	int m_CurrFrameCnt;		// 当前帧数
	int m_CurrCmd;			// 当前执行的分指令
};

class UserPlane {
public:
	// 初始化用户飞机
	void Initialize(int typeID);
	// 重生
	void Respawn();
	// 设置移动
	void Move(int X, int Y);
	// 射击
	void Shoot();
	// 使用擦边弹
	void UseWipe() { m_WipeCnt++; }
	// 获取typeID
	int GetTypeID() const { return m_TypeID; }
	// 获取绘制/擦边判定矩阵
	RECT GetRect() const;
	// 获取击中判定矩阵
	RECT GetHitRect() const;
	// 获取剩余生命数
	int GetLives() const { return m_Lives; }
	// 获取剩余硬币数
	int GetCredits() const { return m_Credits; }
	// 获取剩余护盾数
	int GetShields() const { return m_Shields; }
	// 获取剩余炸弹数
	int GetBombs() const { return m_Bombs; }
	// 获取被击中的帧
	int GetHitFrameCnt() const { return m_HitFrameCnt; }
	// 获取被击毁的帧
	int GetCrashFrameCnt() const { return m_CrashFrameCnt; }
	// 获取倍率
	int GetCurrRate() const { return m_CurrRate; }
	//  得到Item
	void RecvItem(int typeID);
	// 更新飞机
	void Update(MKInput* pIn);
	// 更新炸弹使用
	void UpdateBomb();
	// 刷新，完成一关后使用
	void Refresh();
	// 提升倍率
	void LiftRate();
	// 受伤
	void Hurt();

	// 是否存有擦边弹
	bool HasWipe() const { return m_WipeCnt < m_MaxWipeCnt; }
	// 是否存活
	bool IsAlive() const { return m_Alive; }
	// 是否无敌状态
	bool IsUndead() const { return m_IsUndead; }
	// 是否在放炸弹
	bool IsUsingBomb() const { return m_UseBomb; }
private:
	int m_TypeID;			// 标识ID
	int m_Lives;			// 剩余生命数
	int m_Credits;			// 剩余硬币
	int m_FireLevel;		// 火力等级
	int m_Bombs;			// 炸弹数
	int m_Shields;			// 护盾数
	float m_Vel;			// 移动速度
	
	bool m_IsUndead;		// 不死状态
	bool m_Alive;			// 存活
	bool m_UseBomb;			// 使用炸弹中

	int m_MaxRate;			// 最大倍率
	int m_CurrRate;			// 当前倍率

	int m_WipeCnt;			// 擦边次数
	int m_MaxWipeCnt;		// 最大擦边次数

	int m_HitFrameCnt;		// 被击中的时间帧
	int m_BombFrameCnt;		// X键按下的时间帧
	int m_CrashFrameCnt;	// 击坠的时间帧
	int m_ComboFrameCnt;	// 追踪弹连击时间帧
	int m_ShootFrameCnt;	// 发射子弹的时间帧
	int m_ShootSpeed;		// 射击帧间隔
	

	float m_PosX;			// 飞机左上角对应X位置
	float m_PosY;			// 飞机左上角对应Y位置

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
	float m_PosX;		// 子弹中心对应X位置		
	float m_PosY;		// 子弹中心对应Y位置
	float m_Vel;		// 子弹速度
	float m_Rotation;	// 子弹旋转
	float m_Width;		// 子弹宽度
	float m_Height;		// 子弹高度
	bool m_Alive;		// 子弹存活
	bool m_SideWipe;	// 子弹擦边
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
	float m_PosX;	// 位置X
	float m_PosY;	// 位置Y
	int   m_Type;	// 加强包类型
	bool  m_Alive;	// 存活
};

struct PlaneManager {

	static UserPlane player;									// 己方战机				
	static std::list<EnemyPlane> enemyPlanes;					// 敌方战机（存活在屏幕上的）
	static std::list<EnemyPlane> enemyPlaneInfo;				// 敌方战机信息
	static std::list<Bullet> enemyBullets;						// 敌方子弹（存活在屏幕上的）
	static std::list<Bullet> enemyLasers;						// 敌方激光（存活在屏幕上的）
	static std::list<Bullet> enemyFlames;						// 敌方火焰（存活在屏幕上的）
	static std::list<Bullet> playerBullets;						// 玩家子弹（存活在屏幕上的）
	static std::list<Bullet> playerFlames;						// 玩家跟踪弹（存活在屏幕上的）
	static std::list<Item> items;								// 物体
	static std::vector<CommandSet> enemyCommands;				// 敌方战机指令集
	static std::multimap<int, std::string> stageInfo;			// 当前关卡的灰机控制
	
	static int score;											// 得分
	static int stage;											// 当前关卡

	static bool replayMode;										// 回放模式
	static bool bossMode;										// boss模式
	static bool debugMode;										// debug模式

	static bool noMiss;											// 这场战斗没有死亡
	static bool oneMillion;										// 一百万分奖一条命
	static bool twoMillion;										// 两百万分奖一条命
	static bool threeMillion;									// 三百万分奖一条命
	static bool fiveMillion;									// 五百万分奖一条命
	static bool eightMillion;									// 八百万分奖一条命
	static bool stageComplete;									// 完成所有关卡
	static int finalRate;										// 最终倍率
	static int totalFrameCnt;									// 当前总帧
	static int bossFrameCnt;									// 当前关卡到boss出场前占用的帧数
	static int bossKillFrameCnt;								// boss被击杀的帧数
	static MKInput* pIn;										// 键鼠输入设备

	static CSoundManager csound;								// 声音管理者
	static std::map<std::string, CSound*> snd;					// 射击
	static int volume;											// 音量

	static std::map<int, std::map<unsigned short, unsigned short>> replayInfo;	// 回放内容
	static time_t randomSeed;									// 随机数种子					
	static std::ofstream replayOut;								// 输出录像文件流

	static void Initialize(HWND hWnd, MKInput* pIn);			// 初始化键鼠、声音设备
	static void Release();										// 释放声音
	static void InitEnemyPlanes();								// 初始化敌机
	static void InitStage(int stage);							// 初始化关卡敌机AI
	static void UpdateScene();									// 更新场景，清除非存活物

	static void InitReplay(LPCSTR filePath);					// 初始化回放									
};