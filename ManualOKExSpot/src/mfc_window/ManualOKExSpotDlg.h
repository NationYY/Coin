
// ManualOKExSpotDlg.h : ͷ�ļ�
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

struct SSpotAccountInfo
{
	std::string balance;//���
	std::string hold;		//����
	std::string currency;	//����
	std::string available; //�����ڽ��׵�����
	SSpotAccountInfo() : balance("0"), hold("0"), currency(""), available("")
	{

	}
};

struct SSpotTradeInfo
{
	std::string strClientOrderID;
	std::string orderID;//����ID;
	time_t timeStamp;	//ί��ʱ��
	std::string filledSize;		//�ɽ�����
	double price;		//�����۸�
	std::string status;	//����״̬(open:δ�ɽ� part_filled:���ֳɽ� filled:�ѳɽ� cancelled:�ѳ��� failure:����ʧ�ܣ�
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

// CManualOKExSpotDlg �Ի���
class CManualOKExSpotDlg : public CDialog
{
// ����
public:
	CManualOKExSpotDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_MANUALOKEXSPOT_DIALOG };

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

	std::string m_strInstrumentType;	//���׶���Ϣ
	std::string m_strFirstCoinType;		//����������
	std::string m_strSecondCoinType;	//�ӻ�������

	std::string m_strSpotTradeSize;		//�µ�����
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
