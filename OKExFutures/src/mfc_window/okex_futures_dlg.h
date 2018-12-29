
// okex_futures_dlg.h : ͷ�ļ�
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
	eBollTrend_Normal,			//��ת
	eBollTrend_ShouKou,			//�տ�
	eBollTrend_ShouKouChannel,	//�տ�ͨ��
	eBollTrend_ZhangKou,		//�ſ�
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
	SKlineData(){
		memset(this, 0, sizeof(SKlineData));
	}
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
	double equity;	//�˻�Ȩ��
	double margin;	//��֤��
	SFuturesAccountInfo(){
		memset(this, 0, sizeof(SFuturesAccountInfo));
	}
};

struct SFuturesTradeInfo
{
	std::string strClientOrderID;
	time_t timeStamp;	//ί��ʱ��
	std::string filledQTY;		//�ɽ�����
	std::string orderID;//����ID;
	double price;		//�����۸�
	std::string status;	//����״̬(-1.�����ɹ���0:�ȴ��ɽ� 1:���ֳɽ� 2:ȫ���ɽ� ��
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

// COKExFuturesDlg �Ի���
class COKExFuturesDlg : public CDialogEx
{
// ����
public:
	COKExFuturesDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_OKEXFUTURES_DIALOG };

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
	//std::string m_curWaitClientOrderID;
	//std::string m_curOrderID;
public:
	int m_nBollCycle;				//����������
	int m_nPriceDecimal;			//�۸�С���㾫��
	int m_nZhangKouCheckCycle;		//�����ſ�ȷ������
	int m_nZhangKouTrendCheckCycle;	//�����ſ�����ȷ������ ����������
	int m_nShouKouCheckCycle;		//�����տ�ȷ������
	int m_nZhangKouDoubleConfirmCycle;	//�����ſڶ���ȷ������
	int m_nShoukouDoubleConfirmCycle;	//�����տڶ���ȷ������
	std::string m_strKlineCycle;		//����������
	std::string m_strCoinType;			//��������
	std::string m_strFuturesCycle;		//��Լ����
	std::string m_strFuturesTradeSize;	//�µ�����
	int m_nKlineCycle;					//���������ڶ�Ӧ����
	std::string m_strLeverage;					//��Լ����
	double m_stopLoss;					//ֹ�����
	double m_moveStopProfit;			//�ƶ�ֹӯ����
	std::string m_strStopLoss;					//ֹ�����
	std::string m_strMoveStopProfit;			//�ƶ�ֹӯ����
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
};