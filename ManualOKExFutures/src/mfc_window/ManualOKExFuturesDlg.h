
// ManualOKExFuturesDlg.h : ͷ�ļ�
//

#pragma once
#include <clib/lib/util/config.h>
#include "afxwin.h"
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

struct SFuturesAccountInfo
{
	std::string equity;	//�˻�Ȩ��
	bool bValid;
	SFuturesAccountInfo() : equity("0"), bValid(false)
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
	}
};

struct SFuturesTradePairInfo
{
	SFuturesTradeInfo open;
	SFuturesTradeInfo close;
	std::string closePlanPrice;
	std::string closePlanSize;
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
	std::vector<SFuturesTradePairInfo> m_vecTradePairInfo;

	std::string m_strCoinType;			//��������
	std::string m_strFuturesCycle;		//��Լ����
	std::string m_strFuturesTradeSize;	//�µ�����
	std::string m_strLeverage;			//��Լ����
	int m_nLeverage;					//��Լ����
	STickerData m_curTickData;
	SFuturesAccountInfo m_accountInfo;
	double m_beginMoney;
	std::map<std::string, SFuturesDepth> m_mapDepthSell;
	std::map<std::string, SFuturesDepth> m_mapDepthBuy;
public:
	CListCtrl m_listCtrlOrderOpen;
	CListCtrl m_listCtrlOrderClose;
	CComboBox m_combCoinType;
	CComboBox m_combLeverage;
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
};
