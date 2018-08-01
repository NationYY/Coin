
// InterExchangeSpreadDlg.h : ͷ�ļ�
//
#include "afxwin.h"
#include <clib/lib/util/config.h>

#pragma once
enum eTimerType
{
	eTimerType_APIUpdate,
	eTimerType_Ping,
};
// CInterExchangeSpreadDlg �Ի���
class CInterExchangeSpreadDlg : public CDialogEx
{
public:
	clib::config m_config;
public:
	void UpdateShowEntrustDepth();
	void AddLog(char* szLog, ...);
// ����
public:
	CInterExchangeSpreadDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_INTEREXCHANGESPREAD_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	
	
	CListBox m_listCtrlLog;
	CListCtrl m_listCtrlEntrustDepth;
	afx_msg void OnBnClickedBegin();
};
