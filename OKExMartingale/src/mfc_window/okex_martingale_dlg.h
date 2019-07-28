
// OKExMartingaleDlg.h : 头文件
//

#pragma once
#include <clib/lib/util/config.h>
#include "afxwin.h"
#include "afxcmn.h"
#define DOUBLE_PRECISION 0.00000001
enum eTradeState
{
	eTradeState_WaitOpen,
	eTradeState_Trading
};

struct SKlineData
{
	__int64 time;		//时间
	char szTime[20];
	double openPrice;	//开盘价
	double highPrice;	//最高价
	double lowPrice;	//最低价
	double closePrice;	//收盘价
	std::string volume;	//成交量
	SKlineData() :openPrice(0), highPrice(0), lowPrice(0), closePrice(0),
	 volume("0"), time(0)
	{
		memset(szTime, 0, 20);
	}
};

struct STickerData
{
	bool bValid;
	__int64 time;				//时间
	std::string baseVolume24h;	//24小时成交量，按交易货币统计
	std::string quoteVolume24h; //24小时成交量，按计价货币统计
	double sell;				//卖一价格
	double buy;					//买一价格
	double high;				//24小时最高价格
	double low;					//24小时最低价格
	double last;				//最新成交价格
	STickerData() :bValid(false), time(0), baseVolume24h("0"), quoteVolume24h("0"),
	 sell(0), buy(0), high(0), low(0), last(0)
	{
	}
};

struct SSpotAccountInfo
{
	bool bValid;
	std::string currency;	//币种
	double balance;			//余额
	double hold;			//冻结(不可用)
	double available;		//可用于交易或资金划转的数量
	SSpotAccountInfo() : bValid(false), currency(""), balance(0), hold(0), available(0)
	{
	}
};

struct SSPotTradeInfo
{
	std::string orderID;//订单ID;
	std::string price;	//订单价格
	std::string size;	//成交数量
	std::string side;	//交易类型
	std::string strTimeStamp;//委托时间
	time_t timeStamp;	//委托时间
	std::string filledSize;//已成交数量
	std::string filledNotional;//已成交金额
	std::string status;	//订单状态(open:未成交 part_filled:部分成交 filled:已成交 cancelled:已撤销 failure:订单失败）
	std::string closeSize;//已卖出数量
	bool bBeginStopProfit;
	int stopProfit;
	double minPrice;
	double maxPrice;
	SSPotTradeInfo()
	{
		Reset();
	}
	void Reset()
	{
		timeStamp = 0;
		orderID = "";
		price = "0";
		status = "0";
		size = "0";
		side = "buy";
		filledSize = "0";
		filledNotional = "0";
		closeSize = "0";
		status = "";
		strTimeStamp = "";
		bBeginStopProfit = false;
		stopProfit = 0;
		minPrice = 0.0;
		maxPrice = 0.0;
	}
};

struct SSPotTradePairInfo
{
	SSPotTradeInfo open;
	SSPotTradeInfo close;
};

// COKExMartingaleDlg 对话框
class COKExMartingaleDlg : public CDialogEx
{
// 构造
public:
	COKExMartingaleDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_OKEXMARTINGALE_DIALOG };

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
	void AddKlineData(SKlineData& data);
	void OnRevTickerInfo(STickerData &data);
	void Pong();
	void OnLoginSuccess();
	void SetHScroll();
	void UpdateAccountInfo(SSpotAccountInfo& info);
	void UpdateTradeInfo(SSPotTradeInfo& info);
	void __InitConfigCtrl();
	bool __SaveConfigCtrl();
private:
	void _Update15Sec();
	void _LogicThread();
	void __CheckTrade();
	void _UpdateProfitShow();
	void _UpdateTradeShow();
	void _SetTradeState(eTradeState state);
public:
	afx_msg void OnBnClickedButtonStart();
	CListBox m_ctrlListLog;
private:
	clib::config m_config;
	std::string m_apiKey;
	std::string m_secretKey;
	std::string m_passphrase;
private:
	std::vector<SKlineData> m_vecKlineData;
	int m_nZhangKouConfirmBar;
	double m_nZhangKouMinValue;
	int m_nShouKouConfirmBar;
	int m_nShouKouChannelConfirmBar;
	bool m_bZhangKouUp;
	int m_nZhangKouTradeCheckBar;
	STickerData m_curTickData;
	SSpotAccountInfo m_coinAccountInfo;
	SSpotAccountInfo m_moneyAccountInfo;
	eTradeState m_eTradeState;
	std::vector<SSPotTradePairInfo> m_vectorTradePairs;
	int m_curOpenFinishIndex;		//当前open交易完成的序号
	time_t m_tOpenTime;
	bool m_bStopWhenFinish;
	double m_beginMoney;
	boost::thread m_logicThread;
	bool m_bExit;
	time_t m_tLastUpdate15Sec;
	int m_nStopProfitTimes;
	int m_nFinishTimes;
public:
	bool m_bRun;
	time_t m_tListenPong;
public:
	int m_nPriceDecimal;			//价格小数点精度
	std::string m_strKlineCycle;		//K线周期
	std::string m_strCoinType;			//货币类型
	int m_nKlineCycle;					//K线周期对应秒数
	int m_martingaleStepCnt;			//马丁格尔交易次数
	double m_martingaleMovePersent;		//马丁格尔交易跌幅
	double m_fixedMoneyCnt;	//参与交易的对手币数量
	double m_stopProfitFactor;			//头单止盈系数
	std::string m_strFuturesCycle;		//合约周期
	std::string m_strFuturesTradeSize;	//下单张数
	std::string m_strLeverage;			//合约倍数
	int m_nLeverage;					//合约倍数
	bool m_bSwapFutures;				//是否永续合约
	bool m_bStopProfitMove;
	CComboBox m_combCoinType;
	CEdit m_editMartingaleStepCnt;
	CEdit m_editMartingaleMovePersent;
	CEdit m_editFixedMoneyCnt;
	CEdit m_editStopProfitFactor;
	CEdit m_editCoin;
	afx_msg void OnBnClickedButtonStopWhenFinish();
	CEdit m_editProfit;
	CStatic m_staticCoin;
	CStatic m_staticMoney;
	CEdit m_editMoney;
	CEdit m_editCost;
	afx_msg void OnBnClickedButtonUpdateCost();
	CListCtrl m_listCtrlOpen;
	CListCtrl m_listCtrlClose;
	afx_msg void OnDestroy();
	CStatic m_staticPrice;
	CStatic m_staticStopProfitTimes;
	CStatic m_staticFinishTimes;
	CButton m_btnStopProfitMove;
	CButton m_btnStopProfitFix;
	afx_msg void OnBnClickedStopProfitMove();
	afx_msg void OnBnClickedRadioStopProfitFix();
	CEdit m_editFuturesCycle;
	CComboBox m_combLeverage;
	CComboBox m_combFuturesType;
};
