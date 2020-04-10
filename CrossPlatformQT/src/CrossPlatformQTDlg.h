
// CrossPlatformQTDlg.h : 头文件
//

#pragma once
#include <clib/lib/util/config.h>
#include "afxwin.h"
enum eTimerType
{
	eTimerType_APIUpdate,
	eTimerType_Ping,
};
// CCrossPlatformQTDlg 对话框
class CCrossPlatformQTDlg : public CDialogEx
{
// 构造
public:
	CCrossPlatformQTDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_CROSSPLATFORMQT_DIALOG };

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
private:
	bool __SaveBaseConfigCtrl();
	void __InitBaseConfigCtrl();

public:
	void Pong();
	void OnLoginSuccess();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedButtonStart();
	CComboBox m_combCoinType;
	CComboBox m_combMoneyType;
	CEdit m_editFuturesCycle;
	CComboBox m_combLeverage;
	CComboBox m_combFuturesType;


public:
	clib::config m_config;
	std::string m_apiKey;
	std::string m_secretKey;
	std::string m_passphrase;
	time_t m_tListenPong;
	bool m_bRun;
	bool m_bSwapFutures;
	std::string m_strCoinType;			//主币类型
	std::string m_strMoneyType;			//辅币类型
	std::string m_strFuturesCycle;		//合约周期
	std::string m_strLeverage;			//合约倍数
	int m_nLeverage;					//合约倍数
	int m_nPriceDecimal;
};
