
// ManualOKExFuturesDlg.h : 头文件
//

#pragma once
#include <clib/lib/util/config.h>
#include "afxwin.h"
#include "afxcmn.h"
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



struct SFuturesPositionInfo
{
	std::string bullCount;		//多单数量
	std::string bullFreeCount;	//多单可平数量
	std::string bullPriceAvg;	//多单均价
	std::string bearCount;		//空单数量
	std::string bearFreeCount;	//空单可平数量
	std::string bearPriceAvg;	//空单均价
	std::string broken;			//爆仓价
	bool bValid;
	SFuturesPositionInfo() : bullCount("0"), bullFreeCount("0"), bullPriceAvg("0"), bearCount("0"), bearFreeCount("0"), bearPriceAvg("0"), broken("0"), bValid(false)
	{

	}
};

struct SFuturesAccountInfo
{
	std::string equity;	//账户权益
	bool bValid;
	std::string availBalance;
	SFuturesAccountInfo() : equity("0"), bValid(false), availBalance("0")
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
	time_t tLastALLFillTime;
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
		tLastALLFillTime = 0;
	}
	SFuturesTradeInfo& operator= (const SFuturesTradeInfo &t){
		if(this != &t){
			this->strClientOrderID = t.strClientOrderID;
			this->timeStamp = t.timeStamp;
			this->filledQTY = t.filledQTY;
			this->orderID = t.orderID;
			this->price = t.price;
			this->priceAvg = t.priceAvg;
			this->status = t.status;
			this->tradeType = t.tradeType;
			this->waitClientOrderIDTime = t.waitClientOrderIDTime;
			this->size = t.size;
			this->tLastUpdate = t.tLastUpdate;
			this->bModifyQTY = t.bModifyQTY;
			this->tLastALLFillTime = t.tLastALLFillTime;
		}
		return *this;
	}
};

struct SFuturesTradePairInfo
{
	SFuturesTradeInfo open;
	SFuturesTradeInfo close;
	std::string closePlanPrice;
	std::string closePlanSize;
	double triggerPrice;
	std::string strTriggerPrice;
	std::string openPrice;
	std::string openSize;
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
		triggerPrice = 0.0;
		openPrice = "";
		strTriggerPrice = "";
		openSize = "";
	}
	SFuturesTradePairInfo& operator= (const SFuturesTradePairInfo &t){
		if(this != &t){
			this->open = t.open;
			this->close = t.close;
			this->closePlanPrice = t.closePlanPrice;
			this->closePlanSize = t.closePlanSize;
			this->triggerPrice = t.triggerPrice;
			this->strTriggerPrice = t.strTriggerPrice;
			this->openPrice = t.openPrice;
			this->openSize = t.openSize;
		}
		return *this;
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
	void _UpdatePositionShow();
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
	void UpdatePositionInfo(SFuturesPositionInfo& info);
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
	bool m_bWaitDepthBegin;
	time_t m_tWaitNewSubDepth;
	std::vector<SFuturesTradePairInfo> m_vecTradePairInfo;

	std::string m_strCoinType;			//币类型
	std::string m_strMoneyType;			//货类型
	std::string m_strFuturesCycle;		//合约周期
	std::string m_strFuturesTradeSize;	//下单张数
	std::string m_strLeverage;			//合约倍数
	double m_dLeverage;					//合约倍数
	STickerData m_curTickData;
	SFuturesAccountInfo m_accountInfo;
	SFuturesPositionInfo m_positionInfo;
	double m_beginMoney;
	std::map<std::string, SFuturesDepth> m_mapDepthSell;
	std::map<std::string, SFuturesDepth> m_mapDepthBuy;
	std::vector<SFuturesDepth> m_vecDepthSell;
	std::vector<SFuturesDepth> m_vecDepthBuy;
public:
	CListCtrl m_listCtrlOrderOpen;
	CListCtrl m_listCtrlOrderClose;
	CComboBox m_combCoinType;
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
	CListCtrl m_listCtrlPostionInfo;
	CStatic m_staticAccountAvailInfo;
	CStatic m_staticCanOpenSize;
	afx_msg void OnBnClickedButtonSaveProfit();
	afx_msg void OnBnClickedButtonClearFreeLine();
	afx_msg void OnBnClickedButtonBearFirst();
	afx_msg void OnBnClickedButtonBullFirst();
	CComboBox m_combMoneyType;
	CEdit m_editTriggerPrice;
	CEdit m_editLeverage;
};
