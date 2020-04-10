
// CrossPlatformQTDlg.h : ͷ�ļ�
//

#pragma once
#include <clib/lib/util/config.h>
#include "afxwin.h"
enum eTimerType
{
	eTimerType_APIUpdate,
	eTimerType_Ping,
};
// CCrossPlatformQTDlg �Ի���
class CCrossPlatformQTDlg : public CDialogEx
{
// ����
public:
	CCrossPlatformQTDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_CROSSPLATFORMQT_DIALOG };

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
	bool __SaveBaseConfigCtrl();
	void __InitBaseConfigCtrl();

public:
	void Pong();
	void OnLoginSuccess();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedButtonStart();
	CComboBox m_combCoinType;
	CComboBox m_combMoneyType;
	CEdit m_editFuturesCycle;
	CComboBox m_combLeverage;
	CComboBox m_combFuturesType;


public:
	clib::config m_config;
	std::string m_apiKey;
	std::string m_secretKey;
	std::string m_passphrase;
	time_t m_tListenPong;
	bool m_bRun;
	bool m_bSwapFutures;
	std::string m_strCoinType;			//��������
	std::string m_strMoneyType;			//��������
	std::string m_strFuturesCycle;		//��Լ����
	std::string m_strLeverage;			//��Լ����
	int m_nLeverage;					//��Լ����
	int m_nPriceDecimal;
};
