
// CoinDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"
#include <clib/lib/util/config.h>

// CCoinDlg 对话框
class CCoinDlg : public CDialogEx
{
// 构造
public:
	CCoinDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_COIN_DIALOG };

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
	afx_msg void OnBnClickedButton1();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
public:
	void UpdateBalance();
	void UpdateEntrustDepth();
	CListCtrl m_listBalance;
	clib::config m_config;
	CListCtrl m_listCtrlSellEntrustDepth;
};
