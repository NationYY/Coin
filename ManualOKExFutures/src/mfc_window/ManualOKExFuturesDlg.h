
// ManualOKExFuturesDlg.h : ͷ�ļ�
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
	__int64 time;		//ʱ��
	int volume;			//�ɽ���(��)
	double sell;		//��һ�۸�
	double buy;			//��һ�۸�
	double high;		//24Сʱ��߼۸�
	double low;			//24Сʱ��ͼ۸�
	double last;		//���³ɽ��۸�
	STickerData(){
		memset(this, 0, sizeof(STickerData));
	}
};



struct SFuturesPositionInfo
{
	std::string bullCount;		//�൥����
	std::string bullFreeCount;	//�൥��ƽ����
	std::string bullPriceAvg;	//�൥����
	std::string bearCount;		//�յ�����
	std::string bearFreeCount;	//�յ���ƽ����
	std::string bearPriceAvg;	//�յ�����
	std::string broken;			//���ּ�
	bool bValid;
	SFuturesPositionInfo() : bullCount("0"), bullFreeCount("0"), bullPriceAvg("0"), bearCount("0"), bearFreeCount("0"), bearPriceAvg("0"), broken("0"), bValid(false)
	{

	}
};

struct SFuturesAccountInfo
{
	std::string equity;	//�˻�Ȩ��
	bool bValid;
	std::string availBalance;
	SFuturesAccountInfo() : equity("0"), bValid(false), availBalance("0")
	{

	}
};

struct SFuturesTradeInfo
{
	std::string strClientOrderID;
	time_t timeStamp;	//ί��ʱ��
	std::string filledQTY;		//�ɽ�����
	std::string orderID;//����ID;
	double price;		//�����۸�
	double priceAvg;	//�ɽ�����
	std::string status;	//����״̬(-1.�����ɹ���0:�ȴ��ɽ� 1:���ֳɽ� 2:ȫ���ɽ� ��
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

// CManualOKExFuturesDlg �Ի���
class CManualOKExFuturesDlg : public CDialog
{
// ����
public:
	CManualOKExFuturesDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_MANUALOKEXFUTURES_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
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

	std::string m_strCoinType;			//������
	std::string m_strMoneyType;			//������
	std::string m_strFuturesCycle;		//��Լ����
	std::string m_strFuturesTradeSize;	//�µ�����
	std::string m_strLeverage;			//��Լ����
	double m_dLeverage;					//��Լ����
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
