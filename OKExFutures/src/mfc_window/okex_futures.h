
// okex_futures.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// COKExFuturesApp: 
// �йش����ʵ�֣������ okex_futures.cpp
//

class COKExFuturesApp : public CWinApp
{
public:
	COKExFuturesApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern COKExFuturesApp theApp;