
// CenterServerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "CenterServer.h"
#include "CenterServerDlg.h"
#include "afxdialogex.h"
#include "net/server.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CCenterServerDlg* g_pDlg = NULL;
// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CCenterServerDlg 对话框



CCenterServerDlg::CCenterServerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CCenterServerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pNet = NULL;
	m_pClientFactory = NULL;
	g_pDlg = this;
}

void CCenterServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CCenterServerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON1, &CCenterServerDlg::OnBnClickedButtonUpdateCfg)
END_MESSAGE_MAP()


// CCenterServerDlg 消息处理程序

BOOL CCenterServerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标
	m_accountManager.Init();
	if(!m_config.open("./config.ini"))
		return FALSE;
	m_ip = m_config.get("server", "ip", "127.0.0.1");
	m_port = m_config.get("server", "port", "9191");
	// TODO:  在此添加额外的初始化代码
	if(m_pNet == NULL)
	{
		m_pNet = new clib::net_manager();
		m_pNet->init(1, 1);
		if(m_pNet->start_run())
		{
			m_netHandle.init();
			CreateServer();
		}
	}
	SetTimer(1, 1, NULL);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CCenterServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CCenterServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CCenterServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CCenterServerDlg::CreateServer()
{
	if(m_pNet != NULL)
	{
		if(m_pClientFactory == NULL)
			m_pClientFactory = new client_factory();

		clib::pserver ps = m_pNet->create_server(m_ip.c_str(), m_port.c_str(), &m_netHandle, m_pClientFactory);
		if(ps != NULL)
		{
			ps->del_ref();
			return;
		}
	}

}

void CCenterServerDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	if(m_pNet)
		m_pNet->update(30);
	m_accountManager.Update();
	CDialogEx::OnTimer(nIDEvent);
}


void CCenterServerDlg::OnBnClickedButtonUpdateCfg()
{
	m_accountManager.Init();
}
