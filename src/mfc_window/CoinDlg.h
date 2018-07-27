
// CoinDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"
#include <clib/lib/util/config.h>
#include "afxwin.h"
enum eTimerType
{
	eTimerType_APIUpdate,
	eTimerType_Balance,
	eTimerType_EntrustDepth,
	eTimerType_Trade,
	eTimerType_TradeOrderState,
	eTimerType_Ping,
};
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
	afx_msg void OnBnClickedButtonBegin();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
public:
	void UpdateBalance();
	void UpdateExecutedOrderPrice();
	void UpdateEntrustDepth();
	void UpdateFinishOrder();
	void AddLog(char* szLog, ...);
	CListCtrl m_listBalance;
	clib::config m_config;
	CListCtrl m_listCtrlEntrustDepth;
	CComboBox m_cbMarketType;
	int m_tradeFrequency;
	CStatic m_staticUpdateTime;
	CButton m_btnHightSpeed;
	CButton m_btnNormalSpeed;
	afx_msg void OnBnClickedRadioHightSpeed();
	afx_msg void OnBnClickedRadioNormalSpeed();
	int m_webSokectFailTimes;
	afx_msg void OnBnClickedButtonStop();
	bool m_bIsRun;
	CString m_strTradeVolume;
	CListCtrl m_listFinishOrder;
	double m_tradePremiumPrice;
	int m_tradePriceDecimal;
	struct STradePair
	{
		std::string id1;
		std::string id2;
		time_t tSendTime;
		time_t tLastCheckTime;
		STradePair()
		{
			id1 = id2 = "NULL";
			tSendTime = tLastCheckTime = 0;
		}
	};
	std::map<int, STradePair> m_mapTradePairs;
	eMarketType m_marketType;
	CListBox m_listLog;
};
