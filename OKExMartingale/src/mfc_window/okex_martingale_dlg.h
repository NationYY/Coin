
// OKExMartingaleDlg.h : ͷ�ļ�
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
private:
	void OnBollUpdate();
	void CheckBollTrend();
	void __CheckTrend_Normal();
	void __CheckTrend_ZhangKou();
	void __CheckTrend_ShouKou();
	void __CheckTrend_ShouKouChannel();
	void __SetBollState(eBollTrend state, int nParam = 0, double dParam = 0.0);

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
public:
	bool m_bRun;
	time_t m_tListenPong;
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
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
