#pragma once

#include "d3dUtil.h"
#include "d2dText.h"
#include "MKInput.h"
#define BYTE_TO_UINT(a,b,c,d) (((unsigned)(a) << 24) | ((unsigned)(b) << 16) | ((unsigned)(c) << 8) | ((unsigned)(d)))
#define ReleaseStatic(Type) { Type::Release(); }

// ��ȡͼƬ���ؿ�Ⱥ͸߶�
void GetPictureInfo(LPCSTR fileName, LONG * pOutWidth, LONG * pOutHeight);

// ����Effect
void BuildFX(ID3D11Device* dev, LPCWSTR fileName, ID3DX11Effect** ppEffect);

// �������AABB���ε���ײ
bool RectCollision(const RECT& r1, const RECT& r2);

// �Ƕ�ת����
float DegToRad(float degree);

// ����ClickBox��������ƶ���ClickBoxʱ����������������true��û�������false
bool ClickBox(const MKInput* pIn, const D2D1_RECT_F& rect, D2DText& d2dText, LPCWSTR wstr);

// ���������ֵ
LONG LevelToVolume(int level);