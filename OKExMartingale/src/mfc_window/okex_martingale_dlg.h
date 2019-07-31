
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
	int volume;			//成交量(张)
	double volumeByCurrency;//成交量(币)
	double ma5;
	double ma7;
	double ma15;
	double ma30;
	double ma60;
	double ma90;
	SKlineData(){
		memset(this, 0, sizeof(SKlineData));
	}
};

struct SFuturesDepth
{
	std::string price;
	std::string size;
	std::string brokenSize;
	std::string tradeNum;
	SFuturesDepth(): size(""), brokenSize(""), tradeNum(""), price("")
	{
	}
};

struct STickerData
{
	bool bValid;
	__int64 time;				//时间
	int volume;			//成交量(张)
	double sell;				//卖一价格
	double buy;					//买一价格
	double high;				//24小时最高价格
	double low;					//24小时最低价格
	double last;				//最新成交价格
	STickerData() :bValid(false), time(0), 
	 sell(0), buy(0), high(0), low(0), last(0), volume(0)
	{
	}
};

struct SFuturesAccountInfo
{
	std::string equity;	//账户权益
	std::string availBalance;//账户余额
	bool bValid;
	SFuturesAccountInfo() : equity("0"), bValid(false), availBalance("0")
	{

	}
};

struct SFuturesPositionInfo
{
	std::string bullCount;		//多单数量
	std::string bullFreeCount;	//多单可平数量
	std::string bullPriceAvg;	//多单均价
	std::string bullMargin;		//多单保证金
	std::string bearCount;		//空单数量
	std::string bearFreeCount;	//空单可平数量
	std::string bearPriceAvg;	//空单均价
	std::string bearMargin;		//空单保证金
	std::string broken;			//爆仓价
	bool bValid;
	SFuturesPositionInfo() : bullCount("0"), bullFreeCount("0"), bullPriceAvg("0"), bearCount("0"), bearFreeCount("0"), bearPriceAvg("0"), broken("0"), bValid(false),
		bullMargin("0"), bearMargin("0")
	{

	}
};


struct SFuturesTradeInfo
{
	std::string strClientOrderID;
	time_t timeStamp;	//委托时间
	std::string filledQTY;		//成交数量
	std::string closeSize;
	std::string orderID;//订单ID;
	double price;		//订单价格
	double priceAvg;	//成交均价
	std::string state;	//订单状态(-1.撤单成功；0:等待成交 1:部分成交 2:全部成交)
	eFuturesTradeType tradeType;
	bool bBeginStopProfit;
	std::string size;
	int stopProfit;
	double minPrice;
	double maxPrice;
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
		state = "";
		tradeType = eFuturesTradeType_OpenBull;
		size = "0";
		stopProfit = 0;
		minPrice = 0.0;
		maxPrice = 0.0;
		bBeginStopProfit = false;
		closeSize = "0";
	}
	SFuturesTradeInfo& operator= (const SFuturesTradeInfo &t){
		if(this != &t){
			this->strClientOrderID = t.strClientOrderID;
			this->timeStamp = t.timeStamp;
			this->filledQTY = t.filledQTY;
			this->orderID = t.orderID;
			this->price = t.price;
			this->priceAvg = t.priceAvg;
			this->state = t.state;
			this->tradeType = t.tradeType;
			this->size = t.size;
			this->stopProfit = t.stopProfit;
			this->minPrice = t.minPrice;
			this->maxPrice = t.maxPrice;
			this->bBeginStopProfit = t.bBeginStopProfit;
			this->closeSize = t.closeSize;
		}
		return *this;
	}
};

struct SFuturesTradePairInfo
{
	SFuturesTradeInfo open;
	SFuturesTradeInfo close;
};

// COKExMartingaleDlg 对话框
class COKExMartingaleDlg : public CDialog
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
	void ComplementedKLine(time_t tNowKlineTick, int kLineCnt);
	void OnRevTickerInfo(STickerData &data);
	void Pong();
	void OnLoginSuccess();
	void SetHScroll();
	void UpdateAccountInfo(SFuturesAccountInfo& info);
	void UpdatePositionInfo(SFuturesPositionInfo& info);
	void UpdateTradeInfo(SFuturesTradeInfo& info);
	void __InitConfigCtrl();
	bool __SaveConfigCtrl();
	void ClearDepth();
	void UpdateDepthInfo(bool bBuy, SFuturesDepth& info);
	bool CheckDepthInfo(int checkNum, std::string& checkSrc);
private:
	void _Update3Sec();
	void _Update15Sec();
	void _LogicThread();
	void __CheckTrade();
	void _UpdateProfitShow();
	void _UpdateTradeShow();
	void _UpdatePositionShow();
	void _SetTradeState(eTradeState state);
	bool _CheckMoney(std::string& strCurrency);
	void _SaveData();
	void _LoadData();
	void _QueryOrderInfo(std::string& orderID, const char* szLogTitle, const char* state_check=NULL);
	void _MakeMA();
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
	STickerData m_curTickData;
	SFuturesAccountInfo m_accountInfo;
	SFuturesPositionInfo m_positionInfo;
	eTradeState m_eTradeState;
	std::vector<SFuturesTradePairInfo> m_vectorTradePairs;
	int m_curOpenFinishIndex;		//当前open交易完成的序号
	time_t m_tOpenTime;
	bool m_bStopWhenFinish;
	double m_beginMoney;
	boost::thread m_logicThread;
	bool m_bExit;
	time_t m_tLastUpdate15Sec;
	time_t m_tLastUpdate3Sec;
	int m_nStopProfitTimes;
	int m_nFinishTimes;
	time_t m_tWaitNewSubDepth;
	bool m_bOpenBull;
	bool m_bSaveData;
public:
	bool m_bRun;
	bool m_bLoginSuccess;
	time_t m_tListenPong;
public:
	int m_nPriceDecimal;			//价格小数点精度
	std::string m_strKlineCycle;		//K线周期
	std::string m_strCoinType;			//货币类型
	int m_nKlineCycle;					//K线周期对应秒数
	int m_nFirstTradeSize;				//首次下单张数
	int m_martingaleStepCnt;			//马丁格尔交易次数
	double m_martingaleMovePersent;		//马丁格尔交易跌幅
	double m_stopProfitFactor;			//头单止盈系数
	std::string m_strFuturesCycle;		//合约周期
	std::string m_strLeverage;			//合约倍数
	int m_nLeverage;					//合约倍数
	bool m_bSwapFutures;				//是否永续合约
	int m_nTrendType;					//0做多 1做空 2自动
	bool m_bStopProfitMove;
	CComboBox m_combCoinType;
	CEdit m_editMartingaleStepCnt;
	CEdit m_editMartingaleMovePersent;
	CEdit m_editStopProfitFactor;
	CEdit m_editCoin;
	afx_msg void OnBnClickedButtonStopWhenFinish();
	CEdit m_editProfit;
	CStatic m_staticCoin;
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
	CEdit m_editFirstTradeSize;
	std::map<std::string, SFuturesDepth> m_mapDepthSell;
	std::map<std::string, SFuturesDepth> m_mapDepthBuy;
	bool m_bWaitDepthBegin;
	bool m_bFirstKLine;
	CComboBox m_combKLineCycle;
	CButton m_btnTrendBull;
	CButton m_btnTrendBear;
	CButton m_btnTrendAuto;
	afx_msg void OnBnClickedTrendBull();
	afx_msg void OnBnClickedTrendBear();
	afx_msg void OnBnClickedTrendAuto();
	bool m_bNeedSubscribe;
	CStatic m_staticDingDan;
	CListCtrl m_listCtrlPostionInfo;
	afx_msg void OnBnClickedButtonUpdateTrend();
};
