
// ManualOKExFuturesDlg.h : 头文件
//

#pragma once
#include <clib/lib/util/config.h>
#include "afxwin.h"
#define DOUBLE_PRECISION 0.00000001
enum eTimerType
{
	eTimerType_APIUpdate,
	eTimerType_Account,
	eTimerType_Ping,
	eTimerType_TradeOrder,
	eTimerType_ConnetServer,
};

struct STickerData
{
	bool bValid;
	__int64 time;		//时间
	int volume;			//成交量(张)
	double sell;		//卖一价格
	double buy;			//买一价格
	double high;		//24小时最高价格
	double low;			//24小时最低价格
	double last;		//最新成交价格
	STickerData(){
		memset(this, 0, sizeof(STickerData));
	}
};

struct SFuturesAccountInfo
{
	std::string equity;	//账户权益
	bool bValid;
	SFuturesAccountInfo() : equity("0"), bValid(false)
	{

	}
};

struct SFuturesTradeInfo
{
	std::string strClientOrderID;
	time_t timeStamp;	//委托时间
	std::string filledQTY;		//成交数量
	std::string orderID;//订单ID;
	double price;		//订单价格
	double priceAvg;	//成交均价
	std::string status;	//订单状态(-1.撤单成功；0:等待成交 1:部分成交 2:全部成交 ）
	eFuturesTradeType tradeType;
	time_t waitClientOrderIDTime;
	std::string size;
	time_t tLastUpdate;
	bool bModifyQTY;
	SFuturesTradeInfo()
	{
		Reset();
	}
	void Reset()
	{
		strClientOrderID = "";
		timeStamp = 0;
		filledQTY = "0";
		orderID = "";
		price = 0.0;
		priceAvg = 0.0;
		status = "";
		tradeType = eFuturesTradeType_OpenBull;
		waitClientOrderIDTime = 0;
		size = "0";
		tLastUpdate = 0;
		bModifyQTY = false;
	}
};

struct SFuturesTradePairInfo
{
	SFuturesTradeInfo open;
	SFuturesTradeInfo close;
	std::string closePlanPrice;
	std::string closePlanSize;
	SFuturesTradePairInfo()
	{
		Reset();
	}
	void Reset()
	{
		open.Reset();
		close.Reset();
		closePlanPrice = "";
		closePlanSize = "";
	}
};

struct SFuturesDepth
{
	std::string price;
	std::string size;
	int brokenSize;
	int tradeNum;
	SFuturesDepth(): size(""), brokenSize(0), tradeNum(0), price("")
	{
	}
};

// CManualOKExFuturesDlg 对话框
class CManualOKExFuturesDlg : public CDialog
{
// 构造
public:
	CManualOKExFuturesDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_MANUALOKEXFUTURES_DIALOG };

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
	void __InitBaseConfigCtrl();
	void __InitTradeConfigCtrl();
	bool __SaveBaseConfigCtrl();
	bool __SaveTradeConfigCtrl();
	void _UpdateTradeShow();
	void _UpdateAccountShow();
	void _UpdateDepthShow();
	int _GetFreeOrderIndex();
	void _OpenOrder(eFuturesTradeType type);
	void _SaveData();
	void _CheckAllOrder();
public:
	void OnRevTickerInfo(STickerData &data);
	void Pong();
	void OnLoginSuccess();
	void UpdateTradeInfo(SFuturesTradeInfo& info);
	void UpdateAccountInfo(SFuturesAccountInfo& info);
	void ClearDepth();
	void UpdateDepthInfo(bool bBuy, SFuturesDepth& info);
	bool CheckDepthInfo(int checkNum, std::string& checkSrc);
	void OnTradeSuccess(std::string& clientOrderID, std::string& serverOrderID);
	void OnTradeFail(std::string& clientOrderID);
	void SetHScroll();
public:
	time_t m_tListenPong;
	bool m_bRun;
	bool m_bSwapFutures;
	clib::config m_config;
	std::string m_apiKey;
	std::string m_secretKey;
	std::string m_passphrase;
	int m_nPriceDecimal;
	std::vector<SFuturesTradePairInfo> m_vecTradePairInfo;

	std::string m_strCoinType;			//货币类型
	std::string m_strFuturesCycle;		//合约周期
	std::string m_strFuturesTradeSize;	//下单张数
	std::string m_strLeverage;			//合约倍数
	int m_nLeverage;					//合约倍数
	STickerData m_curTickData;
	SFuturesAccountInfo m_accountInfo;
	double m_beginMoney;
	std::map<std::string, SFuturesDepth> m_mapDepthSell;
	std::map<std::string, SFuturesDepth> m_mapDepthBuy;
public:
	CListCtrl m_listCtrlOrderOpen;
	CListCtrl m_listCtrlOrderClose;
	CComboBox m_combCoinType;
	CComboBox m_combLeverage;
	CEdit m_editFuturesCycle;
	CEdit m_editFuturesTradeSize;
	CComboBox m_combFuturesType;
	CEdit m_editCapital;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedStart();
	CListBox m_ctrlListLog;
	CListCtrl m_listCtrlDepth;
	CEdit m_editOpenPrice;
	afx_msg void OnBnClickedButtonOpenBull();
	afx_msg void OnBnClickedButtonOpenBear();
	CStatic m_staticProfit;
	CStatic m_staticAccountInfo;
	CEdit m_editClosePrice;
	CEdit m_editCloseSize;
	afx_msg void OnBnClickedButtonClose();
	afx_msg void OnBnClickedButtonCancel();
	afx_msg void OnBnClickedButtonUpdateBeginMoney();
};
