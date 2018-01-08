文本编辑器  V1.2
作者       X_Jun

文本编辑器的初衷是为了简化游戏关卡的设计，将其从程序代码中分离。只要稍加学习，
并理解其中的参数设置，你也可以创建属于自己的关卡。


游戏的运行依赖三个部分的文档组成：
EnemyInfo.txt     存储了所有飞机的属性
EnemyLevelX.txt   X指的是关卡号，用于设置该关卡所有飞机的AI
StageX.txt        X指的是关卡号，用于设置该关卡所有飞机的布置顺序
注意：1.关卡的编写必须是连续的，例如要创建第5关得确保前4关的文件都要存在
      2.游戏限定在60帧。
      3.文件名不能是EnemyLevel01.txt，必须为EnemyLevel1.txt

************************* EnemyInfo.txt文件编写 *******************************
每个飞机的编写参数和顺序要求如下：
enemy_plane1                        注释：第一行描述飞机类型，不会被读入
typeID:     1                       注释：该类型飞机占用的ID
HP:         5                       注释：血量，1发普通子弹1伤害，1发追踪弹5伤害
score:      640                     注释：初始得分
penalty:    -2                      注释：每一帧的惩罚分
minscore:   200                     注释：最低保证得分
velX:       0.0                     注释：初始横向速度（向右）
velY:       10.0                    注释：初始纵向速度（向下）
posY:       -200.0                  注释：初始纵向位置
width:      100                     注释：纹理宽度/飞机宽度
height:     100                     注释：纹理高度/飞机高度
follow:     0                       注释：0表示不跟随玩家，1表示跟随
inside:     0                       注释：0表示默认行动下会飞出界面，1表示不会飞出界面
isRotating: 0                       注释：飞机时候会旋转
deathsnd:   BLAST1                  注释：指定死亡音效，可选范围BLAST1到BLAST4
texName:    enemy_plane1            注释：指定使用的纹理贴图(存储在Texture文件夹内) 
////////////////////////////        注释：用于结束标记

注意：
1. 冒号和数字间必须要有空格，否则会联通数字被忽略导致读取出错
2. 该文件不支持注释
3. 数字左边的描述不区分大小写，你可以写TYPEID
4. 不能缺写，不能多写，不能调换顺序
********************************************************************************
************************** EnemyLevelX.txt文件编写 *****************************
一个飞机同时执行两套指令集：主指令(MainCmd)和副指令。
主指令只能有一套，但副指令可以有多套，通过指定cmdCount的值来确定有多少套副指令
以下是支持的函数：

============================= MainCmd中可用函数 ================================
函数名    SetCmd  
参数      Pos...
用法      设置当前副指令为Pos里面的其中一个，范围为[0, cmdCount - 1]
使用展示  SetCmd 4 5 6


函数名    SetFrameIfHPLess
参数      HP, FrameVal
用法      若血量值小于等于HP，将当前帧设为FrameVal
使用展示  SetFrameIfHPLess 1200 600


函数名    SetFrame 
参数      FrameVal
用法      当前帧设为FrameVal
使用展示  SetFrame 500

============================= 指令集中可用函数 =================================
函数名    SetVelX
参数      VelX
用法      设置X方向速度
使用展示  SetVelX 4.0


函数名    SetVelY
参数      VelY
用法      设置Y方向速度
使用展示  SetVelY 4.0


函数名    SetAwayVelX
参数      VelX
用法      当飞机在屏幕左半侧的时候，VelX取负值作为X方向速度
          当飞机在屏幕右半侧的时候，VelX取正值作为X方向速度
使用展示  SetAwayVelX 4.0


函数名    Shoot
参数      PosX PosY Vel Angles... 
用法          在飞机贴图坐标系中，X轴向右，Y轴向下，且飞机贴图左上角   ┏━━→
          作为原点，(PosX, PosY)为该贴图坐标系的一点，表示子弹中心的   ┃
          起始发射坐标。                                               ┃  ·(PosX,PosY)
              Vel为发射速度                                            ↓
              Angle为发射的角度，其中屏幕XY坐标系及角度规定如下：
                      -90°
                       ↑
                       ┃
           -180°      ┃
                 ←━━╋━━→ 0°X
            180°      ┃
                       ┃
                       ↓
                       90°
                        Y
              可以一次性指定多个Angle，即一个函数可以发射多个子弹。
使用展示  Shoot 30 50 5.0 90
          Shoot 40 50 8.0 60 70 80 90 100 110 120 130


函数名    ShootToPlayer
参数      PosX PosY Vel Angles...
用法          以飞机贴图坐标系中的位置(PosX, PosY)作为子弹的发射起点，
          发射速度为Vel，Angle为0.0时子弹会攻击玩家
              若Angle越大，子弹偏移玩家的角度也越大
              可以一次性指定多个Angle，即一个函数可以发射多个子弹。
使用展示  ShootToPlayer     50 95 4.0 0.0
          ShootToPlayer     50 95 4.0 -6.0 -3.0 0.0 3.0 6.0


函数名    ShootRand
参数      PosX PosY Vel Angle OffsetAngle nums
用法          以飞机贴图坐标系中的位置(PosX, PosY)作为子弹的发射起点，
          发射速度为Vel，子弹发射角度是随机的，区间为
              [Angle - OffsetAngle, Angle + OffsetAngle]
              nums为发射的子弹数目
使用展示  ShootRand         50 100 90.0 5.0 3


函数名    ShootToPlayerRand  
参数      PosX PosY Vel AngleOffset nums
用法          以飞机贴图坐标系中的位置(PosX, PosY)作为子弹的发射起点，
          发射速度为Vel，子弹的偏移角度为AngleOffset
              nums为发射的子弹数目
使用展示  ShootToPlayerRand 56  175 5.0 12.0 1


函数名    ShootLaser
参数      PosX PosY Vel
用法          以飞机贴图坐标系中的位置(PosX, PosY)作为子弹的发射起点，
          发射速度为Vel，发射角度固定为90°
使用展示  ShootLaser 50 100 12.0


函数名    ShootFlame
参数      PosX PosY Vel Angles...
用法          Vel为发射速度
              Angle为发射角度
              可以一次性指定多个Angle，即一个函数可以发射多个火焰弹。
使用展示  ShootFlame     30 50 5.0 90
          ShootFlame     40 50 8.0 60 70 80 90 100 110 120 130


函数名    ShootFlameRand
参数      PosX PosY Vel Angle OffsetAngle nums
用法          以飞机贴图坐标系中的位置(PosX, PosY)作为火焰弹的发射起点，
          发射速度为Vel，火焰弹发射角度是随机的，区间为
              [Angle - OffsetAngle, Angle + OffsetAngle]
              nums为发射的火焰弹数目
使用展示  ShootFlameRand      50 100 90.0 5.0 3



函数名    Charge
参数      0
用法      播放CHARGE.WAV音效
使用展示  Charge 0
====================================================================

每个飞机的编写参数和顺序要求参照如下格式：
typeID: 1    cmdCount: 1                 | typeID为飞机ID，cmdCount为副指令数目
MainCmd                                  | 主指令开始的标记
0    SetCmd            0                 | 第一个数为相对帧数，当相对帧数到达该值时，执行该函数 
=============================            | 副指令开始的标记，对应的是副指令0
35   SetVelY           0.0               | 同样首先需要提供相对帧数
35   ShootToPlayer     50 95 4.0 0.0     | 
125  SetVelY           10.0              |
125  ShootToPlayer     50 95 4.0 0.0     |
/////////////////////////                | 结束标记


注意：
1. 函数指令区分大小写
2. 必须严格使用=====分界线和////分界线
3. 冒号和数字之间必须要有空格
4. 两个函数之间可以有空格，但最后一个函数必须紧挨分界线

********************************************************************************
***************************** StageX.txt文件编写 *******************************
开头可以有注释，但必须以//为首
首先需要提供BossFrameCnt，作为标记Boss出场的对应帧
在默认关卡，游戏从第0帧开始
而在Boss关卡，游戏从BossFrameCnt - 1000开始

飞机的布置：
每一行需要提供四个参数： Frame TypeID MinPosX MaxPosX
Frame对应出场的帧数
TypeID对应出场的飞机ID
MinPosX和MaxPosX对应飞机出场的位置是随机的，范围在[MinPosX, MaxPosX]。
注意游戏的分辨率为750x900

由于作者将TypeID 3设置为带奖励Item的飞机，若TypeID为3，这一行需要提供五个参数：
              Frame TypeID MinPosX MaxPosX DropItem
若DropItem为1，该飞机被击杀后奖励Bomb，2为Power，3为Shield

注意：行与行之间可以有空格

以下是样例：
BossFrameCnt: 6000

300   1  600 600
320   1  570 570
340   1  540 540
360   1  510 510
380   1  480 480
400   1  450 450
420   1  420 420
440   1  390 390

*********************************************************************************
******************************    实际测试      *********************************
在编写好三个文档后，打开 超级战机.exe,若未能显示正常界面，说明文件编写的格式有误，
需要核对修改。只有编写格式正确时才会正常显示界面。那么赶快体验一下吧！

版本     V1.0
更新日期 2017-8-18
作者     X_Jun


