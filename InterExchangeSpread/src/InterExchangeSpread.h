
// InterExchangeSpread.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CInterExchangeSpreadApp: 
// �йش����ʵ�֣������ InterExchangeSpread.cpp
//

class CInterExchangeSpreadApp : public CWinApp
{
public:
	CInterExchangeSpreadApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CInterExchangeSpreadApp theApp;