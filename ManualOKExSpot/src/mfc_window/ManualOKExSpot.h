
// ManualOKExSpot.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CManualOKExSpotApp: 
// �йش����ʵ�֣������ ManualOKExSpot.cpp
//

class CManualOKExSpotApp : public CWinApp
{
public:
	CManualOKExSpotApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CManualOKExSpotApp theApp;