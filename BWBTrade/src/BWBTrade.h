
// BWBTrade.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CBWBTradeApp: 
// �йش����ʵ�֣������ BWBTrade.cpp
//

class CBWBTradeApp : public CWinApp
{
public:
	CBWBTradeApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CBWBTradeApp theApp;