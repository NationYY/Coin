
// OKExMartingaleDlg.h : ͷ�ļ�
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
	__int64 time;		//ʱ��
	char szTime[20];
	double openPrice;	//���̼�
	double highPrice;	//��߼�
	double lowPrice;	//��ͼ�
	double closePrice;	//���̼�
	int volume;			//�ɽ���(��)
	double volumeByCurrency;//�ɽ���(��)
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
	__int64 time;				//ʱ��
	int volume;			//�ɽ���(��)
	double sell;				//��һ�۸�
	double buy;					//��һ�۸�
	double high;				//24Сʱ��߼۸�
	double low;					//24Сʱ��ͼ۸�
	double last;				//���³ɽ��۸�
	STickerData() :bValid(false), time(0), 
	 sell(0), buy(0), high(0), low(0), last(0), volume(0)
	{
	}
};

struct SFuturesAccountInfo
{
	std::string equity;	//�˻�Ȩ��
	std::string availBalance;//�˻����
	bool bValid;
	SFuturesAccountInfo() : equity("0"), bValid(false), availBalance("0")
	{

	}
};

struct SFuturesPositionInfo
{
	std::string bullCount;		//�൥����
	std::string bullFreeCount;	//�൥��ƽ����
	std::string bullPriceAvg;	//�൥����
	std::string bullMargin;		//�൥��֤��
	std::string bearCount;		//�յ�����
	std::string bearFreeCount;	//�յ���ƽ����
	std::string bearPriceAvg;	//�յ�����
	std::string bearMargin;		//�յ���֤��
	std::string broken;			//���ּ�
	bool bValid;
	SFuturesPositionInfo() : bullCount("0"), bullFreeCount("0"), bullPriceAvg("0"), bearCount("0"), bearFreeCount("0"), bearPriceAvg("0"), broken("0"), bValid(false),
		bullMargin("0"), bearMargin("0")
	{

	}
};


struct SFuturesTradeInfo
{
	std::string strClientOrderID;
	time_t timeStamp;	//ί��ʱ��
	std::string filledQTY;		//�ɽ�����
	std::string closeSize;
	std::string orderID;//����ID;
	double price;		//�����۸�
	double priceAvg;	//�ɽ�����
	std::string state;	//����״̬(-1.�����ɹ���0:�ȴ��ɽ� 1:���ֳɽ� 2:ȫ���ɽ�)
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

// COKExMartingaleDlg �Ի���
class COKExMartingaleDlg : public CDialog
{
// ����
public:
	COKExMartingaleDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_OKEXMARTINGALE_DIALOG };

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
	int m_curOpenFinishIndex;		//��ǰopen������ɵ����
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
	int m_nPriceDecimal;			//�۸�С���㾫��
	std::string m_strKlineCycle;		//K������
	std::string m_strCoinType;			//��������
	int m_nKlineCycle;					//K�����ڶ�Ӧ����
	int m_nFirstTradeSize;				//�״��µ�����
	int m_martingaleStepCnt;			//��������״���
	double m_martingaleMovePersent;		//��������׵���
	double m_stopProfitFactor;			//ͷ��ֹӯϵ��
	std::string m_strFuturesCycle;		//��Լ����
	std::string m_strLeverage;			//��Լ����
	int m_nLeverage;					//��Լ����
	bool m_bSwapFutures;				//�Ƿ�������Լ
	int m_nTrendType;					//0���� 1���� 2�Զ�
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
