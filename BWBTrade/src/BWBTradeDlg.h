
// BWBTradeDlg.h : 头文件
//

#pragma once
#include <clib/lib/util/config.h>
#include "afxwin.h"
#include "afxcmn.h"
enum eTimerType
{
	eTimerType_APIUpdate,
	eTimerType_Balance,
	eTimerType_EntrustDepth,
};

// CBWBTradeDlg 对话框
class CBWBTradeDlg : public CDialogEx
{
public:
	clib::config m_config;
	bool m_bIsRun;
public:
	void CheckPrice();
	void AddLog(char* szLog, ...);
	void UpdateEntrustDepth();
	void UpdateBalance();
// 构造
public:
	CBWBTradeDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_BWBTRADE_DIALOG };

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
	CListBox m_listLog;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedBegin();
	CListCtrl m_listCtrlBwbUsdt;
	CListCtrl m_listCtrlBwbBtc;
	double m_bwbUsdtBuyPrice;
	double m_bwbUsdtSellPrice;
	double m_bwbUsdtWatchBuyPrice;
	double m_bwbUsdtWatchSellPrice;
	double m_bwbBtcWatchBuyPrice;
	double m_bwbBtcWatchSellPrice;
	double m_bwbBtcBuyPrice;
	double m_bwbBtcSellPrice;
	CListCtrl m_listCtrlBalance;
	std::map<std::string, time_t> m_mapSellOrders;
	CEdit m_editBwbUdstBuyPrice;
	CEdit m_editBwbUdstSellPrice;
	CEdit m_editBwbUdstWatchBuyPrice;
	CEdit m_editBwbUdstWatchSellPrice;

	CEdit m_editBwbBtcBuyPrice;
	CEdit m_editBwbBtcSellPrice;
	CEdit m_editBwbBtcWatchBuyPrice;
	CEdit m_editBwbBtcWatchSellPrice;
};
