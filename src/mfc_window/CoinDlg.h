
// CoinDlg.h : ͷ�ļ�
//

#pragma once
#include "afxcmn.h"
#include <clib/lib/util/config.h>

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
	afx_msg void OnBnClickedButton1();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
public:
	void UpdateBalance();
	void UpdateEntrustDepth();
	CListCtrl m_listBalance;
	clib::config m_config;
	CListCtrl m_listCtrlSellEntrustDepth;
};
