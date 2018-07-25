
// CoinDlg.h : ͷ�ļ�
//

#pragma once
#include "afxcmn.h"
#include <clib/lib/util/config.h>
#include "afxwin.h"
enum eTimerType
{
	eTimerType_APIUpdate,
	eTimerType_Balance,
	eTimerType_EntrustDepth,
	eTimerType_Trade,
};
// CCoinDlg �Ի���
class CCoinDlg : public CDialogEx
{
// ����
public:
	CCoinDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_COIN_DIALOG };

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
	afx_msg void OnBnClickedButtonBegin();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
public:
	void UpdateBalance();
	void UpdateEntrustDepth();
	CListCtrl m_listBalance;
	clib::config m_config;
	CListCtrl m_listCtrlEntrustDepth;
	CComboBox m_cbMarketType;
	double m_tradeVolume;
	int m_tradeFrequency;
	CStatic m_staticUpdateTime;
	CButton m_btnHightSpeed;
	CButton m_btnNormalSpeed;
	afx_msg void OnBnClickedRadioHightSpeed();
	afx_msg void OnBnClickedRadioNormalSpeed();
	int m_webSokectFailTimes;
	afx_msg void OnBnClickedButtonStop();
	bool m_bIsRun;
};
