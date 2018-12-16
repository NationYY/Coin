
// OKExFuturesDlg.h : ͷ�ļ�
//

#pragma once

#include <clib/lib/util/config.h>
enum eTimerType
{
	eTimerType_APIUpdate,
	eTimerType_EntrustDepth,
	eTimerType_Ping,
};

struct SKlineData
{
	__int64 time;		//ʱ��
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

struct SBollInfo
{
	__int64 time;
	double mb;
	double up;
	double dn;
	SBollInfo(){
		memset(this, 0, sizeof(SBollInfo));
	}
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
	std::string m_accessKey;
	std::string m_secretKey;
	int m_nBollCycle;
	int m_nPriceDecimal;
public:
	afx_msg void OnBnClickedButtonStart();
public:
	void AddKlineData(SKlineData& data);
private:
	void Test();
	void OnBollUpdate();
private:
	std::vector<SKlineData> m_vecKlineData;
	std::vector<SBollInfo> m_vecBollData;
};
