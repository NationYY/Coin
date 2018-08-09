
// BWBTradeDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "BWBTrade.h"
#include "BWBTradeDlg.h"
#include "afxdialogex.h"
#include "exchange/bw/bw_exchange.h"
#include <clib/lib/file/file_util.h>
#include <MMSystem.h>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
CBWExchange* g_pExchange = NULL;
CBWBTradeDlg* g_pBWBTradeDlg = NULL;
void local_http_callbak_message(eHttpAPIType apiType, Json::Value& retObj, const std::string& strRet, int customData, std::string strCustomData)
{
}

void local_websocket_callbak_open(const char* szExchangeName)
{
	if(g_pBWBTradeDlg->m_bIsRun)
	{
		if(g_pExchange->GetWebSocket())
			g_pExchange->GetWebSocket()->API_EntrustDepth(eMarketType_BWB_BTC, 5, true);
		if(g_pExchange->GetWebSocket())
			g_pExchange->GetWebSocket()->API_EntrustDepth(eMarketType_BWB_USDT, 5, true);
	}
	g_pBWBTradeDlg->AddLog("行情连接成功!");
}
void local_websocket_callbak_close(const char* szExchangeName)
{
	g_pBWBTradeDlg->AddLog("行情断开连接!");
}
void local_websocket_callbak_fail(const char* szExchangeName)
{
	g_pBWBTradeDlg->AddLog("行情连接失败!");
}

void local_websocket_callbak_message(eWebsocketAPIType apiType, const char* szExchangeName, Json::Value& retObj, const std::string& strRet)
{
	switch(apiType)
	{
	case eWebsocketAPIType_EntrustDepth:
		g_pBWBTradeDlg->UpdateEntrustDepth();
		break;
	}
	OutputDebugString(strRet.c_str());
	OutputDebugString("\n");
}
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


// CBWBTradeDlg 对话框



CBWBTradeDlg::CBWBTradeDlg(CWnd* pParent /*=NULL*/)
: CDialogEx(CBWBTradeDlg::IDD, pParent), m_bIsRun(false)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	g_pBWBTradeDlg = this;
}

void CBWBTradeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_listLog);
	DDX_Control(pDX, IDC_LIST3, m_listCtrlBwbUsdt);
	DDX_Control(pDX, IDC_LIST2, m_listCtrlBwbBtc);
}

BEGIN_MESSAGE_MAP(CBWBTradeDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDOK, &CBWBTradeDlg::OnBnClickedBegin)
END_MESSAGE_MAP()


// CBWBTradeDlg 消息处理程序

BOOL CBWBTradeDlg::OnInitDialog()
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
	if(!m_config.open("./config.ini"))
		return FALSE;
	const char* id = m_config.get("bw", "id", "");
	const char* key = m_config.get("bw", "key", "");
	// TODO:  在此添加额外的初始化代码
	g_pExchange = new CBWExchange(id, key);
	g_pExchange->SetHttpCallBackMessage(local_http_callbak_message);
	g_pExchange->SetWebSocketCallBackOpen(local_websocket_callbak_open);
	g_pExchange->SetWebSocketCallBackClose(local_websocket_callbak_close);
	g_pExchange->SetWebSocketCallBackFail(local_websocket_callbak_fail);
	g_pExchange->SetWebSocketCallBackMessage(local_websocket_callbak_message);
	g_pExchange->Run();

	clib::string log_path = "log/";
	bool bRet = clib::file_util::mkfiledir(log_path.c_str(), true);

	LocalLogger& localLogger = LocalLogger::GetInstance();
	localLogger.SetBatchMode(true);
	localLogger.SetLogPath(log_path.c_str());
	localLogger.Start();

	m_listCtrlBwbUsdt.InsertColumn(0, "", LVCFMT_CENTER, 30);
	m_listCtrlBwbUsdt.InsertColumn(1, "价", LVCFMT_CENTER, 100);
	m_listCtrlBwbUsdt.InsertColumn(2, "量", LVCFMT_CENTER, 100);
	m_listCtrlBwbUsdt.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

	m_listCtrlBwbBtc.InsertColumn(0, "", LVCFMT_CENTER, 30);
	m_listCtrlBwbBtc.InsertColumn(1, "价", LVCFMT_CENTER, 100);
	m_listCtrlBwbBtc.InsertColumn(2, "量", LVCFMT_CENTER, 100);
	m_listCtrlBwbBtc.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

	SetTimer(eTimerType_APIUpdate, 1, NULL);
	SetTimer(eTimerType_Balance, 5000, NULL);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CBWBTradeDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CBWBTradeDlg::OnPaint()
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
HCURSOR CBWBTradeDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CBWBTradeDlg::AddLog(char* szLog, ...)
{
	time_t tNow = time(NULL);
	tm* pTM = localtime(&tNow);
	char context[1100] = {0};
	_snprintf(context, 1100, "%d:%d:%d ", pTM->tm_hour, pTM->tm_min, pTM->tm_sec);
	char _context[1024] = {0};
	va_list args;
	int n;
	va_start(args, szLog);
	n = vsnprintf(_context, sizeof(_context), szLog, args);
	va_end(args);
	strcat(context, _context);
	m_listLog.InsertString(0, context);
}

void CBWBTradeDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	switch(nIDEvent)
	{
	case eTimerType_APIUpdate:
		{
			LocalLogger::GetInstancePt()->SwapFront2Middle();
			if(g_pExchange)
				g_pExchange->Update();
		}
		break;
	case eTimerType_Balance:
		{
			if(g_pExchange && g_pExchange->GetHttp())
				g_pExchange->GetHttp()->API_Balance();
		}
		break;
	}
	CDialogEx::OnTimer(nIDEvent);
}


void CBWBTradeDlg::UpdateEntrustDepth()
{
	const int showLines = 5;
	m_listCtrlBwbUsdt.DeleteAllItems();
	m_listCtrlBwbBtc.DeleteAllItems();
	CDataCenter* pDataCenter = g_pExchange->GetDataCenter();
	{
		std::map<std::string, std::string>& mapBuyEntrustDepth = pDataCenter->m_mapEntrustDepth["BWB_USDT"].mapBuyEntrustDepth;
		std::map<std::string, std::string>& mapSellEntrustDepth = pDataCenter->m_mapEntrustDepth["BWB_USDT"].mapSellEntrustDepth;
		int sellLine = min(mapSellEntrustDepth.size(), showLines);
		for(int i = 0; i < sellLine; ++i)
		{
			m_listCtrlBwbUsdt.InsertItem(i, "");
		}
		std::map<std::string, std::string>::iterator itB = mapSellEntrustDepth.begin();
		std::map<std::string, std::string>::iterator itE = mapSellEntrustDepth.end();
		CString szFormat;
		int count = 0;
		while(itB != itE)
		{
			szFormat.Format("%d", count + 1);
			m_listCtrlBwbUsdt.SetItemText(sellLine - 1 - count, 0, szFormat);
			szFormat.Format("%s", itB->first.c_str());
			m_listCtrlBwbUsdt.SetItemText(sellLine - 1 - count, 1, szFormat);
			szFormat.Format("%s", itB->second.c_str());
			m_listCtrlBwbUsdt.SetItemText(sellLine - 1 - count, 2, szFormat);
			if(++count >= sellLine)
				break;
			++itB;
		}
		m_listCtrlBwbUsdt.InsertItem(sellLine, "");
		m_listCtrlBwbUsdt.SetItemText(sellLine, 0, "---");
		m_listCtrlBwbUsdt.SetItemText(sellLine, 1, "-------------");
		m_listCtrlBwbUsdt.SetItemText(sellLine, 2, "-------------");
		int buyLine = min(mapBuyEntrustDepth.size(), showLines);
		for(int i = 0; i < sellLine; ++i)
		{
			m_listCtrlBwbUsdt.InsertItem(sellLine+1+i, "");
		}
		if(buyLine > 0)
		{
			std::map<std::string, std::string>::iterator itB = mapBuyEntrustDepth.begin();
			std::map<std::string, std::string>::iterator itE = mapBuyEntrustDepth.end();
			CString _szFormat;
			int count = 0;
			itE--;
			while(itB != itE)
			{
				_szFormat.Format("%d", count + 1);
				m_listCtrlBwbUsdt.SetItemText(sellLine+1+count, 0, _szFormat);
				_szFormat.Format("%s", itE->first.c_str());
				m_listCtrlBwbUsdt.SetItemText(sellLine+1+count, 1, _szFormat);
				_szFormat.Format("%s", itE->second.c_str());
				m_listCtrlBwbUsdt.SetItemText(sellLine+1+count, 2, _szFormat);
				if(++count >= buyLine)
					break;
				itE--;
			}
			if(count < buyLine)
			{
				_szFormat.Format("%d", count + 1);
				m_listCtrlBwbUsdt.SetItemText(sellLine + 1 + count, 0, _szFormat);
				_szFormat.Format("%s", itB->first.c_str());
				m_listCtrlBwbUsdt.SetItemText(sellLine + 1 + count, 1, _szFormat);
				_szFormat.Format("%s", itB->second.c_str());
				m_listCtrlBwbUsdt.SetItemText(sellLine + 1 + count, 2, _szFormat);
			}
		}
	}
	{

		std::map<std::string, std::string>& mapBuyEntrustDepth = pDataCenter->m_mapEntrustDepth["BWB_BTC"].mapBuyEntrustDepth;
		std::map<std::string, std::string>& mapSellEntrustDepth = pDataCenter->m_mapEntrustDepth["BWB_BTC"].mapSellEntrustDepth;
		int sellLine = min(mapSellEntrustDepth.size(), showLines);
		for(int i = 0; i < sellLine; ++i)
		{
			m_listCtrlBwbBtc.InsertItem(i, "");
		}
		std::map<std::string, std::string>::iterator itB = mapSellEntrustDepth.begin();
		std::map<std::string, std::string>::iterator itE = mapSellEntrustDepth.end();
		CString szFormat;
		int count = 0;
		while(itB != itE)
		{
			szFormat.Format("%d", count + 1);
			m_listCtrlBwbBtc.SetItemText(sellLine - 1 - count, 0, szFormat);
			szFormat.Format("%s", itB->first.c_str());
			m_listCtrlBwbBtc.SetItemText(sellLine - 1 - count, 1, szFormat);
			szFormat.Format("%s", itB->second.c_str());
			m_listCtrlBwbBtc.SetItemText(sellLine - 1 - count, 2, szFormat);
			if(++count >= sellLine)
				break;
			++itB;
		}
		m_listCtrlBwbBtc.InsertItem(sellLine, "");
		m_listCtrlBwbBtc.SetItemText(sellLine, 0, "---");
		m_listCtrlBwbBtc.SetItemText(sellLine, 1, "-------------");
		m_listCtrlBwbBtc.SetItemText(sellLine, 2, "-------------");
		int buyLine = min(mapBuyEntrustDepth.size(), showLines);
		for(int i = 0; i < sellLine; ++i)
		{
			m_listCtrlBwbBtc.InsertItem(sellLine+1+i, "");
		}
		if(buyLine > 0)
		{
			std::map<std::string, std::string>::iterator itB = mapBuyEntrustDepth.begin();
			std::map<std::string, std::string>::iterator itE = mapBuyEntrustDepth.end();
			CString _szFormat;
			int count = 0;
			itE--;
			while(itB != itE)
			{
				_szFormat.Format("%d", count + 1);
				m_listCtrlBwbBtc.SetItemText(sellLine+1+count, 0, _szFormat);
				_szFormat.Format("%s", itE->first.c_str());
				m_listCtrlBwbBtc.SetItemText(sellLine+1+count, 1, _szFormat);
				_szFormat.Format("%s", itE->second.c_str());
				m_listCtrlBwbBtc.SetItemText(sellLine+1+count, 2, _szFormat);
				if(++count >= buyLine)
					break;
				itE--;
			}
			if(count < buyLine)
			{
				_szFormat.Format("%d", count + 1);
				m_listCtrlBwbBtc.SetItemText(sellLine + 1 + count, 0, _szFormat);
				_szFormat.Format("%s", itB->first.c_str());
				m_listCtrlBwbBtc.SetItemText(sellLine + 1 + count, 1, _szFormat);
				_szFormat.Format("%s", itB->second.c_str());
				m_listCtrlBwbBtc.SetItemText(sellLine + 1 + count, 2, _szFormat);
			}
		}
	}
}

void CBWBTradeDlg::OnBnClickedBegin()
{
	m_bIsRun = true;
	if(g_pExchange->GetWebSocket())
		g_pExchange->GetWebSocket()->API_EntrustDepth(eMarketType_BWB_BTC, 5, true);
	if(g_pExchange->GetWebSocket())
		g_pExchange->GetWebSocket()->API_EntrustDepth(eMarketType_BWB_USDT, 5, true);
}
