
// OKExMartingale.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// COKExMartingaleApp: 
// �йش����ʵ�֣������ OKExMartingale.cpp
//

class COKExMartingaleApp : public CWinApp
{
public:
	COKExMartingaleApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern COKExMartingaleApp theApp;