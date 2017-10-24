#pragma once

#include "d3dUtil.h"
#include "d2dText.h"
#include "MKInput.h"
#define BYTE_TO_UINT(a,b,c,d) (((unsigned)(a) << 24) | ((unsigned)(b) << 16) | ((unsigned)(c) << 8) | ((unsigned)(d)))
#define ReleaseStatic(Type) { Type::Release(); }

// 获取图片像素宽度和高度
void GetPictureInfo(LPCSTR fileName, LONG * pOutWidth, LONG * pOutHeight);

// 生成Effect
void BuildFX(ID3D11Device* dev, LPCWSTR fileName, ID3DX11Effect** ppEffect);

// 检测两个AABB矩形的碰撞
bool RectCollision(const RECT& r1, const RECT& r2);

// 角度转弧度
float DegToRad(float degree);

// 生成ClickBox，鼠标在移动到ClickBox时会高亮，点击它返回true，没点击返回false
bool ClickBox(const MKInput* pIn, const D2D1_RECT_F& rect, D2DText& d2dText, LPCWSTR wstr);

// 计算出音量值
LONG LevelToVolume(int level);