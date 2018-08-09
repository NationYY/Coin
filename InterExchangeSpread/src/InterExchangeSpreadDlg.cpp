
// InterExchangeSpreadDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "InterExchangeSpread.h"
#include "InterExchangeSpreadDlg.h"
#include "afxdialogex.h"
#include "exchange/coinex/coinex_exchange.h"
#include "exchange/huobi_pro/huobi_pro_exchange.h"
#include "exchange/okex/okex_exchange.h"
#include <clib/lib/file/file_util.h>
#include "websocket_callback_func.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

std::list<CExchange*> g_listExchange;
CInterExchangeSpreadDlg* g_pDlg = NULL;
// 用于应用程序“关于”菜单项的 CAboutDlg 对话框
void local_websocket_callbak_message(eWebsocketAPIType apiType, const char* szExchangeName, Json::Value& retObj, const std::string& strRet)
{
	switch(apiType)
	{
	case eWebsocketAPIType_EntrustDepth:
		g_pDlg->UpdateShowEntrustDepth();
		break;
	default:
		break;
	}
}

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


// CInterExchangeSpreadDlg 对话框



CInterExchangeSpreadDlg::CInterExchangeSpreadDlg(CWnd* pParent /*=NULL*/)
: CDialogEx(CInterExchangeSpreadDlg::IDD, pParent), m_bIsRun(false)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	g_pDlg = this;
}

void CInterExchangeSpreadDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_listCtrlLog);
	DDX_Control(pDX, IDC_LIST2, m_listCtrlEntrustDepth);
	
}

BEGIN_MESSAGE_MAP(CInterExchangeSpreadDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDOK, &CInterExchangeSpreadDlg::OnBnClickedBegin)
	ON_WM_CLOSE()
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CInterExchangeSpreadDlg 消息处理程序

BOOL CInterExchangeSpreadDlg::OnInitDialog()
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
	const char* id = m_config.get("coinex", "id", "");
	const char* key = m_config.get("coinex", "key", "");
	CCoinexExchange* pCoinexExchange = new CCoinexExchange(id, key);
	g_listExchange.push_back(pCoinexExchange);
	pCoinexExchange->SetWebSocketCallBackOpen(common_websocket_callbak_open);
	pCoinexExchange->SetWebSocketCallBackClose(common_websocket_callbak_close);
	pCoinexExchange->SetWebSocketCallBackFail(common_websocket_callbak_fail);
	pCoinexExchange->SetWebSocketCallBackMessage(local_websocket_callbak_message);
	pCoinexExchange->Run(true, 5, 5);

	CHuobiProExchange* pHuobiProExchange = new CHuobiProExchange(id, key);
	g_listExchange.push_back(pHuobiProExchange);
	pHuobiProExchange->SetWebSocketCallBackOpen(common_websocket_callbak_open);
	pHuobiProExchange->SetWebSocketCallBackClose(common_websocket_callbak_close);
	pHuobiProExchange->SetWebSocketCallBackFail(common_websocket_callbak_fail);
	pHuobiProExchange->SetWebSocketCallBackMessage(local_websocket_callbak_message);
	pHuobiProExchange->Run(true, 5, 5);

	COkexExchange* pOkexExchange = new COkexExchange(id, key);
	g_listExchange.push_back(pOkexExchange);
	pOkexExchange->SetWebSocketCallBackOpen(common_websocket_callbak_open);
	pOkexExchange->SetWebSocketCallBackClose(common_websocket_callbak_close);
	pOkexExchange->SetWebSocketCallBackFail(common_websocket_callbak_fail);
	pOkexExchange->SetWebSocketCallBackMessage(local_websocket_callbak_message);
	pOkexExchange->Run(true, 5, 5);
	
	// TODO:  在此添加额外的初始化代码
	SetTimer(eTimerType_APIUpdate, 1, NULL);
	SetTimer(eTimerType_Ping, 5000, NULL);
	clib::string log_path = "log/";
	bool bRet = clib::file_util::mkfiledir(log_path.c_str(), true);
	LocalLogger& localLogger = LocalLogger::GetInstance();
	localLogger.SetBatchMode(true);
	localLogger.SetLogPath(log_path.c_str());
	localLogger.Start();
	m_listCtrlEntrustDepth.InsertColumn(0, "保留", LVCFMT_CENTER, 0);
	m_listCtrlEntrustDepth.InsertColumn(1, "交易所", LVCFMT_CENTER, 70);
	m_listCtrlEntrustDepth.InsertColumn(2, "买一量", LVCFMT_CENTER, 104);
	m_listCtrlEntrustDepth.InsertColumn(3, "买一价", LVCFMT_CENTER, 104);
	m_listCtrlEntrustDepth.InsertColumn(4, "卖一价", LVCFMT_CENTER, 104);
	m_listCtrlEntrustDepth.InsertColumn(5, "卖一量", LVCFMT_CENTER, 104);
	m_listCtrlEntrustDepth.InsertColumn(6, "taker费率", LVCFMT_CENTER, 69);
	m_listCtrlEntrustDepth.DeleteColumn(0);
	m_listCtrlEntrustDepth.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CInterExchangeSpreadDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CInterExchangeSpreadDlg::OnPaint()
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
HCURSOR CInterExchangeSpreadDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CInterExchangeSpreadDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	switch(nIDEvent)
	{
	case eTimerType_APIUpdate:
		{
			LocalLogger::GetInstancePt()->SwapFront2Middle();
			std::list<CExchange*>::iterator itB = g_listExchange.begin();
			std::list<CExchange*>::iterator itE = g_listExchange.end();
			while(itB != itE)
			{
				(*itB)->Update();
				++itB;
			}
		}
		
		break;
	case eTimerType_Ping:
		{
			std::list<CExchange*>::iterator itB = g_listExchange.begin();
			std::list<CExchange*>::iterator itE = g_listExchange.end();
			while(itB != itE)
			{
				if((*itB)->GetWebSocket())
					(*itB)->GetWebSocket()->Ping();
				++itB;
			}
		}
		break;
	}
	CDialogEx::OnTimer(nIDEvent);
}


void CInterExchangeSpreadDlg::AddLog(char* szLog, ...)
{
	time_t tNow = time(NULL);
	tm* pTM = localtime(&tNow);
	char context[1100] = { 0 };
	_snprintf(context, 1100, "%02d:%02d:%02d ", pTM->tm_hour, pTM->tm_min, pTM->tm_sec);
	char _context[1024] = { 0 };
	va_list args;
	int n;
	va_start(args, szLog);
	n = vsnprintf(_context, sizeof(_context), szLog, args);
	va_end(args);
	strcat(context, _context);
	m_listCtrlLog.InsertString(0, context);
}

void CInterExchangeSpreadDlg::OnBnClickedBegin()
{
	std::list<CExchange*>::iterator itB = g_listExchange.begin();
	std::list<CExchange*>::iterator itE = g_listExchange.end();
	while(itB != itE)
	{
		(*itB)->GetWebSocket()->API_EntrustDepth(eMarketType_ETH_BTC, 5, true);
		++itB;
	}
	m_bIsRun = true;
}


void CInterExchangeSpreadDlg::UpdateShowEntrustDepth()
{
	m_listCtrlEntrustDepth.DeleteAllItems();
	std::list<CExchange*>::iterator itB = g_listExchange.begin();
	std::list<CExchange*>::iterator itE = g_listExchange.end();
	int index = 0;
	CString szFormat;
	while(itB != itE)
	{
		m_listCtrlEntrustDepth.InsertItem(index, "");
		szFormat.Format("%s", (*itB)->GetName());
		m_listCtrlEntrustDepth.SetItemText(index, 0, szFormat);
		std::map<std::string, std::string>& mapBuyEntrustDepth = (*itB)->GetDataCenter()->m_mapEntrustDepth[""].mapBuyEntrustDepth;
		std::map<std::string, std::string>& mapSellEntrustDepth = (*itB)->GetDataCenter()->m_mapEntrustDepth[""].mapSellEntrustDepth;
		if(mapBuyEntrustDepth.size())
		{
			std::map<std::string, std::string>::iterator it = mapBuyEntrustDepth.end();
			it--;
			szFormat.Format("%s", it->second.c_str());
			m_listCtrlEntrustDepth.SetItemText(index, 1, szFormat);
			szFormat.Format("%s", it->first.c_str());
			m_listCtrlEntrustDepth.SetItemText(index, 2, szFormat);
		}
		if(mapSellEntrustDepth.size())
		{
			std::map<std::string, std::string>::iterator it = mapSellEntrustDepth.begin();
			szFormat.Format("%s", it->first.c_str());
			m_listCtrlEntrustDepth.SetItemText(index, 3, szFormat);
			szFormat.Format("%s", it->second.c_str());
			m_listCtrlEntrustDepth.SetItemText(index, 4, szFormat);
		}
		
		szFormat.Format("%s%%", CFuncCommon::Double2String((*itB)->GetTakerRate()*100, 2).c_str());
		m_listCtrlEntrustDepth.SetItemText(index, 5, szFormat);
		++itB;
	}

}

void CInterExchangeSpreadDlg::OnWebsocketConnect(const char* szExchangeName)
{
	std::list<CExchange*>::iterator itB = g_listExchange.begin();
	std::list<CExchange*>::iterator itE = g_listExchange.end();
	while(itB != itE)
	{
		if(strcmp((*itB)->GetName(), szExchangeName) == 0)
			(*itB)->GetWebSocket()->API_EntrustDepth(eMarketType_ETH_BTC, 5, true);
		++itB;
	}
}

void CInterExchangeSpreadDlg::OnClose()
{
	std::list<CExchange*>::iterator itB = g_listExchange.begin();
	std::list<CExchange*>::iterator itE = g_listExchange.end();
	while(itB != itE)
	{
		delete (*itB);
		++itB;
	}
	g_listExchange.clear();
	LocalLogger::ReleaseInstance();
	CDialogEx::OnClose();
}
