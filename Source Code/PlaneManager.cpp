#include "PlaneManager.h"

UserPlane PlaneManager::player;
std::list<EnemyPlane> PlaneManager::enemyPlanes;
std::list<EnemyPlane> PlaneManager::enemyPlaneInfo;
std::list<Bullet> PlaneManager::enemyBullets;
std::list<Bullet> PlaneManager::enemyLasers;
std::list<Bullet> PlaneManager::enemyFlames;
std::list<Bullet> PlaneManager::playerBullets;
std::list<Bullet> PlaneManager::playerFlames;
std::list<Item> PlaneManager::items;
std::vector<CommandSet> PlaneManager::enemyCommands;
std::multimap<int, std::string> PlaneManager::stageInfo;
MKInput* PlaneManager::pIn = nullptr;
int PlaneManager::score = 0;
int PlaneManager::stage = 1;
int PlaneManager::finalRate = 1;
int PlaneManager::totalFrameCnt = 0;
int PlaneManager::bossFrameCnt = 0;
int PlaneManager::bossKillFrameCnt = 0;
bool PlaneManager::noMiss = true;
bool PlaneManager::bossMode = false;
bool PlaneManager::debugMode = true;
bool PlaneManager::oneMillion = true;									
bool PlaneManager::twoMillion = true;									
bool PlaneManager::threeMillion = true;								
bool PlaneManager::fiveMillion = true;								
bool PlaneManager::eightMillion = true;		
bool PlaneManager::stageComplete = false;

std::ofstream PlaneManager::replayOut;
bool PlaneManager::replayMode = false;
time_t PlaneManager::randomSeed = 0;
std::map<int, std::map<unsigned short, unsigned short>> PlaneManager::replayInfo;

CSoundManager PlaneManager::csound;
std::map<std::string, CSound*>  PlaneManager::snd;
int PlaneManager::volume = 5;

EnemyPlane::EnemyPlane(int typeID, int HP, int score, int penalty, int minScore, float velX, float velY,
	 float posY, float width, float height, bool follow, bool inside, bool isRotating, std::string deathSnd, std::string texName)
	: m_TypeID(typeID), m_HP(HP), m_MaxHP(HP), m_Score(score), m_Penalty(penalty), m_MinScore(minScore),
	m_VelX(velX), m_VelY(velY), m_PosX(0.0f), m_PosY(posY), m_Width(width), m_Height(height), m_Alive(true),
	m_CurrFrameCnt(0), m_CurrCmd(0), m_Follow(follow), m_Inside(inside), m_IsRotating(isRotating), 
	m_DeathSnd(deathSnd), m_TexName(texName)
{
}

void EnemyPlane::InitAI(CommandSet* commandSet)
{
	m_pCmd = commandSet;
}

void EnemyPlane::SetPosX(int minPosX, int maxPosX)
{
	if (minPosX == maxPosX)
	{
		m_PosX = minPosX;
	}
	else
	{
		m_PosX = minPosX + rand() % (maxPosX - minPosX);
	}
}

void EnemyPlane::Update()
{
	// �ж����
	if (m_HP <= 0)
	{
		m_Alive = false;
		// ������Ч����
		PlaneManager::snd[m_DeathSnd]->Play(0UL, 0UL, LevelToVolume(PlaneManager::volume));
		PlaneManager::score += m_Score;
		// ��IDΪ3������λ������һ��Item
		if (m_TypeID == 3)
		{
			PlaneManager::items.emplace_front(m_PosX + m_Width / 2,
				m_PosY + m_Height / 2, m_Item);
		}
		return;
	}
	else if (m_PosX < -m_Width || m_PosX > 750 || m_PosY > 900 || (m_PosY < -m_Height && m_VelY < 0.0f))
	{
		m_Alive = false;
		return;
	}

	// �ȼ����ָ��
	auto BegEnd = m_pCmd->mainCmd.equal_range(m_CurrFrameCnt);
	auto Beg = BegEnd.first, End = BegEnd.second;
	for (; Beg != End; ++Beg)
	{
		std::string& funcName = Beg->second.funcName;
		std::istringstream iss(Beg->second.funcArgs);
		int arg;
		if (funcName == "SetCmd")
		{
			// ��ѡָ�
			std::vector<int> nums;
			while (iss >> arg)
				nums.push_back(arg);
			// �������һ��ָ��
			m_CurrCmd = nums[rand() % nums.size()];

		}
		else if (funcName == "SetFrame")
		{
			iss >> m_CurrFrameCnt;
		}
		else if (funcName == "SetFrameIfHPLess")
		{
			// ��Ѫ��С�ڵ���HP����ת��ĳFrameVal����յ�Ļ
			iss >> arg;
			if (m_HP <= arg)
			{
				iss >> m_CurrFrameCnt;
				PlaneManager::snd["BLAST3"]->Play(0UL, 0UL, LevelToVolume(PlaneManager::volume));
				PlaneManager::enemyFlames.clear();
				PlaneManager::enemyBullets.clear();
				PlaneManager::enemyLasers.clear();
			}
				
		}
	}

	// Ȼ�����ָ��
	auto BegEnd2 = m_pCmd->Cmds[m_CurrCmd].equal_range(m_CurrFrameCnt);
	auto Beg2 = BegEnd2.first, End2 = BegEnd2.second;
	for (; Beg2 != End2; ++Beg2)
	{
		std::string& funcName = Beg2->second.funcName;
		std::istringstream iss(Beg2->second.funcArgs);
		if (funcName == "SetVelX")
		{
			float velX;
			iss >> velX;
			SetVelX(velX);
		}
		else if (funcName == "SetVelY")
		{
			float velY;
			iss >> velY;
			SetVelY(velY);
		}
		else if (funcName == "SetAwayVelX")
		{
			float velX;
			iss >> velX;
			SetAwayVelX(velX);
		}
		else if (funcName == "Shoot" && !PlaneManager::player.GetCrashFrameCnt())
		{
			int offsetX, offsetY;
			float vel, angle;
			iss >> offsetX >> offsetY >> vel;
			std::vector<int> angles;
			while (iss >> angle)
				angles.push_back(angle);
			Shoot(offsetX, offsetY, vel, angles);
		}
		else if (funcName == "ShootToPlayer" && !PlaneManager::player.GetCrashFrameCnt())
		{
			int offsetX, offsetY;
			float vel, angle;
			iss >> offsetX >> offsetY >> vel;
			std::vector<int> angles;
			while (iss >> angle)
				angles.push_back(angle);
			ShootToPlayer(offsetX, offsetY, vel, angles);
		}
		else if (funcName == "ShootRand" && !PlaneManager::player.GetCrashFrameCnt())
		{
			int offsetX, offsetY, nums;
			float vel, angle, offsetAngle;
			iss >> offsetX >> offsetY >> vel >> angle >> offsetAngle >> nums;
			ShootRand(offsetX, offsetY, vel, angle, offsetAngle, nums);
		}
		else if (funcName == "ShootToPlayerRand" && !PlaneManager::player.GetCrashFrameCnt())
		{
			int offsetX, offsetY, nums;
			float vel, offsetAngle;
			iss >> offsetX >> offsetY >> vel >> offsetAngle >> nums;
			ShootToPlayerRand(offsetX, offsetY, vel, offsetAngle, nums);
		}
		else if (funcName == "ShootLaser" && !PlaneManager::player.GetCrashFrameCnt())
		{
			int offsetX, offsetY;
			float vel;
			iss >> offsetX >> offsetY >> vel;
			ShootLaser(offsetX, offsetY, vel);
		}
		else if (funcName == "MakeLaserRand" && !PlaneManager::player.GetCrashFrameCnt())
		{
			int minX, maxX, nums;
			float vel;
			iss >> minX >> maxX >> vel >> nums;
			MakeLaserRand(minX, maxX, vel, nums);
		}
		else if (funcName == "ShootFlame" && !PlaneManager::player.GetCrashFrameCnt())
		{
			int offsetX, offsetY;
			float vel, angle;
			iss >> offsetX >> offsetY >> vel;
			std::vector<int> angles;
			while (iss >> angle)
				angles.push_back(angle);
			ShootFlame(offsetX, offsetY, vel, angles);
		}
		else if (funcName == "ShootFlameRand" && !PlaneManager::player.GetCrashFrameCnt())
		{
			int offsetX, offsetY, nums;
			float vel, angle, offsetAngle;
			iss >> offsetX >> offsetY >> vel >> angle >> offsetAngle >> nums;
			ShootFlameRand(offsetX, offsetY, vel, angle, offsetAngle, nums);
		}
		else if (funcName == "ShootFlameToPlayerRand" && !PlaneManager::player.GetCrashFrameCnt())
		{
			int offsetX, offsetY, nums;
			float vel, offsetAngle;
			iss >> offsetX >> offsetY >> vel >> offsetAngle >> nums;
			ShootFlameToPlayerRand(offsetX, offsetY, vel, offsetAngle, nums);
		}
		else if (funcName == "MakeFlameRand" && !PlaneManager::player.GetCrashFrameCnt())
		{
			int minX, maxX, nums;
			float vel;
			iss >> minX >> maxX >> vel >> nums;
			MakeFlameRand(minX, maxX, vel, nums);
		}
		else if (funcName == "Charge")
			PlaneManager::snd["CHARGE"]->Play(0UL, 0UL, LevelToVolume(PlaneManager::volume));
	}

	// �Ƿ����������
	if (m_Follow)
	{
		RECT rc = PlaneManager::player.GetRect();
		if (m_PosX + m_Width / 2 - 50.0f > (rc.right + rc.left) / 2.0f && m_VelX > 0.0f)
			m_VelX *= -1;
		else if (m_PosX + m_Width / 2 + 50.0f < (rc.right + rc.left) / 2.0f && m_VelX < 0.0f)
			m_VelX *= -1;
	}

	// �Ƿ��ܳ��߽�
	if (m_Inside)
	{
		if (m_PosX < 50.0f)
		{
			m_PosX = 50.0f;
			m_VelX = fabs(m_VelX);
		}
		else if (m_PosX > 700.0f - m_Width)
		{
			m_PosX = 700.0f - m_Width;
			m_VelX = -fabs(m_VelX);
		}
	}

	// ���µл�
	m_PosX += m_VelX;
	m_PosY += m_VelY;
	// ÿ�θ��¶��ή�͵÷�
	if (m_Score > m_MinScore)
		m_Score -= m_Penalty;
	m_CurrFrameCnt++;
}

RECT EnemyPlane::GetRect() const
{
	return RECT{
		static_cast<LONG>(m_PosX),
		static_cast<LONG>(m_PosY),
		static_cast<LONG>(m_PosX + m_Width),
		static_cast<LONG>(m_PosY + m_Height),
	};
}

void EnemyPlane::Hurt(int val)
{
	m_HP -= val;
}

void EnemyPlane::Shoot(int offsetX, int offsetY, float vel, const std::vector<int>& angles)
{
	RECT rc = GetRect();
	for (auto angle : angles)
	{
		PlaneManager::enemyBullets.emplace_front(rc.left + offsetX, rc.top + offsetY,
			vel, DegToRad(angle), 16, 16);
	}
}

void EnemyPlane::ShootToPlayer(int offsetX, int offsetY, float vel, const std::vector<int>& angles)
{
	RECT rc = GetRect();
	RECT urc = PlaneManager::player.GetHitRect();
	float shootX = (float)rc.left + offsetX;
	float shootY = (float)rc.top + offsetY;
	float targetX = (urc.left + urc.right) / 2.0f;
	float targetY = (urc.bottom + urc.top) / 2.0f;
	float rad = atan2f(targetY - shootY, targetX - shootX);
	for (auto angle : angles)
	{
		PlaneManager::enemyBullets.emplace_front(shootX, shootY,
			vel, rad + DegToRad(angle), 16, 16);
	}
}

void EnemyPlane::ShootRand(int offsetX, int offsetY, float vel, int angle, int offsetAngle, int nums)
{
	RECT rc = GetRect();
	for (int i = 0; i < nums; ++i)
	{
		int randAngle = rand() % (2 * offsetAngle + 1) - offsetAngle;
		PlaneManager::enemyBullets.emplace_front(rc.left + offsetX, rc.top + offsetY,
			vel, DegToRad(angle + randAngle),
			16, 16);
	}
}

void EnemyPlane::ShootToPlayerRand(int offsetX, int offsetY, float vel, int offsetAngle, int nums)
{
	RECT rc = GetRect();
	RECT urc = PlaneManager::player.GetHitRect();
	float shootX = rc.left + offsetX;
	float shootY = rc.top + offsetY;
	float targetX = (urc.left + urc.right) / 2.0f;
	float targetY = (urc.bottom + urc.top) / 2.0f;
	float rad = atan2f(targetY - shootY, targetX - shootX);
	for (int i = 0; i < nums; ++i)
	{
		int randAngle = rand() % (2 * offsetAngle + 1) - offsetAngle;
		PlaneManager::enemyBullets.emplace_front(shootX, shootY,
			vel, rad + DegToRad(randAngle), 16, 16);
	}
}

void EnemyPlane::ShootLaser(int offsetX, int offsetY, float vel)
{
	RECT rc = GetRect();
	PlaneManager::snd["LASER"]->Play(0UL, 0UL, LevelToVolume(PlaneManager::volume));
	PlaneManager::enemyLasers.emplace_front(rc.left + offsetX,
		rc.top + offsetY, vel, MathHelper::Pi / 2, 100, 10);
}

void EnemyPlane::MakeLaserRand(int minX, int maxX, float vel, int nums)
{
	
	PlaneManager::snd["LASER"]->Play(0UL, 0UL, LevelToVolume(PlaneManager::volume));
	for (int i = 0; i < nums; ++i)
	{
		int posX = minX + rand() % (maxX - minX);
		PlaneManager::enemyLasers.emplace_front(posX, 0, vel, MathHelper::Pi / 2, 100, 10);
	}
	
}

void EnemyPlane::ShootFlame(int offsetX, int offsetY, float vel, const std::vector<int>& angles)
{
	PlaneManager::snd["VULCAN"]->Play(0UL, 0UL, LevelToVolume(PlaneManager::volume));
	RECT rc = GetRect();
	for (auto angle : angles)
	{
		PlaneManager::enemyFlames.emplace_front(rc.left + offsetX, rc.top + offsetY,
			vel, DegToRad(angle), 60, 12);
	}
}

void EnemyPlane::ShootFlameRand(int offsetX, int offsetY, float vel, int angle, int offsetAngle, int nums)
{
	PlaneManager::snd["VULCAN"]->Play(0UL, 0UL, LevelToVolume(PlaneManager::volume));
	RECT rc = GetRect();
	for (int i = 0; i < nums; ++i)
	{
		int randAngle = rand() % (2 * offsetAngle + 1) - offsetAngle;
		PlaneManager::enemyFlames.emplace_front(rc.left + offsetX, rc.top + offsetY,
			vel, DegToRad(angle + randAngle),
			60, 12);
	}
}

void EnemyPlane::ShootFlameToPlayerRand(int offsetX, int offsetY, float vel, int offsetAngle, int nums)
{
	PlaneManager::snd["VULCAN"]->Play(0UL, 0UL, LevelToVolume(PlaneManager::volume));
	RECT rc = GetRect();
	RECT urc = PlaneManager::player.GetHitRect();
	float shootX = rc.left + offsetX;
	float shootY = rc.top + offsetY;
	float targetX = (urc.left + urc.right) / 2.0f;
	float targetY = (urc.bottom + urc.top) / 2.0f;
	float rad = atan2f(targetY - shootY, targetX - shootX);
	for (int i = 0; i < nums; ++i)
	{
		int randAngle = rand() % (2 * offsetAngle + 1) - offsetAngle;
		PlaneManager::enemyFlames.emplace_front(shootX, shootY,
			vel, rad + DegToRad(randAngle), 60, 12);
	}
}

void EnemyPlane::MakeFlameRand(int minX, int maxX, float vel, int nums)
{
	PlaneManager::snd["VULCAN"]->Play(0UL, 0UL, LevelToVolume(PlaneManager::volume));
	for (int i = 0; i < nums; ++i)
	{
		int posX = minX + rand() % (maxX - minX);
		PlaneManager::enemyFlames.emplace_front(posX, 0, vel, MathHelper::Pi / 2, 60, 12);
	}
}










void UserPlane::Initialize(int typeID)
{
	if (typeID == 1)
	{	
		m_ShootSpeed = 6;
		m_Vel = 6.0f;
		m_MaxRate = 12;
		m_MaxWipeCnt = 12;
	}
	else if (typeID == 2)
	{
		m_ShootSpeed = 5;
		m_Vel = 8.0f;
		m_MaxRate = 16;
		m_MaxWipeCnt = 16;
	}
	m_TypeID = typeID;

	m_PosX = 335.0f;
	m_PosY = 750.0f;
	m_Lives = 4;
	m_Credits = 4;
	m_FireLevel = 1;
	m_Bombs = 3;
	m_Shields = 0;
	m_IsUndead = false;
	m_UseBomb = false;

	m_ShootFrameCnt = 0;
	m_HitFrameCnt = 0;
	m_CrashFrameCnt = 0;
	m_ComboFrameCnt = 0;
	m_BombFrameCnt = 0;

	m_WipeCnt = 0;

	m_CurrRate = 1;
	m_Alive = true;

	PlaneManager::oneMillion = true;
	PlaneManager::twoMillion = true;
	PlaneManager::threeMillion = true;
	PlaneManager::fiveMillion = true;
	PlaneManager::eightMillion = true;
}

void UserPlane::Respawn()
{
	m_Lives = 5;
	m_Credits--;
	m_IsUndead = true;
	m_UseBomb = false;

	m_ShootFrameCnt = 0;
	m_HitFrameCnt = 0;
	m_CrashFrameCnt = PlaneManager::totalFrameCnt - 119;
	m_ComboFrameCnt = 0;

	PlaneManager::items.emplace_front(300.0f, 200.0f, 1);
	PlaneManager::items.emplace_front(400.0f, 200.0f, 2);
	PlaneManager::items.emplace_front(500.0f, 200.0f, 1);
}

void UserPlane::Move(int X, int Y)
{
	m_PosX += X * m_Vel;
	m_PosY += Y * m_Vel;
	if (m_PosX < 0.0f)
		m_PosX = 0.0f;
	else if (m_PosX > 670.0f)
		m_PosX = 670.0f;

	if (m_PosY < 0.0f)
		m_PosY = 0.0f;
	else if (m_PosY > 800.0f)
		m_PosY = 800.0f;
}

void UserPlane::Shoot()
{
	static int shootCnt = 0;
	shootCnt++;
	if (m_TypeID == 1)
	{
		PlaneManager::snd["SHOT"]->Play(0UL, 0UL, LevelToVolume(PlaneManager::volume));
		auto shoot = [&](float angle) {
			PlaneManager::playerBullets.emplace_front(m_PosX + 40.0f, m_PosY + 0.0f, 18.0f, angle, 60, 12);
		};
		if (m_FireLevel == 1)
		{
			if (shootCnt % 2 == 0)
			{
				shoot(DegToRad(-86.0f));
				shoot(DegToRad(-94.0f));
			}
			else
			{
				shoot(DegToRad(-80.0f));
				shoot(DegToRad(-90.0f));
				shoot(DegToRad(-100.0f));
			}	
		}
		else if (m_FireLevel == 2)
		{
			if (shootCnt % 2 == 0)
			{
				shoot(DegToRad(-78.0f));
				shoot(DegToRad(-86.0f));
				shoot(DegToRad(-94.0f));
				shoot(DegToRad(-102.0f));
			}
			else
			{
				shoot(DegToRad(-80.0f));
				shoot(DegToRad(-90.0f));
				shoot(DegToRad(-100.0f));
			}
		}
		else if (m_FireLevel == 3)
		{
			if (shootCnt % 2 == 0)
			{
				shoot(DegToRad(-90.0f));
				shoot(DegToRad(-78.0f));
				shoot(DegToRad(-72.0f));
				shoot(DegToRad(-102.0f));
				shoot(DegToRad(-108.0f));
			}
			else
			{
				shoot(DegToRad(-75.0f));
				shoot(DegToRad(-84.0f));
				shoot(DegToRad(-96.0f));
				shoot(DegToRad(-105.0f));
			}
				
		}
	}
	else if (m_TypeID == 2)
	{
		PlaneManager::snd["SHOT2"]->Play(0UL, 0UL, LevelToVolume(PlaneManager::volume));
		auto shoot = [&](float pos) {
			PlaneManager::playerBullets.emplace_front(m_PosX + pos, m_PosY + 0.0f, 
				21.0f, DegToRad(-90.0f), 60, 12);
		};
		if (m_FireLevel == 1)
		{
				shoot(30.0f);
				shoot(50.0f);
		}
		else if (m_FireLevel == 2)
		{
				shoot(20.0f);
				shoot(40.0f);
				shoot(60.0f);
		}
		else if (m_FireLevel == 3)
		{
			if (shootCnt % 2 == 0)
			{
				shoot(10.0f);
				shoot(30.0f);
				shoot(50.0f);
				shoot(70.0f);
			}
			else
			{
				shoot(20.0f);
				shoot(40.0f);
				shoot(60.0f);
			}
		}
	}
}

RECT UserPlane::GetRect() const
{
	return RECT{
		static_cast<LONG>(m_PosX),
		static_cast<LONG>(m_PosY),
		static_cast<LONG>(m_PosX + 80),
		static_cast<LONG>(m_PosY + 100),
	};
	
}

RECT UserPlane::GetHitRect() const
{
	return RECT{
		static_cast<LONG>(m_PosX + 38),
		static_cast<LONG>(m_PosY + 48),
		static_cast<LONG>(m_PosX + 42),
		static_cast<LONG>(m_PosY + 52),
	};
}

void UserPlane::RecvItem(int typeID)
{
	// Bomb
	if (typeID == 1)
	{
		PlaneManager::snd["ITEM1"]->Play(0UL, 0UL, LevelToVolume(PlaneManager::volume));
		if (m_Bombs < 5)
			m_Bombs++;
		else
			PlaneManager::score += 30000;
	}
	// Power
	else if (typeID == 2)
	{
		PlaneManager::snd["ITEM0"]->Play(0UL, 0UL, LevelToVolume(PlaneManager::volume));
		if (m_FireLevel < 3)
			m_FireLevel++;
		else
			PlaneManager::score += 30000;
	}
	// Shield
	else if (typeID == 3)
	{
		PlaneManager::snd["ITEM2"]->Play(0UL, 0UL, LevelToVolume(PlaneManager::volume));
		if (!m_Shields)
			m_Shields = 3;
		else if (m_Shields < 3)
			m_Shields++;
		else
			PlaneManager::score += 30000;
	}
}

void UserPlane::Update(MKInput * pIn)
{
	// ����ʱ��2s������
	if (m_CrashFrameCnt && m_Lives && PlaneManager::totalFrameCnt - m_CrashFrameCnt == 120)
	{
		m_Alive = true;
		m_IsUndead = true;
		m_PosX = 335.0f;
		m_PosY = 750.0f;
		m_Lives--;
		m_Bombs = 3;
		m_Shields = 0;
		m_FireLevel = 1;
		m_CurrRate = 1;
		m_WipeCnt = 0;
		m_HitFrameCnt = 0;
		// ��������Item��һ��Power��һ��Shield
		PlaneManager::items.emplace_front(200.0f, 200.0f, 3);
		PlaneManager::items.emplace_front(600.0f, 200.0f, 2);
	}
	// ���ֺ�з�2s�ڲ������ӵ�
	if (m_CrashFrameCnt && PlaneManager::totalFrameCnt - m_CrashFrameCnt >= 240)
	{
		m_CrashFrameCnt = 0;
		m_IsUndead = false;
	}
	// �����к����2s�޵�
	if (m_HitFrameCnt && PlaneManager::totalFrameCnt - m_HitFrameCnt >= 120)
	{
		m_HitFrameCnt = 0;
		m_IsUndead = false;
	}
	// ÿ5s�ָ����в��ߴ���
	if (PlaneManager::totalFrameCnt % 300 == 0)
		m_WipeCnt = 0;
	// ���¼�����Ӧ�¼�
	if (m_Alive)
	{
		// ����ǻط�ģʽ
		if (PlaneManager::replayMode)
		{
			auto& info = PlaneManager::replayInfo[PlaneManager::stage];
			auto it = info.find(PlaneManager::totalFrameCnt);
			if (it != info.end())
			{
				if (it->second & REPLAY_UP)
					Move(0, -1);
				if (it->second & REPLAY_DOWN)
					Move(0, 1);
				if (it->second & REPLAY_LEFT)
					Move(-1, 0);
				if (it->second & REPLAY_RIGHT)
					Move(1, 0);
				// �������
				if (!m_BombFrameCnt && (it->second & REPLAY_Z) &&
					PlaneManager::totalFrameCnt - m_ShootFrameCnt > m_ShootSpeed)
				{
					Shoot();
					m_ShootFrameCnt = PlaneManager::totalFrameCnt;
				}
				// ʹ��ը��
				if (!m_BombFrameCnt && (it->second & REPLAY_X) && m_Bombs)
				{
					m_BombFrameCnt = PlaneManager::totalFrameCnt;
					m_Bombs--;
					m_IsUndead = m_UseBomb = true;
				}
				info.erase(it);
			}
		}
		else
		{
			unsigned short record[2]{};		//֡��¼
			if (pIn->Key_Down(DIK_UPARROW) || pIn->XBoxHandle_ThumbLY() > 10000 || pIn->XBoxHandle_ButtonDown(XINPUT_GAMEPAD_DPAD_UP))
			{
				Move(0, -1);
				record[1] |= REPLAY_UP;
			}
			if (pIn->Key_Down(DIK_DOWNARROW) || pIn->XBoxHandle_ThumbLY() < -10000 || pIn->XBoxHandle_ButtonDown(XINPUT_GAMEPAD_DPAD_DOWN))
			{
				Move(0, 1);
				record[1] |= REPLAY_DOWN;
			}
			if (pIn->Key_Down(DIK_LEFTARROW) || pIn->XBoxHandle_ThumbLX() < -10000 || pIn->XBoxHandle_ButtonDown(XINPUT_GAMEPAD_DPAD_LEFT))
			{
				Move(-1, 0);
				record[1] |= REPLAY_LEFT;
			}
			if (pIn->Key_Down(DIK_RIGHTARROW) || pIn->XBoxHandle_ThumbLX() > 10000 || pIn->XBoxHandle_ButtonDown(XINPUT_GAMEPAD_DPAD_RIGHT))
			{
				Move(1, 0);
				record[1] |= REPLAY_RIGHT;
			}

			// �������
			if (!m_BombFrameCnt && (pIn->Key_Down(DIK_Z) || pIn->XBoxHandle_ButtonDown(XINPUT_GAMEPAD_X)) &&
				PlaneManager::totalFrameCnt - m_ShootFrameCnt > m_ShootSpeed)
			{
				Shoot();
				m_ShootFrameCnt = PlaneManager::totalFrameCnt;
				record[1] |= REPLAY_Z;
			}
			// ʹ��ը��
			if (!m_BombFrameCnt && (pIn->Key_Down(DIK_X) || pIn->XBoxHandle_ButtonDown(XINPUT_GAMEPAD_A)) && m_Bombs)
			{
				m_BombFrameCnt = PlaneManager::totalFrameCnt;
				m_Bombs--;
				m_IsUndead = m_UseBomb = true;
				record[1] |= REPLAY_X;
			}

			// �����һ֡�ж�����д��¼���ļ�
			if (record[1])
			{
				record[0] = (unsigned short)PlaneManager::totalFrameCnt;
				PlaneManager::replayOut.write(reinterpret_cast<char *>(&record), sizeof(record));
			}
		}
		
		

		
	}
	// ����ը��
	UpdateBomb();

	// �������Ƿ�ﵽ����Ҫ��
	if (PlaneManager::oneMillion && PlaneManager::score >= 1000000)
	{
		PlaneManager::snd["1UP"]->Play(0UL, 0UL, LevelToVolume(PlaneManager::volume));
		PlaneManager::oneMillion = false;
		m_Lives++;
	}
	if (PlaneManager::twoMillion && PlaneManager::score >= 2000000)
	{
		PlaneManager::snd["1UP"]->Play(0UL, 0UL, LevelToVolume(PlaneManager::volume));
		PlaneManager::twoMillion = false;
		m_Lives++;
	}
	if (PlaneManager::threeMillion && PlaneManager::score >= 3000000)
	{
		PlaneManager::snd["1UP"]->Play(0UL, 0UL, LevelToVolume(PlaneManager::volume));
		PlaneManager::threeMillion = false;
		m_Lives++;
	}
	if (PlaneManager::fiveMillion && PlaneManager::score >= 5000000)
	{
		PlaneManager::snd["1UP"]->Play(0UL, 0UL, LevelToVolume(PlaneManager::volume));
		PlaneManager::fiveMillion = false;
		m_Lives++;
	}
	if (PlaneManager::eightMillion && PlaneManager::score >= 8000000)
	{
		PlaneManager::snd["1UP"]->Play(0UL, 0UL, LevelToVolume(PlaneManager::volume));
		PlaneManager::eightMillion = false;
		m_Lives++;
	}

	// ��ʱ��(3s)û�б��ʹ��ף�����һ�뱶��
	if (m_CurrRate > 1 && (PlaneManager::totalFrameCnt - m_ComboFrameCnt) > 180)
	{
		m_CurrRate /= 2;
		m_ComboFrameCnt = PlaneManager::totalFrameCnt;
	}
}

void UserPlane::UpdateBomb()
{
	// ը������3s
	if (m_BombFrameCnt && PlaneManager::totalFrameCnt - m_BombFrameCnt > 180)
	{
		m_BombFrameCnt = 0;
		m_IsUndead = m_UseBomb = false;
		return;
	}

	if (m_BombFrameCnt)
	{
		m_CurrRate = 1;	// ����ǿ�ƽ���1
		if (m_TypeID == 1 && PlaneManager::totalFrameCnt - m_ShootFrameCnt > m_ShootSpeed)
		{
			PlaneManager::snd["BOMBER1"]->Play(0UL, 0UL, LevelToVolume(PlaneManager::volume));
			auto shoot = [&](float angle) {
				PlaneManager::playerBullets.emplace_front(m_PosX + 40.0f, m_PosY + 0.0f,
					18.0f, angle, 60, 12);
			};
			for (int i = 0; i < 7; ++i)
				shoot(DegToRad(-120.0f + i * 10.0f));
			m_ShootFrameCnt = PlaneManager::totalFrameCnt;
		}
		else if (m_TypeID == 2 && PlaneManager::totalFrameCnt - m_ShootFrameCnt > m_ShootSpeed)
		{
			PlaneManager::snd["CWS"]->Play(0UL, 0UL, LevelToVolume(PlaneManager::volume));
			RECT playerRect = GetRect();
			PlaneManager::playerFlames.emplace_front(40.0f + playerRect.left, 100.0f + playerRect.top,
				36.0f, DegToRad(90.0f), 60, 12);
			m_ShootFrameCnt = PlaneManager::totalFrameCnt;
		}
	}

}

void UserPlane::Refresh()
{
	m_FireLevel = 1;
	m_BombFrameCnt = 0;
	m_ComboFrameCnt = 0;
	m_CrashFrameCnt = 0;
	m_CurrRate = 1;
	m_HitFrameCnt = 0;
	m_IsUndead = false;
	m_ShootFrameCnt = 0;
}

void UserPlane::LiftRate()
{
	if (m_CurrRate < m_MaxRate)
	{
		m_CurrRate++;
	}
	m_ComboFrameCnt = PlaneManager::totalFrameCnt;
}

void UserPlane::Hurt()
{
	if (m_Alive && !m_IsUndead)
	{
		// �ж������ƶܣ������޵�ʱ��
		if (m_Shields > 0)
		{
			PlaneManager::snd["DAMAGE"]->Play(0UL, 0UL, LevelToVolume(PlaneManager::volume));
			m_Shields--;
			m_HitFrameCnt = PlaneManager::totalFrameCnt;
			m_IsUndead = true;
		}
		else
		{
			// û��������
			PlaneManager::snd["MYBLAST"]->Play(0UL, 0UL, LevelToVolume(PlaneManager::volume));
			m_Alive = false;
			PlaneManager::noMiss = false;
			m_HitFrameCnt = m_CrashFrameCnt = PlaneManager::totalFrameCnt;
		}
	}
}





Bullet::Bullet(float posX, float posY, float vel, float radian, float width, float height)
	: m_PosX(posX), m_PosY(posY), m_Vel(vel), m_Rotation(radian),
	m_Width(width), m_Height(height), m_Alive(true), m_SideWipe(false)
{
}

void Bullet::Update()
{
	m_PosX += m_Vel * cosf(m_Rotation);
	m_PosY += m_Vel * sinf(m_Rotation);
	if (m_PosX < 0.0f || m_PosX > 750.0f || m_PosY < 0.0f || m_PosY > 900.0f)
		m_Alive = false;
}

void Bullet::Follow(const RECT & rect)
{
	float centerX = (rect.right + rect.left) / 2.0f;
	float centerY = (rect.top + rect.bottom) / 2.0f;
	float rad = atan2f(centerY - m_PosY, centerX - m_PosX);
	// ���ٵ���
	float diff = m_Rotation - rad;
	if (diff < 0.0f)
		diff += MathHelper::Pi * 2;
	if (fmod(diff, MathHelper::Pi * 2) < 0.4f)
		m_Rotation = rad;
	else if (diff < MathHelper::Pi)
		m_Rotation -= 0.4f;
	else if (diff > MathHelper::Pi)
		m_Rotation += 0.4f;
	
	m_PosX += m_Vel * cosf(m_Rotation);
	m_PosY += m_Vel * sinf(m_Rotation);
	if (m_PosX < 0.0f || m_PosX > 750.0f || m_PosY < 0.0f || m_PosY > 1000.0f)
		m_Alive = false;
}

RECT Bullet::GetRect() const
{
	return RECT{
		static_cast<LONG>(m_PosX - m_Width / 2),
		static_cast<LONG>(m_PosY - m_Height / 2),
		static_cast<LONG>(m_PosX + m_Width / 2),
		static_cast<LONG>(m_PosY + m_Height / 2)
	};
}

RECT Bullet::GetHitRect() const
{
	// �����������̶�����������Ϊ�����ж�
	if (m_Width > 80.0f)
	{
		return RECT{
			static_cast<LONG>(m_PosX - 5),
			static_cast<LONG>(m_PosY - 50),
			static_cast<LONG>(m_PosX + 5),
			static_cast<LONG>(m_PosY + 50)
		};
	}
	// ���඼�������������ж�
	else
	{
		LONG minLen = min(m_Width, m_Height);
		return RECT{
			static_cast<LONG>(m_PosX - minLen / 2),
			static_cast<LONG>(m_PosY - minLen / 2),
			static_cast<LONG>(m_PosX + minLen / 2),
			static_cast<LONG>(m_PosY + minLen / 2)
		};
	}
	
}

void PlaneManager::Initialize(HWND hWnd, MKInput * pIn)
{
	PlaneManager::pIn = pIn;
	csound.Initialize(hWnd, DSSCL_PRIORITY);
	csound.SetPrimaryBufferFormat(2, 22050, 16);
	// DSBCAPS_CTRLVOLUME���Կ�������
	csound.Create(&snd["SHOT"], L"Sound/SHOT.WAV", DSBCAPS_CTRLVOLUME, GUID_NULL, 20UL);
	csound.Create(&snd["SHOT2"], L"Sound/SHOT2.WAV", DSBCAPS_CTRLVOLUME, GUID_NULL, 20UL);
	csound.Create(&snd["BLAST1"], L"Sound/BLAST1.WAV", DSBCAPS_CTRLVOLUME, GUID_NULL, 20UL);
	csound.Create(&snd["BLAST2"], L"Sound/BLAST2.WAV", DSBCAPS_CTRLVOLUME, GUID_NULL, 20UL);
	csound.Create(&snd["BLAST3"], L"Sound/BLAST3.WAV", DSBCAPS_CTRLVOLUME, GUID_NULL, 20UL);
	csound.Create(&snd["BLAST4"], L"Sound/BLAST4.WAV", DSBCAPS_CTRLVOLUME);
	csound.Create(&snd["BOMBER1"], L"Sound/BOMBER1.WAV", DSBCAPS_CTRLVOLUME, GUID_NULL, 20UL);
	csound.Create(&snd["ITEM0"], L"Sound/ITEM0.WAV", DSBCAPS_CTRLVOLUME);
	csound.Create(&snd["ITEM1"], L"Sound/ITEM1.WAV", DSBCAPS_CTRLVOLUME);
	csound.Create(&snd["ITEM2"], L"Sound/ITEM2.WAV", DSBCAPS_CTRLVOLUME);
	csound.Create(&snd["MYBLAST"], L"Sound/MYBLAST.WAV", DSBCAPS_CTRLVOLUME);
	csound.Create(&snd["CWS"], L"Sound/CWS.WAV", DSBCAPS_CTRLVOLUME, GUID_NULL, 20UL);
	csound.Create(&snd["DAMAGE"], L"Sound/DAMAGE.WAV", DSBCAPS_CTRLVOLUME);
	csound.Create(&snd["SELECT"], L"Sound/SELECT.WAV", DSBCAPS_CTRLVOLUME);
	csound.Create(&snd["WARNING"], L"Sound/WARNING.WAV", DSBCAPS_CTRLVOLUME);
	csound.Create(&snd["START"], L"Sound/START.WAV", DSBCAPS_CTRLVOLUME, GUID_NULL, 5UL);
	csound.Create(&snd["LASER"], L"Sound/LASER.WAV", DSBCAPS_CTRLVOLUME, GUID_NULL, 20UL);
	csound.Create(&snd["CHARGE"], L"Sound/CHARGE.WAV", DSBCAPS_CTRLVOLUME, GUID_NULL, 5UL);
	csound.Create(&snd["1UP"], L"Sound/1UP.WAV", DSBCAPS_CTRLVOLUME);
	csound.Create(&snd["KASURI"], L"Sound/KASURI.WAV", DSBCAPS_CTRLVOLUME, GUID_NULL, 20UL);
	csound.Create(&snd["VULCAN"], L"Sound/VULCAN.WAV", DSBCAPS_CTRLVOLUME, GUID_NULL, 50UL);
}

void PlaneManager::Release()
{
	for (auto& ptr : snd)
	{
		if (ptr.second)
			delete ptr.second;
	}
}

void PlaneManager::InitEnemyPlanes()
{
	FILE * fp = fopen("Data/EnemyInfo.txt", "r");
	if (!fp)
	{
		MessageBox(0, L"�Ҳ����л���Ϣ�ļ���", 0, 0);
		exit(EXIT_FAILURE);
	}
	else
	{
		char buffer1[256], buffer2[256];
		int typeID, HP, score, penalty, minscore;
		float velX, velY, posY;
		int width, height, follow, inside, isRotating;
		
		enemyPlaneInfo.clear();
		while (fgets(buffer1, sizeof(buffer1), fp))
		{
			if (buffer1[0] == buffer1[1] && buffer1[0] == '/')
				continue;
			fscanf_s(fp, "%*s%d%*s%d%*s%d%*s%d%*s%d%*s%f%*s%f%*s%f%*s%d%*s%d%*s%d%*s%d%*s%d%*s%s%*s%s",
				&typeID, &HP, &score, &penalty, &minscore, &velX, &velY, &posY, &width,
				&height, &follow, &inside, &isRotating, buffer1, 256, buffer2, 256);

			enemyPlaneInfo.emplace_front(typeID, HP, score, penalty, minscore, 
				velX, velY, posY, width, height, follow, inside, isRotating, buffer1, buffer2);
			while (fgetc(fp) != '\n')
			{
				continue;
			}
		}
		fclose(fp);
	}
}

void PlaneManager::InitStage(int stage)
{
	PlaneManager::stage = stage;
	// ��յ�ǰ֡
	totalFrameCnt = 0;
	bossKillFrameCnt = 0;
	// �ع鱶��
	finalRate = 1;
	// ���NoMiss
	noMiss = true;
	// ���Complete״̬
	stageComplete = false;
	// ��յл����ӵ���Item
	enemyPlanes.clear();
	enemyBullets.clear();
	enemyFlames.clear();
	enemyCommands.clear();
	enemyLasers.clear();
	playerBullets.clear();
	playerFlames.clear();
	stageInfo.clear();
	items.clear();

	// Bossģʽ�µ�������Item
	if (bossMode)
	{
		// ��������Item��һ��Power��һ��Shield
		PlaneManager::items.emplace_front(200.0f, 200.0f, 3);
		PlaneManager::items.emplace_front(600.0f, 200.0f, 2);
	}

	char fileName[MAX_PATH];
	sprintf_s(fileName, "Data/EnemyLevel%d.txt", stage);
	std::ifstream fin(fileName);
	// ��ȡ�ؿ��л��Ĺ�����ʽ
	if (!fin.is_open())
	{
		MessageBox(0, L"�Ҳ����л�AI�ļ���", 0, 0);
		exit(EXIT_FAILURE);
	}
	else
	{
		std::string str, ignore;
		int typeID, cmdCount, frameCnt;
		Command lineCmd;
		// ��վ�ָ��
		enemyCommands.clear();
		while (!fin.eof())
		{		
			if (!(fin >> ignore >> typeID >> ignore >> cmdCount >> ignore))
				break;
			// ������ָ�
			enemyCommands.push_back(CommandSet());
			enemyCommands.back().typeID = typeID;
			enemyCommands.back().Cmds.resize(cmdCount);
			// ��ȡ������
			while (fin >> frameCnt >> lineCmd.funcName &&
				std::getline(fin, lineCmd.funcArgs))
			{
				enemyCommands.back().mainCmd.insert(std::make_pair(frameCnt, lineCmd));
			}
			fin.clear();
			std::getline(fin, ignore);
			// ��ȡ���
			for (int i = 0; i < cmdCount; ++i)
			{
				while (fin >> frameCnt >> lineCmd.funcName &&
					std::getline(fin, lineCmd.funcArgs))
				{
					enemyCommands.back().Cmds[i].insert(std::make_pair(frameCnt, lineCmd));
				}
				fin.clear();
				std::getline(fin, ignore);
			}
		}
		fin.close();
	}
	
	sprintf_s(fileName, "Data/Stage%d.txt", stage);
	fin.open(fileName);
	// ��ȡ�ؿ��л��ĳ�����ʽ
	std::multimap<int, std::string> emptyMap;
	stageInfo.swap(emptyMap);
	if (!fin.is_open())
	{
		MessageBox(0, L"�Ҳ����ؿ���Ϣ�ļ���", 0, 0);
		exit(EXIT_FAILURE);
	}
	else
	{
		std::string str;
		int frame;
		// ����ǰ���ע��
		while (fin.peek() == '/')
			std::getline(fin, str);
		fin >> str >> bossFrameCnt;

		while (fin >> frame && std::getline(fin, str))
		{
			stageInfo.insert(std::make_pair(frame, str));
		}
		fin.close();

		if (bossMode)
			totalFrameCnt = bossFrameCnt - 1000;
	}


}

void PlaneManager::UpdateScene()
{
	// ��ӵл�
	auto BegEnd = stageInfo.equal_range(totalFrameCnt);
	auto Beg = BegEnd.first, End = BegEnd.second;
	for (; Beg != End; ++Beg)
	{
		std::istringstream iss(Beg->second);
		int typeID, minPosX, maxPosX, item;
		// ��ӦtypeID, minPosX, maxPosX
		iss >> typeID >> minPosX >> maxPosX;
		// �ҵ�ID�Ŷ�Ӧ�Ѿ���ʼ���õķɻ����ƹ�ȥ
		auto itPlane = std::find_if(enemyPlaneInfo.begin(), enemyPlaneInfo.end(), [typeID](const EnemyPlane& plane) {
			return plane.GetTypeID() == typeID;
		});
		enemyPlanes.push_front(*itPlane);
		// ���ɻ�ID��Ϊ3������������
		if (typeID == 3)
		{
			iss >> item;
			enemyPlanes.front().SetItem(item);
		}
		// �ҵ�ID�Ŷ�Ӧ��ָ�����ʼ��
		auto itCmd = std::find_if(enemyCommands.begin(), enemyCommands.end(), [typeID](const CommandSet& cmdSet) {
			return cmdSet.typeID == typeID;
		});
		enemyPlanes.front().InitAI(&*itCmd);
		// ��ʼ���ɻ�λ��
		enemyPlanes.front().SetPosX(minPosX, maxPosX);
	}

	// ��ײ���
	RECT playerHitRect = player.GetHitRect();
	RECT playerRect = player.GetRect();
	bool playerHurt = false;
	
	// ����Ƿ��޵��Ҵ��
	if (!PlaneManager::player.IsUndead() && PlaneManager::player.IsAlive())
	{
		// ��⼺����л�����ײ
		if (!playerHurt)
		{
			for (auto& plane : enemyPlanes)
			{
				if (plane.IsAlive() && RectCollision(playerHitRect, plane.GetRect()))
				{
					player.Hurt();
					playerHurt = true;
					break;
				}
			}
		}


		// ��⼺�����ӵ�����ײ
		if (!playerHurt)
		{
			for (auto& bullet : enemyBullets)
			{
				if (bullet.IsAlive() && RectCollision(playerHitRect, bullet.GetHitRect()))
				{
					bullet.Destroy();
					player.Hurt();
					playerHurt = true;
					break;
				}
			}
		}

		// ��⼺���뼤�����ײ
		if (!playerHurt)
		{
			for (auto& laser : enemyLasers)
			{
				if (laser.IsAlive() && RectCollision(playerHitRect, laser.GetHitRect()))
				{
					laser.Destroy();
					player.Hurt();
					playerHurt = true;
					break;
				}
			}
		}

		// ��⼺����������ײ
		if (!playerHurt)
		{
			for (auto& flame : enemyFlames)
			{
				if (flame.IsAlive() && RectCollision(playerHitRect, flame.GetHitRect()))
				{
					flame.Destroy();
					player.Hurt();
					playerHurt = true;
					break;
				}
			}
		}
	}
	
	// ����Ƿ���
	if (player.IsAlive())
	{
		// ��⼺�����ӵ��Ĳ���
		for (auto& bullet : enemyBullets)
		{
			if (!bullet.IsSideWipe() && RectCollision(playerRect, bullet.GetHitRect()))
			{
				if (player.HasWipe())
				{
					snd["CWS"]->Play(0UL, 0UL, LevelToVolume(PlaneManager::volume));
					bullet.SetWipe();
					player.UseWipe();
					playerFlames.emplace_back(40.0f + playerRect.left, 100.0f + playerRect.top,
						36.0f, DegToRad(90.0f), 60, 12);
				}
				else
					snd["KASURI"]->Play(0UL, 0UL, LevelToVolume(PlaneManager::volume));
				break;
			}
		}
	
		// ��⼺���뼤��Ĳ���
		for (auto& laser : enemyLasers)
		{
			if (!laser.IsSideWipe() && RectCollision(playerRect, laser.GetHitRect()))
			{
				if (player.HasWipe())
				{
					snd["CWS"]->Play(0UL, 0UL, LevelToVolume(PlaneManager::volume));
					laser.SetWipe();
					player.UseWipe();
					playerFlames.emplace_back(40.0f + playerRect.left, 100.0f + playerRect.top,
						36.0f, DegToRad(90.0f), 60, 12);
				}
				else
					snd["KASURI"]->Play(0UL, 0UL, LevelToVolume(PlaneManager::volume));
				break;
			}
		}

		// ��⼺�����ӵ��Ĳ���
		for (auto& flame : enemyFlames)
		{
			if (!flame.IsSideWipe() && RectCollision(playerRect, flame.GetHitRect()))
			{
				if (player.HasWipe())
				{
					snd["CWS"]->Play(0UL, 0UL, LevelToVolume(PlaneManager::volume));
					flame.SetWipe();
					player.UseWipe();
					playerFlames.emplace_back(40.0f + playerRect.left, 100.0f + playerRect.top,
						36.0f, DegToRad(90.0f), 60, 12);
				}
				else
					snd["KASURI"]->Play(0UL, 0UL, LevelToVolume(PlaneManager::volume));
				break;
			}
		}

		// ��⼺����Item����ײ
		for (auto& item : items)
		{
			if (item.IsAlive() && RectCollision(playerRect, item.GetRect()))
			{
				item.Destroy();
				player.RecvItem(item.GetType());
				break;
			}
		}
	}

	// ���з��뼺���ӵ�����ײ
	for (auto& bullet : playerBullets)
	{
		for (auto& plane : enemyPlanes)
		{
			if (bullet.IsAlive() && plane.IsAlive() && 
				RectCollision(bullet.GetHitRect(), plane.GetRect()))
			{
				bullet.Destroy();
				if (!(PlaneManager::player.GetCrashFrameCnt() || PlaneManager::player.IsUsingBomb()) || plane.GetTypeID() != 15)
				{
					plane.Hurt(1);
					score += 10 * player.GetCurrRate();
				}
				break;
			}
		}
	}

	// ���з��뼺�����ٵ�����ײ
	for (auto& flame : playerFlames)
	{
		for (auto& plane : enemyPlanes)
		{
			if (flame.IsAlive() && plane.IsAlive() &&
				RectCollision(flame.GetHitRect(), plane.GetRect()))
			{
				flame.Destroy();
				if (!(PlaneManager::player.GetCrashFrameCnt() || PlaneManager::player.IsUsingBomb()) || plane.GetTypeID() != 15)
				{
					plane.Hurt(4);
					score += 40 * player.GetCurrRate();

					// �������ʣ���ը����ʱ�򲻸��±���
					if (!player.IsUsingBomb())
						player.LiftRate();
				}
				break;
			}
		}
	}

	
	auto UpdateFlame = [&](Bullet& flame) { 
		if (flame.IsAlive())
		{
			if (enemyPlanes.empty())
				flame.Update();
			else
			{
				if (enemyPlanes.back().IsAlive())
					flame.Follow(enemyPlanes.back().GetRect());
				else
					flame.Update();
			}
				
		}
	};
	auto UpdateBullet = [](Bullet& bullet) {  if (bullet.IsAlive()) bullet.Update(); };
	auto UpdatePlane = [](EnemyPlane& plane) { if (plane.IsAlive()) plane.Update(); };
	auto UpdateItem = [](Item& item) { if (item.IsAlive()) item.Update(); };
	// ��ұ��ƶ� ���� ��ұ����ٺ�ճ�����2s�� ���� ����ڷ�ը���������˵�Ļ
	if ((playerHurt || player.GetCrashFrameCnt() || player.IsUsingBomb()) 
		&& player.IsAlive())
	{
		enemyBullets.clear();
		enemyLasers.clear();
		enemyFlames.clear();
	}
	else
	{
		// ���µз��ӵ�
		std::for_each(enemyBullets.begin(), enemyBullets.end(), UpdateBullet);
		std::for_each(enemyLasers.begin(), enemyLasers.end(), UpdateBullet);
		std::for_each(enemyFlames.begin(), enemyFlames.end(), UpdateBullet);
	}
	// ���µз��ɻ�
	std::for_each(enemyPlanes.begin(), enemyPlanes.end(), UpdatePlane);
	// �����ҷ��ӵ�
	std::for_each(playerBullets.begin(), playerBullets.end(), UpdateBullet);
	std::for_each(playerFlames.begin(), playerFlames.end(), UpdateFlame);
	// ��������
	std::for_each(items.begin(), items.end(), UpdateItem);
	// �������
	player.Update(pIn);


	// ����������ɳ��߽�ĵл����ӵ������ߵ�
	auto BulletDead = [](const Bullet& bullet) {return !bullet.IsAlive(); };
	auto EnemyPlaneDead = [](const EnemyPlane& plane) { return !plane.IsAlive(); };
	auto ItemDead = [](const Item& item) { return !item.IsAlive(); };
	enemyBullets.remove_if(BulletDead);
	enemyLasers.remove_if(BulletDead);
	enemyFlames.remove_if(BulletDead);
	playerBullets.remove_if(BulletDead);
	playerFlames.remove_if(BulletDead);
	enemyPlanes.remove_if(EnemyPlaneDead);
	items.remove_if(ItemDead);
	// ��֡����1
	++totalFrameCnt;
}

void PlaneManager::InitReplay(LPCSTR filePath)
{
	std::ifstream fin(filePath, std::ios::in | std::ios::binary);
	// ��ʼ�������
	fin.read(reinterpret_cast<char*>(&randomSeed), sizeof(time_t));
	srand((unsigned)randomSeed);
	// ��ʼ���ؿ�
	int rpStage;
	fin.read(reinterpret_cast<char *>(&rpStage), sizeof(int));
	// ��ʼ��Bossģʽ
	int rpBossMode;
	fin.read(reinterpret_cast<char *>(&rpBossMode), sizeof(int));
	bossMode = rpBossMode;
	InitStage(rpStage);
	// ��ʼ���ɻ�����
	int rpPlaneType;
	fin.read(reinterpret_cast<char *>(&rpPlaneType), sizeof(int));
	player.Initialize(rpPlaneType);
	// ��ȡ�ؿ���Ϣ
	unsigned short data[2];
	replayInfo.clear();
	while (!fin.eof())
	{
		fin.read(reinterpret_cast<char *>(&data), sizeof(data));
		if (data[0] == 0xffff)
			rpStage = data[1];
		else
			replayInfo[rpStage][data[0]] = data[1];
	}
	fin.close();
}

void Item::Update()
{
	m_PosY += 2.0f;
	if (m_PosY > 900.0f)
		m_Alive = false;
}

RECT Item::GetRect() const
{
	return RECT{
		static_cast<LONG>(m_PosX - 25),
		static_cast<LONG>(m_PosY - 25),
		static_cast<LONG>(m_PosX + 25),
		static_cast<LONG>(m_PosY + 25),
	};
}
