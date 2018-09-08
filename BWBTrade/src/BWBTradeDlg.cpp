
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
	switch(apiType)
	{
	case eHttpAPIType_Balance:
		{
			g_pBWBTradeDlg->UpdateBalance();
			g_pBWBTradeDlg->SetTimer(eTimerType_Balance, 5000, NULL);
		}
		break;
	case eHttpAPIType_Trade:
		{
			if(retObj.isObject() && retObj["resMsg"].isObject() && retObj["resMsg"]["code"].isString() && retObj["resMsg"]["code"].asString() == "1")
			{
				if(customData == 0)
				{
					g_pBWBTradeDlg->AddLog("卖单挂单成功~");
					std::string orderID = retObj["datas"]["entrustId"].asString();
					g_pBWBTradeDlg->m_mapSellOrders[orderID] = time(NULL);
				}
				else
				{
					g_pBWBTradeDlg->AddLog("买单挂单成功~");
				}
			}
		}
		break;
	case eHttpAPIType_CancelTrade:
		{
		}
		break;
	}
}

void local_websocket_callbak_open(const char* szExchangeName)
{
	if(g_pExchange->GetWebSocket())
		g_pExchange->GetWebSocket()->API_EntrustDepth(eMarketType_BWB_BTC, 5, true);
	if(g_pExchange->GetWebSocket())
		g_pExchange->GetWebSocket()->API_EntrustDepth(eMarketType_BWB_USDT, 5, true);
	if(g_pExchange->GetWebSocket())
		g_pExchange->GetWebSocket()->API_LatestExecutedOrder(eMarketType_BWB_BTC);
	if(g_pExchange->GetWebSocket())
		g_pExchange->GetWebSocket()->API_LatestExecutedOrder(eMarketType_BWB_USDT);
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
	case eWebsocketAPIType_LatestExecutedOrder:
		g_pBWBTradeDlg->UpdateEntrustDepth();
		g_pBWBTradeDlg->CheckPrice();
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
, m_bwbUsdtBuyPrice(0)
, m_bwbUsdtSellPrice(0)
, m_bwbUsdtWatchSellPrice(0)
, m_bwbBtcWatchBuyPrice(0)
, m_bwbBtcWatchSellPrice(0)
, m_bwbBtcBuyPrice(0)
, m_bwbBtcSellPrice(0)
, m_bwbUsdtWatchBuyPrice(0)
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
	DDX_Text(pDX, IDC_EDIT1, m_bwbUsdtBuyPrice);
	DDX_Text(pDX, IDC_EDIT2, m_bwbUsdtSellPrice);
	DDX_Text(pDX, IDC_EDIT3, m_bwbUsdtWatchBuyPrice);
	DDX_Text(pDX, IDC_EDIT4, m_bwbUsdtWatchSellPrice);
	DDX_Text(pDX, IDC_EDIT7, m_bwbBtcWatchBuyPrice);
	DDX_Text(pDX, IDC_EDIT8, m_bwbBtcWatchSellPrice);
	DDX_Text(pDX, IDC_EDIT5, m_bwbBtcBuyPrice);
	DDX_Text(pDX, IDC_EDIT6, m_bwbBtcSellPrice);
	DDX_Control(pDX, IDC_LIST4, m_listCtrlBalance);
	DDX_Control(pDX, IDC_EDIT1, m_editBwbUdstBuyPrice);
	DDX_Control(pDX, IDC_EDIT2, m_editBwbUdstSellPrice);
	DDX_Control(pDX, IDC_EDIT3, m_editBwbUdstWatchBuyPrice);
	DDX_Control(pDX, IDC_EDIT4, m_editBwbUdstWatchSellPrice);
	DDX_Control(pDX, IDC_EDIT5, m_editBwbBtcBuyPrice);
	DDX_Control(pDX, IDC_EDIT6, m_editBwbBtcSellPrice);
	DDX_Control(pDX, IDC_EDIT7, m_editBwbBtcWatchBuyPrice);
	DDX_Control(pDX, IDC_EDIT8, m_editBwbBtcWatchSellPrice);
}

BEGIN_MESSAGE_MAP(CBWBTradeDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDOK, &CBWBTradeDlg::OnBnClickedBegin)
	ON_WM_ERASEBKGND()
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
	const char* bwb_usdt_buy_price = m_config.get("bw", "bwb_usdt_buy", "");
	m_editBwbUdstBuyPrice.SetWindowTextA(bwb_usdt_buy_price);
	const char* bwb_usdt_sell_price = m_config.get("bw", "bwb_usdt_sell", "");
	m_editBwbUdstSellPrice.SetWindowTextA(bwb_usdt_sell_price);
	const char* bwb_usdt_watch_buy_price = m_config.get("bw", "bwb_usdt_watch_buy", "");
	m_editBwbUdstWatchBuyPrice.SetWindowTextA(bwb_usdt_watch_buy_price);
	const char* bwb_usdt_watch_sell_price = m_config.get("bw", "bwb_usdt_watch_sell", "");
	m_editBwbUdstWatchSellPrice.SetWindowTextA(bwb_usdt_watch_sell_price);

	const char* bwb_btc_buy_price = m_config.get("bw", "bwb_btc_buy", "");
	m_editBwbBtcBuyPrice.SetWindowTextA(bwb_btc_buy_price);
	const char* bwb_btc_sell_price = m_config.get("bw", "bwb_btc_sell", "");
	m_editBwbBtcSellPrice.SetWindowTextA(bwb_btc_sell_price);
	const char* bwb_btc_watch_buy_price = m_config.get("bw", "bwb_btc_watch_buy", "");
	m_editBwbBtcWatchBuyPrice.SetWindowTextA(bwb_btc_watch_buy_price);
	const char* bwb_btc_watch_sell_price = m_config.get("bw", "bwb_btc_watch_sell", "");
	m_editBwbBtcWatchSellPrice.SetWindowTextA(bwb_btc_watch_sell_price);

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

	m_listCtrlBalance.InsertColumn(0, "币种", LVCFMT_CENTER, 40);
	m_listCtrlBalance.InsertColumn(1, "余额", LVCFMT_CENTER, 100);
	m_listCtrlBalance.InsertColumn(2, "可用", LVCFMT_CENTER, 100);
	m_listCtrlBalance.InsertColumn(3, "冻结", LVCFMT_CENTER, 100);
	m_listCtrlBalance.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
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
			std::map<std::string, time_t>::iterator itB = m_mapSellOrders.begin();
			std::map<std::string, time_t>::iterator itE = m_mapSellOrders.end();
			time_t tNow = time(NULL);
			while(itB != itE)
			{
				if(tNow - itB->second > 60*10)
				{
					g_pExchange->GetHttp()->API_CancelTrade(eMarketType_BWB_USDT, itB->first, "");
					itB = m_mapSellOrders.erase(itB);
				}
				else
					++itB;
			}
		}
		break;
	case eTimerType_Balance:
		{
			if(g_pExchange && g_pExchange->GetHttp())
			{
				KillTimer(eTimerType_Balance);
				g_pExchange->GetHttp()->API_Balance();
			}
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
		std::map<std::string, std::string>& mapBuyEntrustDepth = pDataCenter->m_mapEntrustDepth["302"].mapBuyEntrustDepth;
		std::map<std::string, std::string>& mapSellEntrustDepth = pDataCenter->m_mapEntrustDepth["302"].mapSellEntrustDepth;
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
		double lastPrice = pDataCenter->m_mapLatestExecutedOrderPrice["BWB_USDT"];
		m_listCtrlBwbUsdt.InsertItem(sellLine, "");
		m_listCtrlBwbUsdt.SetItemText(sellLine, 0, "---");
		m_listCtrlBwbUsdt.SetItemText(sellLine, 1, CFuncCommon::Double2String(lastPrice, 4).c_str());
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
		double lastPrice = pDataCenter->m_mapLatestExecutedOrderPrice["BWB_BTC"];
		m_listCtrlBwbBtc.InsertItem(sellLine, "");
		m_listCtrlBwbBtc.SetItemText(sellLine, 0, "---");
		m_listCtrlBwbBtc.SetItemText(sellLine, 1, CFuncCommon::Double2String(lastPrice, 4).c_str());
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
	UpdateData(TRUE);
	if(CFuncCommon::CheckEqual(m_bwbUsdtBuyPrice, 0))
	{
		AfxMessageBox("请输入BWB/USDT购买价格");
		return;
	}
	if(CFuncCommon::CheckEqual(m_bwbUsdtSellPrice, 0))
	{
		AfxMessageBox("请输入BWB/USDT出售价格");
		return;
	}
	if(CFuncCommon::CheckEqual(m_bwbBtcBuyPrice, 0))
	{
		AfxMessageBox("请输入BWB/BTC购买价格");
		return;
	}
	if(CFuncCommon::CheckEqual(m_bwbBtcSellPrice, 0))
	{
		AfxMessageBox("请输入BWB/BTC出售价格");
		return;
	}
	CString strTemp;
	m_editBwbUdstBuyPrice.GetWindowTextA(strTemp);
	m_config.set_value("bw", "bwb_usdt_buy", strTemp.GetBuffer());
	m_editBwbUdstSellPrice.GetWindowTextA(strTemp);
	m_config.set_value("bw", "bwb_usdt_sell", strTemp.GetBuffer());
	m_editBwbUdstWatchBuyPrice.GetWindowTextA(strTemp);
	m_config.set_value("bw", "bwb_usdt_watch_buy", strTemp.GetBuffer());
	m_editBwbUdstWatchSellPrice.GetWindowTextA(strTemp);
	m_config.set_value("bw", "bwb_usdt_watch_sell", strTemp.GetBuffer());
	m_editBwbBtcBuyPrice.GetWindowTextA(strTemp);
	m_config.set_value("bw", "bwb_btc_buy", strTemp.GetBuffer());
	m_editBwbBtcSellPrice.GetWindowTextA(strTemp);
	m_config.set_value("bw", "bwb_btc_sell", strTemp.GetBuffer());
	m_editBwbBtcWatchBuyPrice.GetWindowTextA(strTemp);
	m_config.set_value("bw", "bwb_btc_watch_buy", strTemp.GetBuffer());
	m_editBwbBtcWatchSellPrice.GetWindowTextA(strTemp);
	m_config.set_value("bw", "bwb_btc_watch_sell", strTemp.GetBuffer());
	m_config.save("./config.ini");
	m_bIsRun = true;
}

void CBWBTradeDlg::UpdateBalance()
{
	m_listCtrlBalance.DeleteAllItems();
	CDataCenter* pDataCenter = g_pExchange->GetDataCenter();
	std::map<std::string, SBalanceInfo>::iterator itB = pDataCenter->m_mapBalanceInfo.begin();
	std::map<std::string, SBalanceInfo>::iterator itE = pDataCenter->m_mapBalanceInfo.end();
	int index = 0;
	CString szFormat;
	while(itB != itE)
	{
		m_listCtrlBalance.InsertItem(index, "");
		szFormat.Format("%s", itB->first.c_str());
		m_listCtrlBalance.SetItemText(index, 0, szFormat);
		szFormat.Format("%lf", itB->second.total);
		m_listCtrlBalance.SetItemText(index, 1, szFormat);
		szFormat.Format("%lf", itB->second.balance);
		m_listCtrlBalance.SetItemText(index, 2, szFormat);
		szFormat.Format("%lf", itB->second.freeze);
		m_listCtrlBalance.SetItemText(index, 3, szFormat);
		index++;
		++itB;
	}

}

void CBWBTradeDlg::CheckPrice()
{
	if(m_bIsRun)
	{
		CDataCenter* pDataCenter = g_pExchange->GetDataCenter();
		//BWB/USDT
		{
			if(!CFuncCommon::CheckEqual(m_bwbUsdtWatchSellPrice, 0))
			{
				double price = pDataCenter->m_mapLatestExecutedOrderPrice["BWB_USDT"];
				if(!CFuncCommon::CheckEqual(price, 0))
				{
					if(price > m_bwbUsdtWatchSellPrice && pDataCenter->m_mapBalanceInfo["bwb"].balance >= 1)
					{
						g_pExchange->GetTradeHttp()->API_Trade(eMarketType_BWB_USDT, CFuncCommon::ToString(int(pDataCenter->m_mapBalanceInfo["bwb"].balance)), CFuncCommon::Double2String(m_bwbUsdtSellPrice, 4), false, 0, "");
						pDataCenter->m_mapBalanceInfo["bwb"].balance = 0;
					}
				}
			}
			else
			{
				double price = pDataCenter->m_mapLatestExecutedOrderPrice["BWB_USDT"];
				if(!CFuncCommon::CheckEqual(price, 0))
				{
					if(price > m_bwbUsdtSellPrice && pDataCenter->m_mapBalanceInfo["bwb"].balance >= 1)
					{
						price = price*0.9;
						g_pExchange->GetTradeHttp()->API_Trade(eMarketType_BWB_USDT, CFuncCommon::ToString(int(pDataCenter->m_mapBalanceInfo["bwb"].balance)), CFuncCommon::Double2String(price, 4), false, 0, "");
						pDataCenter->m_mapBalanceInfo["bwb"].balance = 0;
					}
				}
			}
			if(!CFuncCommon::CheckEqual(m_bwbUsdtWatchBuyPrice, 0))
			{
				double price = pDataCenter->m_mapLatestExecutedOrderPrice["BWB_USDT"];
				if(!CFuncCommon::CheckEqual(price, 0))
				{
					if(price < m_bwbUsdtWatchBuyPrice && pDataCenter->m_mapBalanceInfo["usdt"].balance >= 1)
					{
						int cnt = int(pDataCenter->m_mapBalanceInfo["usdt"].balance / m_bwbUsdtBuyPrice) - 2;
						pDataCenter->m_mapBalanceInfo["usdt"].balance = 0;
						g_pExchange->GetTradeHttp()->API_Trade(eMarketType_BWB_USDT, CFuncCommon::ToString(cnt), CFuncCommon::Double2String(m_bwbUsdtBuyPrice, 4), true, 1, "");
					}
				}
			}
			else
			{
				double price = pDataCenter->m_mapLatestExecutedOrderPrice["BWB_USDT"];
				if(!CFuncCommon::CheckEqual(price, 0))
				{
					if(price < m_bwbUsdtBuyPrice && pDataCenter->m_mapBalanceInfo["usdt"].balance >= 1)
					{
						int cnt = int(pDataCenter->m_mapBalanceInfo["usdt"].balance / m_bwbUsdtBuyPrice) - 2;
						pDataCenter->m_mapBalanceInfo["usdt"].balance = 0;
						g_pExchange->GetTradeHttp()->API_Trade(eMarketType_BWB_USDT, CFuncCommon::ToString(cnt), CFuncCommon::Double2String(m_bwbUsdtBuyPrice, 4), true, 1, "");
					}
				}
			}
		}
		//BWB/BTC
		{
			if(!CFuncCommon::CheckEqual(m_bwbBtcWatchSellPrice, 0))
			{
				double price = pDataCenter->m_mapLatestExecutedOrderPrice["BWB_BTC"];
				if(!CFuncCommon::CheckEqual(price, 0))
				{
					if(price > m_bwbBtcWatchSellPrice && pDataCenter->m_mapBalanceInfo["bwb"].balance >= 1)
					{
						g_pExchange->GetTradeHttp()->API_Trade(eMarketType_BWB_BTC, CFuncCommon::ToString(int(pDataCenter->m_mapBalanceInfo["bwb"].balance)), CFuncCommon::Double2String(m_bwbBtcSellPrice, 4), false, 0, "");
						pDataCenter->m_mapBalanceInfo["bwb"].balance = 0;
					}
				}
			}
			else
			{
				double price = pDataCenter->m_mapLatestExecutedOrderPrice["BWB_BTC"];
				if(!CFuncCommon::CheckEqual(price, 0))
				{
					if(price > m_bwbBtcSellPrice && pDataCenter->m_mapBalanceInfo["bwb"].balance >= 1)
					{
						g_pExchange->GetTradeHttp()->API_Trade(eMarketType_BWB_BTC, CFuncCommon::ToString(int(pDataCenter->m_mapBalanceInfo["bwb"].balance)), CFuncCommon::Double2String(m_bwbBtcSellPrice, 4), false, 0, "");
						pDataCenter->m_mapBalanceInfo["bwb"].balance = 0;
					}
				}
			}
			if(!CFuncCommon::CheckEqual(m_bwbBtcWatchBuyPrice, 0))
			{
				double price = pDataCenter->m_mapLatestExecutedOrderPrice["BWB_BTC"];
				if(!CFuncCommon::CheckEqual(price, 0))
				{
					if(price < m_bwbBtcWatchBuyPrice && pDataCenter->m_mapBalanceInfo["btc"].balance >= 1)
					{
						int cnt = int(pDataCenter->m_mapBalanceInfo["btc"].balance / m_bwbBtcBuyPrice) - 2;
						pDataCenter->m_mapBalanceInfo["btc"].balance = 0;
						g_pExchange->GetTradeHttp()->API_Trade(eMarketType_BWB_BTC, CFuncCommon::ToString(cnt), CFuncCommon::Double2String(m_bwbBtcBuyPrice, 4), true, 1, "");
					}
				}
			}
			else
			{
				double price = pDataCenter->m_mapLatestExecutedOrderPrice["BWB_BTC"];
				if(!CFuncCommon::CheckEqual(price, 0))
				{
					if(price < m_bwbBtcBuyPrice && pDataCenter->m_mapBalanceInfo["btc"].balance >= 1)
					{
						int cnt = int(pDataCenter->m_mapBalanceInfo["btc"].balance / m_bwbBtcBuyPrice) - 2;
						pDataCenter->m_mapBalanceInfo["btc"].balance = 0;
						g_pExchange->GetTradeHttp()->API_Trade(eMarketType_BWB_BTC, CFuncCommon::ToString(cnt), CFuncCommon::Double2String(m_bwbBtcBuyPrice, 4), true, 1, "");
					}
				}
			}
		}
	}
}