
// OKExMartingaleDlg.h : 头文件
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

enum eTradeState
{
	eTradeState_WaitOpen,
	eTradeState_WaitTradeOrder,
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
	std::string strClientOrderID;
	std::string orderID;//订单ID;
	std::string price;	//订单价格
	std::string size;	//成交数量
	std::string side;	//交易类型
	std::string strTimeStamp;//委托时间
	time_t timeStamp;	//委托时间
	std::string filledSize;//已成交数量
	std::string filledNotional;//已成交金额
	std::string status;	//订单状态(open:未成交 part_filled:部分成交 filled:已成交 cancelled:已撤销 failure:订单失败）
	time_t waitClientOrderIDTime;
	SSPotTradeInfo()
	{
		Reset();
	}
	void Reset()
	{
		strClientOrderID = "";
		timeStamp = 0;
		orderID = "";
		price = "0";
		status = "0";
		waitClientOrderIDTime = 0;
		size = "0";
		side = "buy";
		filledSize = "0";
		filledNotional = "0";
		status = "";
		strTimeStamp = "";
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
	void OnTradeSuccess(std::string& strClientOrderID, std::string& serverOrderID);
private:
	void OnBollUpdate();
	void CheckBollTrend();
	void __CheckTrend_Normal();
	void __CheckTrend_ZhangKou();
	void __CheckTrend_ShouKou();
	void __CheckTrend_ShouKouChannel();
	void __SetBollState(eBollTrend state, int nParam = 0, double dParam = 0.0);
	void __CheckTrade();
public:
	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnBnClickedButtonTest();
	CListBox m_ctrlListLog;
private:
	clib::config m_config;
	std::string m_apiKey;
	std::string m_secretKey;
	std::string m_passphrase;
	std::set<std::string> m_setAllTestFile;
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
	int m_nZhangKouTradeCheckBar;
	bool m_bTest;
	STickerData m_curTickData;
	SSpotAccountInfo m_accountInfo;
	eTradeState m_eTradeState;
	double m_eachStepMoney;
	std::vector<SSPotTradePairInfo> m_vectorTradePairs;
public:
	bool m_bRun;
	time_t m_tListenPong;
public:
	int m_nBollCycle;				//布林线周期
	int m_nPriceDecimal;			//价格小数点精度
	int m_nVolumeDecimal;			//交易量小数点精度
	int m_nZhangKouCheckCycle;		//布林张口确认周期
	int m_nZhangKouTrendCheckCycle;	//布林张口趋势确认周期 必须是奇数
	int m_nShouKouCheckCycle;		//布林收口确认周期
	int m_nZhangKouDoubleConfirmCycle;	//布林张口二次确认周期
	int m_nShoukouDoubleConfirmCycle;	//布林收口二次确认周期
	std::string m_strKlineCycle;		//布林线周期
	std::string m_strCoinType;			//货币类型
	int m_nKlineCycle;					//布林线周期对应秒数
	std::string m_strMoneyType;//对手货币类型
	int m_martingaleStepCnt;			//马丁格尔交易次数
	double m_martingaleMovePersent;		//马丁格尔交易跌幅
	double m_tradeCharge;				//手续费
	double m_fixedMoneyCnt;	//参与交易的对手币数量
	std::string m_strInstrumentID;		//币对名称
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
