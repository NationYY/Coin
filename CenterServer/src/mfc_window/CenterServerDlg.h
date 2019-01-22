
// CenterServerDlg.h : ͷ�ļ�
//

#pragma once
#include <clib/lib/util/config.h>
#include "net\net_manager.h"
#include "net/client_factory.h"
#include "net/net_handle.h"
#include "logic/account_manager.h"
// CCenterServerDlg �Ի���
class CCenterServerDlg : public CDialogEx
{
// ����
public:
	CCenterServerDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_CENTERSERVER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

	void CreateServer();
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
	std::string m_ip;
	std::string m_port;
	clib::config m_config;
	clib::pnet_manager m_pNet;
	pclient_factory m_pClientFactory;
	client_net_handle m_netHandle;
public:
	CAccountManager m_accountManager;
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedButtonUpdateCfg();
};
