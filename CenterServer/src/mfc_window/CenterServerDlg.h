
// CenterServerDlg.h : 头文件
//

#pragma once
#include <clib/lib/util/config.h>
#include "net\net_manager.h"
#include "net/client_factory.h"
#include "net/net_handle.h"
#include "logic/account_manager.h"
// CCenterServerDlg 对话框
class CCenterServerDlg : public CDialogEx
{
// 构造
public:
	CCenterServerDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_CENTERSERVER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

	void CreateServer();
// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
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
