
// InterExchangeSpreadDlg.h : 头文件
//
#include "afxwin.h"
#include <clib/lib/util/config.h>

#pragma once
enum eTimerType
{
	eTimerType_APIUpdate,
	eTimerType_Ping,
};
// CInterExchangeSpreadDlg 对话框
class CInterExchangeSpreadDlg : public CDialogEx
{
public:
	clib::config m_config;
	bool m_bIsRun;
public:
	void OnWebsocketConnect(const char* szExchangeName);
	void UpdateShowEntrustDepth();
	void AddLog(char* szLog, ...);
// 构造
public:
	CInterExchangeSpreadDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_INTEREXCHANGESPREAD_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
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
	afx_msg void OnClose();
};
