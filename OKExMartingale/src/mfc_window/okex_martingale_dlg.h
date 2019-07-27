
// OKExMartingaleDlg.h : ͷ�ļ�
//

#pragma once
#include <clib/lib/util/config.h>
#include "afxwin.h"
#include "afxcmn.h"
#define DOUBLE_PRECISION 0.00000001
enum eBollTrend
{
	eBollTrend_Normal,			//��ת
	eBollTrend_ShouKou,			//�տ�
	eBollTrend_ShouKouChannel,	//�տ�ͨ��
	eBollTrend_ZhangKou,		//�ſ�
};

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
	std::string volume;	//�ɽ���
	SKlineData() :openPrice(0), highPrice(0), lowPrice(0), closePrice(0),
	 volume("0"), time(0)
	{
		memset(szTime, 0, 20);
	}
};

struct STickerData
{
	bool bValid;
	__int64 time;				//ʱ��
	std::string baseVolume24h;	//24Сʱ�ɽ����������׻���ͳ��
	std::string quoteVolume24h; //24Сʱ�ɽ��������Ƽۻ���ͳ��
	double sell;				//��һ�۸�
	double buy;					//��һ�۸�
	double high;				//24Сʱ��߼۸�
	double low;					//24Сʱ��ͼ۸�
	double last;				//���³ɽ��۸�
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
	std::string currency;	//����
	double balance;			//���
	double hold;			//����(������)
	double available;		//�����ڽ��׻��ʽ�ת������
	SSpotAccountInfo() : bValid(false), currency(""), balance(0), hold(0), available(0)
	{
	}
};

struct SSPotTradeInfo
{
	std::string orderID;//����ID;
	std::string price;	//�����۸�
	std::string size;	//�ɽ�����
	std::string side;	//��������
	std::string strTimeStamp;//ί��ʱ��
	time_t timeStamp;	//ί��ʱ��
	std::string filledSize;//�ѳɽ�����
	std::string filledNotional;//�ѳɽ����
	std::string status;	//����״̬(open:δ�ɽ� part_filled:���ֳɽ� filled:�ѳɽ� cancelled:�ѳ��� failure:����ʧ�ܣ�
	std::string closeSize;//����������
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

// COKExMartingaleDlg �Ի���
class COKExMartingaleDlg : public CDialogEx
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
	void OnBollUpdate();
	void CheckBollTrend();
	void __CheckTrend_Normal();
	void __CheckTrend_ZhangKou();
	void __CheckTrend_ShouKou();
	void __CheckTrend_ShouKouChannel();
	void __SetBollState(eBollTrend state, int nParam = 0, double dParam = 0.0);
	void __CheckTrade();
	void _UpdateProfitShow();
	void _UpdateTradeShow();
	bool _CheckMoney(std::string& strCurrency);
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
	std::vector<SBollInfo> m_vecBollData;
	eBollTrend m_eBollState;
	eBollTrend m_eLastBollState;
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
	int m_curOpenFinishIndex;		//��ǰopen������ɵ����
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
	int m_nBollCycle;				//����������
	int m_nPriceDecimal;			//�۸�С���㾫��
	int m_nVolumeDecimal;			//������С���㾫��
	int m_nZhangKouCheckCycle;		//�����ſ�ȷ������
	int m_nZhangKouTrendCheckCycle;	//�����ſ�����ȷ������ ����������
	int m_nShouKouCheckCycle;		//�����տ�ȷ������
	int m_nZhangKouDoubleConfirmCycle;	//�����ſڶ���ȷ������
	int m_nShoukouDoubleConfirmCycle;	//�����տڶ���ȷ������
	std::string m_strKlineCycle;		//����������
	std::string m_strCoinType;			//��������
	int m_nKlineCycle;					//���������ڶ�Ӧ����
	std::string m_strMoneyType;//���ֻ�������
	int m_martingaleStepCnt;			//��������״���
	double m_martingaleMovePersent;		//��������׵���
	double m_fixedMoneyCnt;	//���뽻�׵Ķ��ֱ�����
	std::string m_strInstrumentID;		//�Ҷ�����
	double m_stopProfitFactor;			//ͷ��ֹӯϵ��
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
