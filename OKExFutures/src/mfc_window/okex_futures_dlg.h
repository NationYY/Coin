
// okex_futures_dlg.h : 头文件
//

#pragma once

#include <clib/lib/util/config.h>
#include "afxwin.h"
enum eTimerType
{
	eTimerType_APIUpdate,
	eTimerType_EntrustDepth,
	eTimerType_Ping,
};

enum eBollTrend
{
	eBollTrend_Normal,			//空转
	eBollTrend_ShouKou,			//收口
	eBollTrend_ShouKouChannel,	//收口通道
	eBollTrend_ZhangKou,		//张口
};

struct SKlineData
{
	__int64 time;		//时间
	char szTime[20];
	double openPrice;	//开盘价
	double highPrice;	//最高价
	double lowPrice;	//最低价
	double closePrice;	//收盘价
	int volume;			//成交量(张)
	double volumeByCurrency;//成交量(币)
	SKlineData(){
		memset(this, 0, sizeof(SKlineData));
	}
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

struct SBollInfo
{
	__int64 time;
	char szTime[20];
	double mb;
	double up;
	double dn;
	SBollInfo(){
		memset(this, 0, sizeof(SBollInfo));
	}
	void Reset(){
		memset(this, 0, sizeof(SBollInfo));
	}
};

struct SFuturesAccountInfo
{
	double equity;	//账户权益
	double margin;	//保证金
	SFuturesAccountInfo(){
		memset(this, 0, sizeof(SFuturesAccountInfo));
	}
};

struct SFuturesTradeInfo
{
	std::string strClientOrderID;
	time_t timeStamp;	//委托时间
	std::string filledQTY;		//成交数量
	std::string orderID;//订单ID;
	double price;		//订单价格
	std::string status;	//订单状态(-1.撤单成功；0:等待成交 1:部分成交 2:全部成交 ）
	eFuturesTradeType tradeType;
	time_t waitClientOrderIDTime;
	int stopProfit;
	std::string size;
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
		status = "0";
		tradeType = eFuturesTradeType_OpenBull;
		waitClientOrderIDTime = 0;
		stopProfit = 0;
		size = "0";
	}
};

struct SFuturesTradePairInfo
{
	SFuturesTradeInfo open;
	SFuturesTradeInfo close;
};

// COKExFuturesDlg 对话框
class COKExFuturesDlg : public CDialogEx
{
// 构造
public:
	COKExFuturesDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_OKEXFUTURES_DIALOG };

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
private:
	clib::config m_config;
	std::string m_apiKey;
	std::string m_secretKey;
	std::string m_passphrase;
	std::set<std::string> m_setAllTestFile;
public:
	afx_msg void OnBnClickedButtonStart();
public:
	void AddKlineData(SKlineData& data);
	void OnRevTickerInfo(STickerData &data);
	void Pong();
	void OnLoginSuccess();
	void OnTradeFail(std::string& clientOrderID);
	void OnTradeSuccess(std::string& clientOrderID, std::string& serverOrderID);
	void UpdateTradeInfo(SFuturesTradeInfo& info);
	void SetHScroll();
private:
	void Test();
	void OnBollUpdate();
	void CheckBollTrend();
	void __CheckTrend_Normal();
	void __CheckTrend_ZhangKou();
	void __CheckTrend_ShouKou();
	void __CheckTrend_ShouKouChannel();

	void __CheckTrade_ZhangKou();
	void __CheckTrade_ShouKou();
	void __CheckTrade_ShouKouChannel();
	void __CheckTradeOrder();
	void __SetBollState(eBollTrend state, int nParam=0, double dParam=0.0);
	bool __CheckCanTrade(eFuturesTradeType eType);
	void __InitConfigCtrl();
	bool __SaveConfigCtrl();
public:
	bool m_bRun;
	time_t m_tListenPong;
private:
	std::vector<SKlineData> m_vecKlineData;
	std::vector<SBollInfo> m_vecBollData;
	eBollTrend m_eBollState;
	eBollTrend m_eLastBollState;
	int m_nZhangKouConfirmBar;
	double m_nZhangKouMinValue;
	int m_nShouKouConfirmBar;
	int m_nShouKouChannelConfirmBar;
	bool m_bZhangKouUp;
	SBollInfo m_curTickBoll;
	STickerData m_curTickData;
	int m_nZhangKouTradeCheckBar;
	std::list<SFuturesTradePairInfo> m_listTradePairInfo;
	bool m_bTest;
	bool m_bStopWhenFinish;
	//std::string m_curWaitClientOrderID;
	//std::string m_curOrderID;
public:
	int m_nBollCycle;				//布林线周期
	int m_nPriceDecimal;			//价格小数点精度
	int m_nZhangKouCheckCycle;		//布林张口确认周期
	int m_nZhangKouTrendCheckCycle;	//布林张口趋势确认周期 必须是奇数
	int m_nShouKouCheckCycle;		//布林收口确认周期
	int m_nZhangKouDoubleConfirmCycle;	//布林张口二次确认周期
	int m_nShoukouDoubleConfirmCycle;	//布林收口二次确认周期
	std::string m_strKlineCycle;		//布林线周期
	std::string m_strCoinType;			//货币类型
	std::string m_strFuturesCycle;		//合约周期
	std::string m_strFuturesTradeSize;	//下单张数
	int m_nKlineCycle;					//布林线周期对应秒数
	std::string m_strLeverage;					//合约倍数
	double m_stopLoss;					//止损比例
	double m_moveStopProfit;			//移动止盈比例
	std::string m_strStopLoss;					//止损比例
	std::string m_strMoveStopProfit;			//移动止盈比例
public:
	afx_msg void OnBnClickedButtonTest();
	afx_msg void OnDestroy();
	CComboBox m_combCoinType;
	CComboBox m_combLeverage;
	CEdit m_editFuturesCycle;
	CEdit m_editFuturesTradeSize;
	CEdit m_editStopLoss;
	CEdit m_editMoveStopProfit;
	CListBox m_ctrlListLog;
	afx_msg void OnBnClickedButtonStopWhenFinish();
};
