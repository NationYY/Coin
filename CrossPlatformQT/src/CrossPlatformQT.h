
// CrossPlatformQT.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CCrossPlatformQTApp: 
// �йش����ʵ�֣������ CrossPlatformQT.cpp
//

class CCrossPlatformQTApp : public CWinApp
{
public:
	CCrossPlatformQTApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CCrossPlatformQTApp theApp;