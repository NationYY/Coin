
// ManualOKExFutures.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CManualOKExFuturesApp: 
// �йش����ʵ�֣������ ManualOKExFutures.cpp
//

class CManualOKExFuturesApp : public CWinApp
{
public:
	CManualOKExFuturesApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CManualOKExFuturesApp theApp;