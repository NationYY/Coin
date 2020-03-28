
// ManualOKExSpotDlg.h : 头文件
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

struct SSpotAccountInfo
{
	std::string balance;//余额
	std::string hold;		//冻结
	std::string currency;	//币种
	std::string available; //可用于交易的数量
	SSpotAccountInfo() : balance("0"), hold("0"), currency(""), available("")
	{

	}
};

struct SSpotTradeInfo
{
	std::string strClientOrderID;
	std::string orderID;//订单ID;
	time_t timeStamp;	//委托时间
	std::string filledSize;		//成交数量
	double price;		//订单价格
	std::string status;	//订单状态(open:未成交 part_filled:部分成交 filled:已成交 cancelled:已撤销 failure:订单失败）
	std::string tradeType;
	time_t waitClientOrderIDTime;
	std::string size;
	time_t tLastUpdate;
	bool bModifyQTY;
	time_t tLastALLFillTime;
	time_t tLastCheck;
	SSpotTradeInfo()
	{
		Reset();
	}
	void Reset()
	{
		strClientOrderID = "";
		timeStamp = 0;
		filledSize = "0";
		orderID = "";
		price = 0.0;
		status = "";
		tradeType = "buy";
		waitClientOrderIDTime = 0;
		size = "0";
		tLastUpdate = 0;
		bModifyQTY = false;
		tLastALLFillTime = 0;
		tLastCheck = 0;
	}
	SSpotTradeInfo& operator= (const SSpotTradeInfo &t){
		if(this != &t){
			this->strClientOrderID = t.strClientOrderID;
			this->timeStamp = t.timeStamp;
			this->filledSize = t.filledSize;
			this->orderID = t.orderID;
			this->price = t.price;
			this->status = t.status;
			this->tradeType = t.tradeType;
			this->waitClientOrderIDTime = t.waitClientOrderIDTime;
			this->size = t.size;
			this->tLastUpdate = t.tLastUpdate;
			this->tLastCheck = t.tLastCheck;
			this->bModifyQTY = t.bModifyQTY;
			this->tLastALLFillTime = t.tLastALLFillTime;
		}
		return *this;
	}
};

struct SSpotTradePairInfo
{
	SSpotTradeInfo open;
	SSpotTradeInfo close;
	std::string closePlanPrice;
	std::string closePlanSize;
	SSpotTradePairInfo()
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
	SSpotTradePairInfo& operator= (const SSpotTradePairInfo &t){
		if(this != &t){
			this->open = t.open;
			this->close = t.close;
			this->closePlanPrice = t.closePlanPrice;
			this->closePlanSize = t.closePlanSize;
		}
		return *this;
	}
};

struct SSpotDepth
{
	std::string price;
	std::string size;
	std::string showSize;
	std::string tradeNum;
	SSpotDepth(): size(""), showSize(""), tradeNum(""), price("")
	{
	}
};

// CManualOKExSpotDlg 对话框
class CManualOKExSpotDlg : public CDialog
{
// 构造
public:
	CManualOKExSpotDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_MANUALOKEXSPOT_DIALOG };

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
	void _OpenOrder(std::string& type);
	void _SaveData();
	void _CheckAllOrder();
public:
	void OnRevTickerInfo(STickerData &data);
	void Pong();
	void OnLoginSuccess();
	void UpdateTradeInfo(SSpotTradeInfo& info);
	void UpdateAccountInfo(SSpotAccountInfo& info);
	void ClearDepth();
	void UpdateDepthInfo(bool bBuy, SSpotDepth& info);
	bool CheckDepthInfo(int checkNum, std::string& checkSrc);
	void OnTradeSuccess(std::string& clientOrderID, std::string& serverOrderID);
	void OnTradeFail(std::string& clientOrderID);
	void OnCancelSuccess(std::string& orderID);
	void SetHScroll();
public:
	time_t m_tListenPong;
	bool m_bRun;
	clib::config m_config;
	std::string m_apiKey;
	std::string m_secretKey;
	std::string m_passphrase;
	int m_nPriceDecimal;
	int m_nSizeDecimal;
	bool m_bWaitDepthBegin;
	time_t m_tWaitNewSubDepth;
	std::vector<SSpotTradePairInfo> m_vecTradePairInfo;

	std::string m_strInstrumentType;	//交易对信息
	std::string m_strFirstCoinType;		//主货币类型
	std::string m_strSecondCoinType;	//从货币类型

	std::string m_strSpotTradeSize;		//下单数量
	STickerData m_curTickData;
	std::list<SSpotAccountInfo> m_listAccountInfo;
	std::map<std::string, SSpotDepth> m_mapDepthSell;
	std::map<std::string, SSpotDepth> m_mapDepthBuy;
	std::map<time_t, std::list<std::string> > m_mapFindTradeinfo;
public:
	CListCtrl m_listCtrlOrderOpen;
	CListCtrl m_listCtrlOrderClose;
	CComboBox m_combCoinType;
	CEdit m_editFuturesCycle;
	CEdit m_editSpotTradeSize;
	CComboBox m_combFuturesType;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedStart();
	CListBox m_ctrlListLog;
	CListCtrl m_listCtrlDepth;
	CEdit m_editTradePrice;
	afx_msg void OnBnClickedButtonBuy();
	afx_msg void OnBnClickedButtonSell();
	CEdit m_editClosePrice;
	CEdit m_editCloseSize;
	afx_msg void OnBnClickedButtonClose();
	afx_msg void OnBnClickedButtonCancel();
	CListCtrl m_listCtrlAccountInfo;
	afx_msg void OnBnClickedButtonClearFreeLine();
	afx_msg void OnBnClickedButtonBearFirst();
	afx_msg void OnBnClickedButtonBullFirst();
	CButton m_checkBoxMargin;
	bool m_bMargin;
	CEdit m_editFixProfit;
};
