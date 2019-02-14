
// okex_futures_dlg.cpp : 实现文件
//

#include "stdafx.h"
#include "okex_futures.h"
#include "okex_futures_dlg.h"
#include "afxdialogex.h"
#include <clib/lib/file/file_util.h>
#include "log/local_log.h"
#include "log/local_action_log.h"
#include "exchange/okex/okex_exchange.h"
#include "common/func_common.h"
#include "test_kline_data.h"
#include "api_callback.h"
#include "net/client.h"
#include "net/nmsg_server_iml.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#define DOUBLE_PRECISION 0.00000001
#define BOLL_DATA m_vecBollData
#define BOLL_DATA_SIZE ((int)m_vecBollData.size())
#define REAL_BOLL_DATA_SIZE ((int)m_vecBollData.size() - m_nBollCycle -1)
#define KLINE_DATA m_vecKlineData
#define KLINE_DATA_SIZE ((int)m_vecKlineData.size())
#define OKEX_CHANGE ((COkexExchange*)pExchange)
#define NEW_MODE 1
// 用于应用程序“关于”菜单项的 CAboutDlg 对话框
CExchange* pExchange = NULL;
COKExFuturesDlg* g_pDlg = NULL;

int LocalLogCallBackFunc(LOG_TYPE type, const char* szLog)
{
	if(g_pDlg->m_ctrlListLog.GetCount() > 1500)
		g_pDlg->m_ctrlListLog.ResetContent();
	g_pDlg->m_ctrlListLog.InsertString(0, szLog);
	g_pDlg->SetHScroll();
	return 0;
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


// COKExFuturesDlg 对话框


COKExFuturesDlg::COKExFuturesDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(COKExFuturesDlg::IDD, pParent)
{
	g_pDlg = this;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_nBollCycle = 20;
	m_nPriceDecimal = 3;
	m_nZhangKouCheckCycle = 20;
	m_nShouKouCheckCycle = 20;
	m_nZhangKouTrendCheckCycle = 3;
	m_bRun = false;
	m_eBollState = eBollTrend_Normal;
	m_eLastBollState = eBollTrend_Normal;
	m_nZhangKouDoubleConfirmCycle = 2;
	m_nShoukouDoubleConfirmCycle = 3;
	m_tListenPong = 0;
	m_tListenServerPong = 0;
	m_strKlineCycle = "candle180s";
	m_nKlineCycle = 180;
	m_strCoinType = "BTC";
	m_strFuturesCycle = "190329";
	m_strFuturesTradeSize = "10";
	m_strLeverage = "20";
	m_nLeverage  = 20;
	m_bTest = false;
	m_stopLoss = 0.04;
	m_moveStopProfit = 0.005;
	m_bStopWhenFinish = false;
	m_bFirstKLine = true;
	m_nMaxTradeCnt = 5;
	m_nMaxDirTradeCnt = 3;
	m_bSwapFutures = false;
	m_nShouKouTradeCheckBar = 0;
	m_nZhangKouTradeCheckBar = 0;
	m_tradeMoment = 0;
	m_bCanLogCheckCanTrade = true;
	m_bCanStopProfit = true;
	m_pNet = NULL;
	m_pServerFactory = NULL;
	m_bSuccessLogin = false;
	m_bollCheckAngle = 1.0005;
	m_todayBeginMoney = 0.0;
	m_bollCheckAngleFast = 1.0007;
	m_bollCheckAngleSlow = 1.0004;
	m_nLastUpdateDay = -1;
	m_nLastTradeBarByShouKouChannel = 0;
}

void COKExFuturesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO2, m_combCoinType);
	DDX_Control(pDX, IDC_COMBO3, m_combLeverage);
	DDX_Control(pDX, IDC_EDIT1, m_editFuturesCycle);
	DDX_Control(pDX, IDC_EDIT2, m_editFuturesTradeSize);
	DDX_Control(pDX, IDC_EDIT3, m_editStopLoss);
	DDX_Control(pDX, IDC_EDIT4, m_editMoveStopProfit);
	DDX_Control(pDX, IDC_LIST2, m_ctrlListLog);
	DDX_Control(pDX, IDC_LIST1, m_listCtrlOrderOpen);
	DDX_Control(pDX, IDC_LIST3, m_listCtrlOrderClose);
	DDX_Control(pDX, IDC_STATIC_CUR_PRICE, m_staticPrice);
	DDX_Control(pDX, IDC_EDIT5, m_editMaxTradeCnt);
	DDX_Control(pDX, IDC_EDIT6, m_editMaxDirTradeCnt);
	DDX_Control(pDX, IDC_COMBO4, m_combFuturesType);
	DDX_Control(pDX, IDC_STATIC_ACCOUNT, m_staticAccountInfo);
	DDX_Control(pDX, IDC_EDIT7, m_editCapital);
	DDX_Control(pDX, IDC_EDIT8, m_editCapitalToday);
	DDX_Control(pDX, IDC_STATIC_PROFIT, m_staticProfit);
	DDX_Control(pDX, IDC_STATIC_TODAY_PROFIT, m_staticTodayProfit);
	
	
	DDX_Control(pDX, IDC_COMBO1, m_combTradeMoment);
	DDX_Control(pDX, IDC_STATIC_ACCOUNT_STATE, m_staticAccountState);
}

BEGIN_MESSAGE_MAP(COKExFuturesDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON1, &COKExFuturesDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON2, &COKExFuturesDlg::OnBnClickedButtonTest)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON3, &COKExFuturesDlg::OnBnClickedButtonStopWhenFinish)
	ON_BN_CLICKED(IDC_BUTTON4, &COKExFuturesDlg::OnBnClickedButtonUpdateTradeSize)
	ON_BN_CLICKED(IDC_BUTTON5, &COKExFuturesDlg::OnBnClickedButtonUpdateCost)
	ON_BN_CLICKED(IDC_BUTTON7, &COKExFuturesDlg::OnBnClickedButtonUpdateTodayCost)
	ON_BN_CLICKED(IDC_BUTTON6, &COKExFuturesDlg::OnBnClickedButtonUpdateTradeMoment)
END_MESSAGE_MAP()


// COKExFuturesDlg 消息处理程序

BOOL COKExFuturesDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	// 将“关于...”菜单项添加到系统菜单中。
	CFuncCommon::GetAllFileInDirectory("./data", m_setAllTestFile);
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
	m_combCoinType.InsertString(0, "BTC");
	m_combCoinType.InsertString(1, "LTC");
	m_combCoinType.InsertString(2, "ETH");
	m_combCoinType.InsertString(3, "ETC");
	m_combCoinType.InsertString(4, "BTG");
	m_combCoinType.InsertString(5, "XRP");
	m_combCoinType.InsertString(6, "EOS");
	m_combCoinType.InsertString(7, "BCH");
	m_combCoinType.InsertString(8, "TRX");

	m_combLeverage.InsertString(0, "10");
	m_combLeverage.InsertString(1, "20");

	m_combTradeMoment.InsertString(0, "趋势出现正向");
	m_combTradeMoment.InsertString(1, "趋势出现反向");

	m_combFuturesType.InsertString(0, "交割合约");
	m_combFuturesType.InsertString(1, "永续合约");
	m_listCtrlOrderOpen.InsertColumn(0, "价格", LVCFMT_CENTER, 90);
	m_listCtrlOrderOpen.InsertColumn(1, "类型", LVCFMT_CENTER, 45);
	m_listCtrlOrderOpen.InsertColumn(2, "成交量", LVCFMT_CENTER, 70);
	m_listCtrlOrderOpen.InsertColumn(3, "状态", LVCFMT_CENTER, 45);
	m_listCtrlOrderOpen.InsertColumn(4, "最低价", LVCFMT_CENTER, 70);
	m_listCtrlOrderOpen.InsertColumn(5, "最高价", LVCFMT_CENTER, 70);
	m_listCtrlOrderOpen.InsertColumn(6, "参考利润", LVCFMT_CENTER, 120);
	m_listCtrlOrderOpen.InsertColumn(7, "下单时间", LVCFMT_CENTER, 115);
	m_listCtrlOrderOpen.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

	m_listCtrlOrderClose.InsertColumn(0, "价格", LVCFMT_CENTER, 85);
	m_listCtrlOrderClose.InsertColumn(1, "成交量", LVCFMT_CENTER, 70);
	m_listCtrlOrderClose.InsertColumn(2, "状态", LVCFMT_CENTER, 45);
	m_listCtrlOrderClose.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	if(!m_config.open("./config.ini"))
		return FALSE;
	m_apiKey = m_config.get("futures", "apiKey", "");
	m_secretKey = m_config.get("futures", "secretKey", "");
	m_passphrase = m_config.get("futures", "passphrase", "");
	m_nLastUpdateDay = m_config.get_int("futures", "lastUpdateDay", -1);
	
	__InitConfigCtrl();

	pExchange = new COkexExchange(m_apiKey, m_secretKey, m_passphrase, true);
	pExchange->SetHttpCallBackMessage(local_http_callbak_message);
	pExchange->SetWebSocketCallBackOpen(local_websocket_callbak_open);
	pExchange->SetWebSocketCallBackClose(local_websocket_callbak_close);
	pExchange->SetWebSocketCallBackFail(local_websocket_callbak_fail);
	pExchange->SetWebSocketCallBackMessage(local_websocket_callbak_message);
	pExchange->Run();

	SetTimer(eTimerType_APIUpdate, 1, NULL);
	SetTimer(eTimerType_Ping, 15000, NULL);
	SetTimer(eTimerType_Account, 3000, NULL);
	SetTimer(eTimerType_TradeOrder, 5000, NULL);
	
	clib::string log_path = "log/";
	bool bRet = clib::file_util::mkfiledir(log_path.c_str(), true);

	CLocalLogger& _localLogger = CLocalLogger::GetInstance();
	_localLogger.SetBatchMode(true);
	_localLogger.SetLogPath(log_path.c_str());
	_localLogger.Start();
	_localLogger.SetCallBackFunc(LocalLogCallBackFunc);
	CLocalActionLog::GetInstancePt()->set_log_path(log_path.c_str());
	CLocalActionLog::GetInstancePt()->start();

	if(m_pNet == NULL)
	{
		m_pNet = new clib::net_manager();
		m_pNet->init(1, 1);
		if(m_pNet->start_run())
		{
			m_netHandle.init();
			ConnectServer();
		}
	}

	// TODO:  在此添加额外的初始化代码
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void COKExFuturesDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void COKExFuturesDlg::OnPaint()
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
HCURSOR COKExFuturesDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void COKExFuturesDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	switch(nIDEvent)
	{
	case eTimerType_APIUpdate:
		{
			CLocalLogger::GetInstancePt()->SwapFront2Middle();
			if(OKEX_CHANGE)
				OKEX_CHANGE->Update();
			if(m_pNet)
				m_pNet->update(30);
			time_t tNow = time(NULL);
			if(m_tListenPong && tNow - m_tListenPong > 15)
			{
				LOCAL_ERROR("ping超时重连");
				m_tListenPong = 0;
				delete pExchange;
				pExchange = new COkexExchange(m_apiKey, m_secretKey, m_passphrase, true);
				pExchange->SetHttpCallBackMessage(local_http_callbak_message);
				pExchange->SetWebSocketCallBackOpen(local_websocket_callbak_open);
				pExchange->SetWebSocketCallBackClose(local_websocket_callbak_close);
				pExchange->SetWebSocketCallBackFail(local_websocket_callbak_fail);
				pExchange->SetWebSocketCallBackMessage(local_websocket_callbak_message);
				pExchange->Run();
			}
			if(m_tListenServerPong && tNow - m_tListenServerPong > 15)
				SetLoginState(false);
		}
		break;
	case eTimerType_Ping:
		{
			if(OKEX_WEB_SOCKET->Ping())
				m_tListenPong = time(NULL);
			if(m_netHandle._session)
			{
				m_tListenServerPong = time(NULL);
				nmsg_server_iml::cg_ping(m_netHandle._session);
			}
		}
		break;
	case eTimerType_Account:
		{
			if(m_bRun)
				OKEX_HTTP->API_FuturesAccountInfoByCurrency(m_bSwapFutures, m_strCoinType);
		}
		break;
	case eTimerType_TradeOrder:
		{
			if(m_bRun)
			{
				time_t tNow = time(NULL);
				std::list<SFuturesTradePairInfo>::iterator itB = m_listTradePairInfo.begin();
				std::list<SFuturesTradePairInfo>::iterator itE = m_listTradePairInfo.end();
				while(itB != itE)
				{
					if((itB->open.status == "1" || itB->open.status == "0") && tNow - itB->open.tLastUpdate >= 10)
						OKEX_HTTP->API_FuturesOrderInfo(true, m_bSwapFutures, m_strCoinType, m_strFuturesCycle, itB->open.orderID);
					if((itB->close.status == "1" || itB->close.status == "0") && tNow - itB->close.tLastUpdate >= 10)
						OKEX_HTTP->API_FuturesOrderInfo(true, m_bSwapFutures, m_strCoinType, m_strFuturesCycle, itB->close.orderID);
					if((itB->stopLoss.status == "1" || itB->stopLoss.status == "0") && tNow - itB->stopLoss.tLastUpdate >= 10)
						OKEX_HTTP->API_FuturesOrderInfo(true, m_bSwapFutures, m_strCoinType, m_strFuturesCycle, itB->stopLoss.orderID);
					++itB;
				}

			}
		}
		break;
	case eTimerType_ConnetServer:
		{
			ConnectServer();
			KillTimer(eTimerType_ConnetServer);
		}
		break;
	}
	CDialogEx::OnTimer(nIDEvent);
}


void COKExFuturesDlg::OnBnClickedButtonStart()
{
	if(!__SaveConfigCtrl())
		return;
	if(m_bRun)
		return;
	if(!m_bSuccessLogin)
	{
		MessageBox("账号验证未通过");
		return;
	}
	bool bFound = false;
	std::string instrumentID;
	if(m_bSwapFutures)
		instrumentID = m_strCoinType + "-USD-SWAP";
	else
		instrumentID = m_strCoinType + "-USD-" + m_strFuturesCycle;
	for(int i = 0; i<3; ++i)
	{
		SHttpResponse resInfo;
		OKEX_HTTP->API_FuturesInstruments(false, m_bSwapFutures, &resInfo);
		bFound = false;
		if(resInfo.retObj.isArray())
		{
			for(int j = 0; j<(int)resInfo.retObj.size(); ++j)
			{
				if(resInfo.retObj[j]["instrument_id"].asString() == instrumentID)
				{
					std::string strTickSize = resInfo.retObj[j]["tick_size"].asString();
					int pos = strTickSize.find_first_of(".");
					if(pos != std::string::npos)
					{
						m_nPriceDecimal = strTickSize.size() - pos - 1;
						bFound = true;
					}
					else
					{
						m_nPriceDecimal = 0;
						bFound = true;
					}
					LOCAL_INFO("PriceDecimal=%d", m_nPriceDecimal);
					if(bFound)
						break;
				}
			}
		}
		if(bFound)
			break;
	}
	if(!bFound)
	{
		MessageBox("未得到交易对详细信息");
		return;
	}
	{
		SHttpResponse resInfo;
		std::string strLeverage = CFuncCommon::ToString(m_nLeverage);
		OKEX_HTTP->API_FuturesSetLeverage(false, m_bSwapFutures, m_strCoinType, strLeverage, &resInfo);
		if(m_bSwapFutures)
		{
			std::string strInstrumentID = m_strCoinType + "-USD-SWAP";
			if(!resInfo.retObj.isObject() || ((resInfo.retObj["instrument_id"].asString() != strInstrumentID) && (resInfo.retObj["code"].asInt() != 35017)))
			{
				MessageBox("设置杠杆失败");
				return;
			}
		}
		else
		{
			if(!resInfo.retObj.isObject() || (resInfo.retObj["result"].asString() != "true"))
			{
				MessageBox("设置杠杆失败");
				return;
			}
		}
	}
	if(OKEX_WEB_SOCKET)
	{
		OKEX_WEB_SOCKET->API_FuturesKlineData(true, m_bSwapFutures, m_strKlineCycle, m_strCoinType, m_strFuturesCycle);
		OKEX_WEB_SOCKET->API_FuturesTickerData(true, m_bSwapFutures, m_strCoinType, m_strFuturesCycle);
		OKEX_WEB_SOCKET->API_LoginFutures(m_apiKey, m_secretKey, time(NULL));
		//OKEX_HTTP->API_FuturesServerTime();
	}
	CString strTitle;
	strTitle.Format("%s-%s", (m_bSwapFutures?"永续合约":"交割合约"), m_strCoinType.c_str());
	CWnd *m_pMainWnd;
	m_pMainWnd = AfxGetMainWnd();
	m_pMainWnd->SetWindowText(strTitle);
	m_bRun = true;
	CActionLog("market", "restart");
	if(m_strCoinType == "BTC")
		m_bollCheckAngle = 1.0003;
}

void COKExFuturesDlg::AddKlineData(SKlineData& data)
{
	if(KLINE_DATA_SIZE)
	{
		if(data.time - KLINE_DATA[KLINE_DATA_SIZE-1].time != m_nKlineCycle)
		{
			CActionLog("boll", "差距%d秒", data.time - KLINE_DATA[KLINE_DATA_SIZE-1].time);
			ComplementedKLine(data.time, int(data.time - KLINE_DATA[KLINE_DATA_SIZE-1].time - m_nKlineCycle)/m_nKlineCycle);
			//KLINE_DATA.clear();
			//BOLL_DATA.clear();
		}
	}
	tm* pTM = localtime(&data.time);
	_snprintf(data.szTime, 20, "%d-%02d-%02d %02d:%02d:%02d", pTM->tm_year+1900, pTM->tm_mon+1, pTM->tm_mday, pTM->tm_hour, pTM->tm_min, pTM->tm_sec);
	if(strcmp(data.szTime, "2019-01-25 13:12:00") == 0)
		int a = 3;
	KLINE_DATA.push_back(data);
	if(KLINE_DATA_SIZE >= m_nBollCycle)
	{
		double totalClosePrice = 0.0;
		for(int i = KLINE_DATA_SIZE-1; i>=KLINE_DATA_SIZE-m_nBollCycle; --i)
		{
			totalClosePrice += KLINE_DATA[i].closePrice;
		}
		double ma = totalClosePrice/m_nBollCycle;
		double totalDifClosePriceSQ = 0.0;
		for(int i = KLINE_DATA_SIZE-1; i>=KLINE_DATA_SIZE-m_nBollCycle; --i)
		{
			totalDifClosePriceSQ += ((KLINE_DATA[i].closePrice - ma)*(KLINE_DATA[i].closePrice - ma));
		}
		double md = sqrt(totalDifClosePriceSQ/m_nBollCycle);
		SBollInfo info;
		info.mb = ma;
		info.up = info.mb + 2*md;
		info.dn = info.mb - 2*md;
		info.mb = CFuncCommon::Round(info.mb+DOUBLE_PRECISION, m_nPriceDecimal);
		info.up = CFuncCommon::Round(info.up+DOUBLE_PRECISION, m_nPriceDecimal);
		info.dn = CFuncCommon::Round(info.dn+DOUBLE_PRECISION, m_nPriceDecimal);
		info.time = data.time;
		tm* pTM = localtime(&info.time);
		_snprintf(info.szTime, 20, "%d-%02d-%02d %02d:%02d:%02d", pTM->tm_year+1900, pTM->tm_mon+1, pTM->tm_mday, pTM->tm_hour, pTM->tm_min, pTM->tm_sec);
		BOLL_DATA.push_back(info);
		OnBollUpdate();
	}
	else
	{
		SBollInfo info;
		BOLL_DATA.push_back(info);
	}
		
}

void COKExFuturesDlg::Test()
{
	m_bTest = true;
	//m_eBollState = eBollTrend_ShouKouChannel;
	//m_eLastBollState = eBollTrend_ShouKou;
	std::set<std::string>::iterator itB = m_setAllTestFile.begin();
	std::set<std::string>::iterator itE = m_setAllTestFile.end();
	char* szEnd = NULL;
	while(itB != itE)
	{
		std::string strFilePath = "./data/";
		strFilePath.append(*itB);
		std::ifstream stream(strFilePath);
		if(!stream.is_open())
			continue;
		char lineBuffer[4096] = {0};
		if(stream.fail())
			continue;
		while(!stream.eof())
		{
			stream.getline(lineBuffer, sizeof(lineBuffer));
			if(*lineBuffer == 0 || (lineBuffer[0] == '/' && lineBuffer[1] == '/') || (lineBuffer[0] == '-' && lineBuffer[1] == '-'))
				continue;
			std::stringstream lineStream(lineBuffer, std::ios_base::in);
			char szContent[4096] = {};
			lineStream >> szContent;
			lineStream >> szContent >> szContent;
			if(strcmp(szContent, "restart") == 0)
			{
				KLINE_DATA.clear();
				BOLL_DATA.clear();
				m_nShouKouConfirmBar = 0;
				m_nZhangKouConfirmBar = 0;
				m_nZhangKouMinValue = 0;
				m_nZhangKouTradeCheckBar = 0;
				m_nShouKouTradeCheckBar = 0;
				m_nShouKouChannelConfirmBar = 0;
				continue;
			}
			Json::Value retObj;
			Json::Reader reader;
			reader.parse(szContent, retObj);
			if(retObj.isObject() && retObj["table"].isString())
			{
				std::string strChannel = retObj["table"].asString();

				std::string strTickChannel;
				if(m_bSwapFutures)
					strTickChannel = "swap/ticker";
				else
					strTickChannel = "futures/ticker";

				std::string strKlineChannel;
				if(m_bSwapFutures)
					strKlineChannel = "swap/";
				else
					strKlineChannel = "futures/";
				strKlineChannel.append(m_strKlineCycle);
				if(strChannel == strKlineChannel)
				{
					SKlineData data;
					data.time = CFuncCommon::ISO8601ToTime(retObj["data"][0]["candle"][0].asString());
					data.openPrice = stod(retObj["data"][0]["candle"][1].asString());
					data.highPrice = stod(retObj["data"][0]["candle"][2].asString());
					data.lowPrice = stod(retObj["data"][0]["candle"][3].asString());
					data.closePrice = stod(retObj["data"][0]["candle"][4].asString());
					data.volume = stoi(retObj["data"][0]["candle"][5].asString());
					data.volumeByCurrency = stod(retObj["data"][0]["candle"][6].asString());
					g_pDlg->AddKlineData(data);
				}
				else if(strChannel == strTickChannel)
				{
					STickerData data;
					data.volume = stoi(retObj["data"][0]["volume_24h"].asString());
					data.sell = stod(retObj["data"][0]["best_ask"].asString());
					data.buy = stod(retObj["data"][0]["best_bid"].asString());
					data.high = stod(retObj["data"][0]["high_24h"].asString());
					data.low = stod(retObj["data"][0]["low_24h"].asString());
					data.last = stod(retObj["data"][0]["last"].asString());
					data.time = CFuncCommon::ISO8601ToTime(retObj["data"][0]["timestamp"].asString());
					g_pDlg->OnRevTickerInfo(data);
				}
			}
		}
		stream.close();
		++itB;
	}
}

void COKExFuturesDlg::OnBollUpdate()
{
	CheckBollTrend();
}

void COKExFuturesDlg::CheckBollTrend()
{
	switch(m_eBollState)
	{
	case eBollTrend_Normal:
		__CheckTrend_Normal();
		break;
	case eBollTrend_ShouKou:
		__CheckTrend_ShouKou();
		break;
	case eBollTrend_ShouKouChannel:
		__CheckTrend_ShouKouChannel();
		break;
	case eBollTrend_ZhangKou:
		__CheckTrend_ZhangKou();
		break;
	default:
		break;
	}
}

void COKExFuturesDlg::__CheckTrend_Normal()
{
	double minValue;
	if(_FindZhangKou(0, minValue))
	{
		__SetBollState(eBollTrend_ZhangKou, 0, minValue);
		return;
	}
	if(_FindShouKou())
	{
		__SetBollState(eBollTrend_ShouKou);
		return;
	}
}

void COKExFuturesDlg::__CheckTrend_ZhangKou()
{
	//寻找收口的同时用N个周期判断张口的成立
	if(BOLL_DATA_SIZE <= m_nZhangKouConfirmBar+m_nZhangKouDoubleConfirmCycle+1)
	{
		if(m_bZhangKouUp)
		{
			if(BOLL_DATA[BOLL_DATA_SIZE-1].up < BOLL_DATA[BOLL_DATA_SIZE-2].up)
			{
				CActionLog("boll", "张口不成立");
				__SetBollState(m_eLastBollState);
			}
			else
			{
				
			}
			if(BOLL_DATA[BOLL_DATA_SIZE-1].dn > BOLL_DATA[BOLL_DATA_SIZE-2].dn)
			{
				CActionLog("boll", "张口不成立");
				__SetBollState(m_eLastBollState);
			}
		}
		else
		{
			if(BOLL_DATA[BOLL_DATA_SIZE-1].dn > BOLL_DATA[BOLL_DATA_SIZE-2].dn)
			{
				CActionLog("boll", "张口不成立");
				__SetBollState(m_eLastBollState);
			}
			if(BOLL_DATA[BOLL_DATA_SIZE-1].up < BOLL_DATA[BOLL_DATA_SIZE-2].up)
			{
				CActionLog("boll", "张口不成立");
				__SetBollState(m_eLastBollState);
			}
		}
	}
	else
	{
		//寻找收口,从确定张口的柱子开始
		if(_FindShouKou())
		{
			__SetBollState(eBollTrend_ShouKou);
			return;
		}
	}
	//超过25个周期直接进入收口通道状态
	if(KLINE_DATA_SIZE-1-m_nZhangKouConfirmBar >= 25)
	{
		__SetBollState(eBollTrend_ShouKouChannel, 1);
		return;
	}
}

void COKExFuturesDlg::__CheckTrend_ShouKou()
{
	//用N个周期来确认收口完成
	if(BOLL_DATA_SIZE >= m_nShouKouConfirmBar+m_nShoukouDoubleConfirmCycle)
	{
		double last = 0.0;
		bool bRet = true;
		for(int i = 0; i<m_nShoukouDoubleConfirmCycle; ++i)
		{
			double offset = BOLL_DATA[BOLL_DATA_SIZE-1-i].up - BOLL_DATA[BOLL_DATA_SIZE-1-i].dn;

			double avgPrice = (KLINE_DATA[KLINE_DATA_SIZE-1-i].highPrice + KLINE_DATA[KLINE_DATA_SIZE-1-i].lowPrice) / 2;
			if(offset/avgPrice >= 0.02)
			{
				bRet = false;
				break;
			}
			if(last > 0)
			{
				if(offset >= last)
				{
					if(offset/last > 1.1)
					{
						bRet = false;
						break;
					}
				}
				else
				{
					if(last/offset > 1.1)
					{
						bRet = false;
						break;
					}
				}
			}
			else
				last = offset;
		}
		if(bRet)
		{
			__SetBollState(eBollTrend_ShouKouChannel, 0);
			return;
		}
	}
	double minValue;
	if(_FindZhangKou(m_nShouKouConfirmBar, minValue))
	{
		__SetBollState(eBollTrend_ZhangKou, 0, minValue);
		return;
	}
	//超过25个周期直接进入收口通道状态
	if(KLINE_DATA_SIZE-1-m_nShouKouConfirmBar >= 25)
	{
		__SetBollState(eBollTrend_ShouKouChannel, 1);
		return;
	}
}


void COKExFuturesDlg::__CheckTrend_ShouKouChannel()
{
	//寻找张口,从确定收口通道的柱子开始
	if(m_eLastBollState == eBollTrend_ShouKou || m_eLastBollState == eBollTrend_ZhangKou)
	{
		double minValue;
		int checkBar = 0;
		if(m_eLastBollState == eBollTrend_ShouKou)
			checkBar = m_nShouKouConfirmBar;
		if(_FindZhangKou(checkBar, minValue))
		{
			__SetBollState(eBollTrend_ZhangKou, 0, minValue);
			return;
		}
	}
}

void COKExFuturesDlg::__SetBollState(eBollTrend state, int nParam, double dParam)
{
	switch(state)
	{
	case eBollTrend_ZhangKou:
		{
			if(KLINE_DATA_SIZE >= m_nZhangKouTrendCheckCycle && REAL_BOLL_DATA_SIZE >= m_nZhangKouTrendCheckCycle)
			{
				int up = 0;
				int down = 0;
				bool bZhangkouUP = false;
				int dir = -1;
				for(int i = KLINE_DATA_SIZE-1; i>=KLINE_DATA_SIZE-m_nZhangKouTrendCheckCycle; --i)
				{
					if(KLINE_DATA[i].lowPrice >= BOLL_DATA[i].up)
					{
						if(dir == -1)
							dir = 1;
						up++;
					}
					else if(KLINE_DATA[i].lowPrice > BOLL_DATA[i].mb && KLINE_DATA[i].lowPrice < BOLL_DATA[i].up && KLINE_DATA[i].closePrice > KLINE_DATA[i].openPrice)
					{
						if(dir == -1)
							dir = 1;
						up++;
					}
					else if(KLINE_DATA[i].highPrice <= BOLL_DATA[i].dn)
					{
						if(dir == -1)
							dir = 0;
						down++;
					}
					else if(KLINE_DATA[i].highPrice < BOLL_DATA[i].mb && KLINE_DATA[i].highPrice > BOLL_DATA[i].dn && KLINE_DATA[i].closePrice < KLINE_DATA[i].openPrice)
					{
						if(dir == -1)
							dir = 0;
						down++;
					}
				}
				CString _szInfo = "";
				if(up > down)
					bZhangkouUP = true;
				else
					bZhangkouUP = false;
				if(up == 0 && down == 0)
					return;
				if(up == down)
				{
					if(dir == 1)
						bZhangkouUP = true;
					else
						bZhangkouUP = false;
				}
				m_nZhangKouConfirmBar = KLINE_DATA_SIZE-1;
				m_nZhangKouTradeCheckBar = m_nZhangKouConfirmBar;
				m_nZhangKouMinValue = dParam;
				m_bZhangKouUp = bZhangkouUP;
				m_lastZhangKouOrder = "";
				CString szInfo;
				szInfo.Format("张口产生<<<< 确认时间[%s] %s 趋势[%s %d:%d]", CFuncCommon::FormatTimeStr(KLINE_DATA[KLINE_DATA_SIZE-1].time).c_str(), (nParam==0 ? "开口角判断" : "柱体穿插判断"), (m_bZhangKouUp?"涨":"跌"), up, down);
				CActionLog("boll", szInfo.GetBuffer());
			}
			else
				return;
		}
		break;
	case eBollTrend_ShouKou:
		{
			std::string strConfirmTime = CFuncCommon::FormatTimeStr(KLINE_DATA[KLINE_DATA_SIZE-1].time);
			CActionLog("boll", "收口产生>>>> 确认时间[%s]", strConfirmTime.c_str());
			m_nShouKouConfirmBar = KLINE_DATA_SIZE-1;
			m_nShouKouTradeCheckBar = m_nShouKouConfirmBar;
		}
		break;
	case eBollTrend_ShouKouChannel:
		{
			std::string strConfirmTime = CFuncCommon::FormatTimeStr(KLINE_DATA[KLINE_DATA_SIZE-1].time);
			CActionLog("boll", "收口通道===== 确认时间[%s] %s", strConfirmTime.c_str(), (nParam==0 ? "趋势判断" : "超时判断"));
			m_nShouKouChannelConfirmBar = KLINE_DATA_SIZE-1;
			m_nLastTradeBarByShouKouChannel = 0;
		}
		break;
	default:
		break;
	}
	m_eLastBollState = m_eBollState;
	m_eBollState = state;
}

void COKExFuturesDlg::OnBnClickedButtonTest()
{
	if(!__SaveConfigCtrl())
		return;
	Test();
}

void COKExFuturesDlg::OnDestroy()
{
	delete pExchange;
	CLocalLogger::ReleaseInstance();
	CLocalActionLog::ReleaseInstance();
	CDialogEx::OnDestroy();

	// TODO:  在此处添加消息处理程序代码
}

void COKExFuturesDlg::Pong()
{
	m_tListenPong = 0;
}

void COKExFuturesDlg::OnRevTickerInfo(STickerData &data)
{
	m_curTickData = data;
	m_curTickData.bValid = true;
	if(KLINE_DATA_SIZE >= m_nBollCycle-1)
	{
		double totalClosePrice = 0.0;
		for(int i = KLINE_DATA_SIZE-1; i>=KLINE_DATA_SIZE-m_nBollCycle+1; --i)
		{
			totalClosePrice += KLINE_DATA[i].closePrice;
		}
		totalClosePrice += data.last;
		double ma = totalClosePrice/m_nBollCycle;

		double totalDifClosePriceSQ = 0.0;
		for(int i = KLINE_DATA_SIZE-1; i>=KLINE_DATA_SIZE-m_nBollCycle+1; --i)
		{
			totalDifClosePriceSQ += ((KLINE_DATA[i].closePrice - ma)*(KLINE_DATA[i].closePrice - ma));
		}
		totalDifClosePriceSQ += (data.last - ma)*(data.last - ma);

		double md = sqrt(totalDifClosePriceSQ/m_nBollCycle);
		m_curTickBoll.Reset();
		m_curTickBoll.mb = ma;
		m_curTickBoll.up = m_curTickBoll.mb + 2*md;
		m_curTickBoll.dn = m_curTickBoll.mb - 2*md;
		m_curTickBoll.mb = CFuncCommon::Round(m_curTickBoll.mb+DOUBLE_PRECISION, m_nPriceDecimal);
		m_curTickBoll.up = CFuncCommon::Round(m_curTickBoll.up+DOUBLE_PRECISION, m_nPriceDecimal);
		m_curTickBoll.dn = CFuncCommon::Round(m_curTickBoll.dn+DOUBLE_PRECISION, m_nPriceDecimal);
		if(!m_bTest)
			CActionLog("tick_boll", "up=%.2f mb=%.2f dn=%.2f", m_curTickBoll.up, m_curTickBoll.mb, m_curTickBoll.dn);
		switch(m_eBollState)
		{
		case eBollTrend_ShouKou:
			__CheckTrade_ShouKou();
			break;
		case eBollTrend_ShouKouChannel:
			__CheckTrade_ShouKouChannel();
			break;
		case eBollTrend_ZhangKou:
			__CheckTrade_ZhangKou();
			break;
		default:
			break;
		}
		__CheckTradeOrder();
	}
	bool bUpdate = false;
	std::list<SFuturesTradePairInfo>::iterator itB = m_listTradePairInfo.begin();
	std::list<SFuturesTradePairInfo>::iterator itE = m_listTradePairInfo.end();
	while(itB != itE)
	{
		if(itB->open.orderID != "" && !CFuncCommon::CheckEqual(itB->open.minPrice, 0.0) && !CFuncCommon::CheckEqual(itB->open.maxPrice, 0.0))
		{
			if(m_curTickData.last < itB->open.minPrice)
			{
				itB->open.minPrice = m_curTickData.last;
				bUpdate = true;
			}
			if(m_curTickData.last > itB->open.maxPrice)
			{
				itB->open.maxPrice = m_curTickData.last;
				bUpdate = true;
			}
		}
		if(itB->stopLoss.orderID != "" && !CFuncCommon::CheckEqual(itB->stopLoss.minPrice, 0.0) && !CFuncCommon::CheckEqual(itB->stopLoss.maxPrice, 0.0))
		{
			if(m_curTickData.last < itB->stopLoss.minPrice)
			{
				itB->stopLoss.minPrice = m_curTickData.last;
				bUpdate = true;
			}
			if(m_curTickData.last > itB->stopLoss.maxPrice)
			{
				itB->stopLoss.maxPrice = m_curTickData.last;
				bUpdate = true;
			}
		}
		++itB;
	}
	_UpdateTradeShow();
	std::string price = CFuncCommon::Double2String(m_curTickData.last + DOUBLE_PRECISION, m_nPriceDecimal);
	price += "[";
	price += CFuncCommon::FormatTimeStr(m_curTickData.time).c_str();
	price += "]";
	m_staticPrice.SetWindowText(price.c_str());
	if(bUpdate)
		_SaveData();
}

void COKExFuturesDlg::__CheckTrade_ZhangKou()
{
	if(!m_bSuccessLogin)
		return;
	if(m_bStopWhenFinish)
		return;
	if(m_tradeMoment == 1)
	{
		//确认张口后第一根柱子
		if(KLINE_DATA_SIZE - 1 - m_nZhangKouTradeCheckBar <= 1)
		{
			if(m_bZhangKouUp)
			{
				double fprice = KLINE_DATA[m_nZhangKouConfirmBar].closePrice;
				fprice = fprice*1.001;
				if(m_curTickData.last > fprice)
					fprice = m_curTickData.last;
				//if(m_curTickData.last < m_curTickBoll.up)
				{
					//挂空单
					if(__CheckCanTrade(eFuturesTradeType_OpenBear))
					{
						m_nZhangKouTradeCheckBar = 0;
						if(m_bTest)
						{
							std::string strClientOrderID = CFuncCommon::GenUUID();
							std::string price = CFuncCommon::Double2String(fprice + DOUBLE_PRECISION, m_nPriceDecimal);
							SFuturesTradePairInfo info;
							info.open.strClientOrderID = strClientOrderID;
							info.open.timeStamp = time(NULL);
							info.open.filledQTY = m_strFuturesTradeSize;
							info.open.orderID = CFuncCommon::GenUUID();
							info.open.price = fprice;
							info.open.priceAvg = fprice;
							info.open.status = 2;
							info.open.size = m_strFuturesTradeSize;
							info.open.tradeType = eFuturesTradeType_OpenBear;
							m_listTradePairInfo.push_back(info);
							CActionLog("trade", "[%s]开布林空单%s张, price=%s, client_oid=%s", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), m_strFuturesTradeSize.c_str(), price.c_str(), strClientOrderID.c_str());
							CActionLog("trade", "[%s]http更新订单信息 client_order=%s, order=%s, filledQTY=%s, price=%s, status=%s, tradeType=1", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), strClientOrderID.c_str(), info.open.orderID.c_str(), info.open.filledQTY.c_str(), price.c_str(), "2");
						}
						else
						{
							std::string strClientOrderID = CFuncCommon::GenUUID();
							std::string price = CFuncCommon::Double2String(fprice + DOUBLE_PRECISION, m_nPriceDecimal);
							OKEX_HTTP->API_FuturesTrade(m_bSwapFutures, eFuturesTradeType_OpenBear, m_strCoinType, m_strFuturesCycle, price, m_strFuturesTradeSize, m_strLeverage, strClientOrderID);
							SFuturesTradePairInfo info;
							info.open.strClientOrderID = strClientOrderID;
							info.open.waitClientOrderIDTime = time(NULL);
							info.open.tradeType = eFuturesTradeType_OpenBear;
							m_listTradePairInfo.push_back(info);
							CActionLog("trade", "开布林空单%s张, price=%s, client_oid=%s", m_strFuturesTradeSize.c_str(), price.c_str(), strClientOrderID.c_str());
						}
						m_bCanLogCheckCanTrade = true;
						m_bCanStopProfit = true;
					}
				}
			}
			else
			{
				double fprice = KLINE_DATA[m_nZhangKouConfirmBar].closePrice;
				fprice = fprice*0.999;
				if(m_curTickData.last < fprice)
					fprice = m_curTickData.last;
				//if(m_curTickData.last > m_curTickBoll.dn)
				{
					//挂多单
					if(__CheckCanTrade(eFuturesTradeType_OpenBull))
					{
						m_nZhangKouTradeCheckBar = 0;
						if(m_bTest)
						{
							std::string strClientOrderID = CFuncCommon::GenUUID();
							std::string price = CFuncCommon::Double2String(fprice + DOUBLE_PRECISION, m_nPriceDecimal);
							SFuturesTradePairInfo info;
							info.open.strClientOrderID = strClientOrderID;
							info.open.timeStamp = time(NULL);
							info.open.filledQTY = m_strFuturesTradeSize;
							info.open.orderID = CFuncCommon::GenUUID();
							info.open.price = fprice;
							info.open.priceAvg = fprice;
							info.open.status = 2;
							info.open.tradeType = eFuturesTradeType_OpenBull;
							info.open.size = m_strFuturesTradeSize;
							m_listTradePairInfo.push_back(info);
							CActionLog("trade", "[%s]开布林多单%s张 price=%s, client_oid=%s", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), m_strFuturesTradeSize.c_str(), price.c_str(), strClientOrderID.c_str());
							CActionLog("trade", "[%s]http更新订单信息 client_order=%s, order=%s, filledQTY=%s, price=%s, status=%s, tradeType=2", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), strClientOrderID.c_str(), info.open.orderID.c_str(), info.open.filledQTY.c_str(), price.c_str(), "2");
						}
						else
						{
							std::string strClientOrderID = CFuncCommon::GenUUID();
							std::string price = CFuncCommon::Double2String(fprice + DOUBLE_PRECISION, m_nPriceDecimal);
							OKEX_HTTP->API_FuturesTrade(m_bSwapFutures, eFuturesTradeType_OpenBull, m_strCoinType, m_strFuturesCycle, price, m_strFuturesTradeSize, m_strLeverage, strClientOrderID);
							SFuturesTradePairInfo info;
							info.open.strClientOrderID = strClientOrderID;
							info.open.waitClientOrderIDTime = time(NULL);
							info.open.tradeType = eFuturesTradeType_OpenBull;
							m_listTradePairInfo.push_back(info);
							CActionLog("trade", "开布林多单%s张 price=%s, client_oid=%s", m_strFuturesTradeSize.c_str(), price.c_str(), strClientOrderID.c_str());
						}
						m_bCanLogCheckCanTrade = true;
						m_bCanStopProfit = true;
					}
				}
			}
		}
	}
	else if(m_tradeMoment == 0)
	{
		//确认张口后第一根柱子
		if(KLINE_DATA_SIZE - 1 - m_nZhangKouTradeCheckBar <= 1)
		{
			if(m_bZhangKouUp)
			{
				if((KLINE_DATA[m_nZhangKouConfirmBar].closePrice > KLINE_DATA[m_nZhangKouConfirmBar].openPrice) && ((KLINE_DATA[m_nZhangKouConfirmBar].closePrice - KLINE_DATA[m_nZhangKouConfirmBar].openPrice) / KLINE_DATA[m_nZhangKouConfirmBar].openPrice) > 0.04)
					return;
				double rate = BOLL_DATA[m_nZhangKouTradeCheckBar].up / BOLL_DATA[m_nZhangKouTradeCheckBar - 1].up;
				double fprice = BOLL_DATA[m_nZhangKouTradeCheckBar].up * rate;
				//if(m_curTickData.last < m_curTickBoll.up)
				{
					//挂多单
					if(__CheckCanTrade(eFuturesTradeType_OpenBull))
					{
						m_nZhangKouTradeCheckBar = 0;
						if(m_bTest)
						{
							std::string strClientOrderID = CFuncCommon::GenUUID();
							std::string price = CFuncCommon::Double2String(fprice + DOUBLE_PRECISION, m_nPriceDecimal);
							SFuturesTradePairInfo info;
							info.open.strClientOrderID = strClientOrderID;
							info.open.timeStamp = time(NULL);
							info.open.filledQTY = m_strFuturesTradeSize;
							info.open.orderID = CFuncCommon::GenUUID();
							info.open.price = fprice;
							info.open.priceAvg = fprice;
							info.open.status = 2;
							info.open.size = m_strFuturesTradeSize;
							info.open.tradeType = eFuturesTradeType_OpenBull;
							info.open.triggerType = 0;
							m_listTradePairInfo.push_back(info);
							CActionLog("trade", "[%s]开布林多单%s张, price=%s, client_oid=%s", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), m_strFuturesTradeSize.c_str(), price.c_str(), strClientOrderID.c_str());
							CActionLog("trade", "[%s]http更新订单信息 client_order=%s, order=%s, filledQTY=%s, price=%s, status=%s, tradeType=1", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), strClientOrderID.c_str(), info.open.orderID.c_str(), info.open.filledQTY.c_str(), price.c_str(), "2");
						}
						else
						{
							std::string strClientOrderID = CFuncCommon::GenUUID();
							std::string price = CFuncCommon::Double2String(fprice + DOUBLE_PRECISION, m_nPriceDecimal);
							OKEX_HTTP->API_FuturesTrade(m_bSwapFutures, eFuturesTradeType_OpenBull, m_strCoinType, m_strFuturesCycle, price, m_strFuturesTradeSize, m_strLeverage, strClientOrderID);
							SFuturesTradePairInfo info;
							info.open.strClientOrderID = strClientOrderID;
							info.open.waitClientOrderIDTime = time(NULL);
							info.open.tradeType = eFuturesTradeType_OpenBull;
							info.open.triggerType = 0;
							m_listTradePairInfo.push_back(info);
							CActionLog("trade", "开布林多单%s张, price=%s, client_oid=%s", m_strFuturesTradeSize.c_str(), price.c_str(), strClientOrderID.c_str());
						}
						m_bCanLogCheckCanTrade = true;
						m_bCanStopProfit = true;
					}
				}
			}
			else
			{
				if((KLINE_DATA[m_nZhangKouConfirmBar].closePrice < KLINE_DATA[m_nZhangKouConfirmBar].openPrice) && ((KLINE_DATA[m_nZhangKouConfirmBar].openPrice - KLINE_DATA[m_nZhangKouConfirmBar].closePrice) / KLINE_DATA[m_nZhangKouConfirmBar].openPrice) > 0.04)
					return;
				double rate = BOLL_DATA[m_nZhangKouTradeCheckBar].dn / BOLL_DATA[m_nZhangKouTradeCheckBar - 1].dn;
				double fprice = BOLL_DATA[m_nZhangKouTradeCheckBar].dn * rate;
				//if(m_curTickData.last > m_curTickBoll.dn)
				{
					//挂空单
					if(__CheckCanTrade(eFuturesTradeType_OpenBear))
					{
						m_nZhangKouTradeCheckBar = 0;
						if(m_bTest)
						{
							std::string strClientOrderID = CFuncCommon::GenUUID();
							std::string price = CFuncCommon::Double2String(fprice + DOUBLE_PRECISION, m_nPriceDecimal);
							SFuturesTradePairInfo info;
							info.open.strClientOrderID = strClientOrderID;
							info.open.timeStamp = time(NULL);
							info.open.filledQTY = m_strFuturesTradeSize;
							info.open.orderID = CFuncCommon::GenUUID();
							info.open.price = fprice;
							info.open.priceAvg = fprice;
							info.open.status = 2;
							info.open.tradeType = eFuturesTradeType_OpenBear;
							info.open.size = m_strFuturesTradeSize;
							info.open.triggerType = 0;
							m_listTradePairInfo.push_back(info);
							CActionLog("trade", "[%s]开布林空单%s张 price=%s, client_oid=%s", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), m_strFuturesTradeSize.c_str(), price.c_str(), strClientOrderID.c_str());
							CActionLog("trade", "[%s]http更新订单信息 client_order=%s, order=%s, filledQTY=%s, price=%s, status=%s, tradeType=2", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), strClientOrderID.c_str(), info.open.orderID.c_str(), info.open.filledQTY.c_str(), price.c_str(), "2");
						}
						else
						{
							std::string strClientOrderID = CFuncCommon::GenUUID();
							std::string price = CFuncCommon::Double2String(fprice + DOUBLE_PRECISION, m_nPriceDecimal);
							OKEX_HTTP->API_FuturesTrade(m_bSwapFutures, eFuturesTradeType_OpenBear, m_strCoinType, m_strFuturesCycle, price, m_strFuturesTradeSize, m_strLeverage, strClientOrderID);
							SFuturesTradePairInfo info;
							info.open.strClientOrderID = strClientOrderID;
							info.open.waitClientOrderIDTime = time(NULL);
							info.open.tradeType = eFuturesTradeType_OpenBear;
							info.open.triggerType = 0;
							m_listTradePairInfo.push_back(info);
							CActionLog("trade", "开布林空单%s张 price=%s, client_oid=%s", m_strFuturesTradeSize.c_str(), price.c_str(), strClientOrderID.c_str());
						}
						m_bCanLogCheckCanTrade = true;
						m_bCanStopProfit = true;
					}
				}
			}
		}
	}
}

void COKExFuturesDlg::__CheckTrade_ShouKou()
{
	return;
	if(!m_bSuccessLogin)
		return;
	if(m_bStopWhenFinish)
		return;
	if(m_tradeMoment != 1)
		return;
	//确认张口后第一根柱子
	if(m_eLastBollState == eBollTrend_ZhangKou && m_nShouKouTradeCheckBar)
	{
		if(m_bZhangKouUp)
		{
			//if(m_curTickData.last < m_curTickBoll.up)
			{
				//用买一价格挂多单
				if(__CheckCanTrade(eFuturesTradeType_OpenBear))
				{
					m_nShouKouTradeCheckBar = 0;
					if(m_bTest)
					{
						std::string strClientOrderID = CFuncCommon::GenUUID();
						std::string price = CFuncCommon::Double2String(m_curTickData.sell + DOUBLE_PRECISION, m_nPriceDecimal);
						SFuturesTradePairInfo info;
						info.open.strClientOrderID = strClientOrderID;
						info.open.timeStamp = time(NULL);
						info.open.filledQTY = m_strFuturesTradeSize;
						info.open.orderID = CFuncCommon::GenUUID();
						info.open.price = m_curTickData.buy;
						info.open.priceAvg = m_curTickData.buy;
						info.open.status = 2;
						info.open.tradeType = eFuturesTradeType_OpenBear;
						m_listTradePairInfo.push_back(info);
						CActionLog("trade", "[%s]开空单%s张, price=%s, client_oid=%s", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), m_strFuturesTradeSize.c_str(), price.c_str(), strClientOrderID.c_str());
						CActionLog("trade", "[%s]http更新订单信息 client_order=%s, order=%s, filledQTY=%s, price=%s, status=%s, tradeType=2", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), strClientOrderID.c_str(), info.open.orderID.c_str(), info.open.filledQTY.c_str(), price.c_str(), "2");
					}
					else
					{
						std::string strClientOrderID = CFuncCommon::GenUUID();
						std::string price = CFuncCommon::Double2String(m_curTickData.sell + DOUBLE_PRECISION, m_nPriceDecimal);
						OKEX_HTTP->API_FuturesTrade(m_bSwapFutures, eFuturesTradeType_OpenBear, m_strCoinType, m_strFuturesCycle, price, m_strFuturesTradeSize, m_strLeverage, strClientOrderID);
						SFuturesTradePairInfo info;
						info.open.strClientOrderID = strClientOrderID;
						info.open.waitClientOrderIDTime = time(NULL);
						info.open.tradeType = eFuturesTradeType_OpenBear;
						m_listTradePairInfo.push_back(info);
						CActionLog("trade", "开空单%s张, price=%s, client_oid=%s", m_strFuturesTradeSize.c_str(), price.c_str(), strClientOrderID.c_str());
					}
					m_bCanLogCheckCanTrade = true;
					m_bCanStopProfit = true;
				}
			}
		}
		else
		{
			//if(m_curTickData.last > m_curTickBoll.dn)
			{
				//用卖一价格挂空单
				if(__CheckCanTrade(eFuturesTradeType_OpenBull))
				{
					m_nShouKouTradeCheckBar = 0;
					if(m_bTest)
					{
						std::string strClientOrderID = CFuncCommon::GenUUID();
						std::string price = CFuncCommon::Double2String(m_curTickData.buy + DOUBLE_PRECISION, m_nPriceDecimal);
						SFuturesTradePairInfo info;
						info.open.strClientOrderID = strClientOrderID;
						info.open.timeStamp = time(NULL);
						info.open.filledQTY = m_strFuturesTradeSize.c_str();
						info.open.orderID = CFuncCommon::GenUUID();
						info.open.price = m_curTickData.sell;
						info.open.priceAvg = m_curTickData.sell;
						info.open.status = 2;
						info.open.tradeType = eFuturesTradeType_OpenBull;
						m_listTradePairInfo.push_back(info);
						CActionLog("trade", "[%s]开多单%s张 price=%s, client_oid=%s", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), m_strFuturesTradeSize.c_str(), price.c_str(), strClientOrderID.c_str());
						CActionLog("trade", "[%s]http更新订单信息 client_order=%s, order=%s, filledQTY=%s, price=%s, status=%s, tradeType=1", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), strClientOrderID.c_str(), info.open.orderID.c_str(), info.open.filledQTY.c_str(), price.c_str(), "2");
					}
					else
					{
						std::string strClientOrderID = CFuncCommon::GenUUID();
						std::string price = CFuncCommon::Double2String(m_curTickData.buy + DOUBLE_PRECISION, m_nPriceDecimal);
						OKEX_HTTP->API_FuturesTrade(m_bSwapFutures, eFuturesTradeType_OpenBull, m_strCoinType, m_strFuturesCycle, price, m_strFuturesTradeSize, m_strLeverage, strClientOrderID);
						SFuturesTradePairInfo info;
						info.open.strClientOrderID = strClientOrderID;
						info.open.waitClientOrderIDTime = time(NULL);
						info.open.tradeType = eFuturesTradeType_OpenBull;
						m_listTradePairInfo.push_back(info);
						CActionLog("trade", "开多单%s张 price=%s, client_oid=%s", m_strFuturesTradeSize.c_str(), price.c_str(), strClientOrderID.c_str());
					}
					m_bCanLogCheckCanTrade = true;
					m_bCanStopProfit = true;
				}
			}
		}
	}
}

void COKExFuturesDlg::__CheckTrade_ShouKouChannel()
{
	if(!m_bSuccessLogin)
		return;
	//柱子穿插md, 如果中间纠缠的柱子不超过两个则确定趋势
	if(REAL_BOLL_DATA_SIZE >= 6 && (KLINE_DATA_SIZE-1-m_nLastTradeBarByShouKouChannel) > 8)
	{
		//第1根在mb上,并且是绿柱
		if(KLINE_DATA[KLINE_DATA_SIZE-1].lowPrice > BOLL_DATA[BOLL_DATA_SIZE-1].mb && KLINE_DATA[KLINE_DATA_SIZE-1].closePrice > KLINE_DATA[KLINE_DATA_SIZE-1].openPrice)
		{
			//第2根在mb上,并且第1根收价高于第2根
			if(KLINE_DATA[KLINE_DATA_SIZE-2].lowPrice > BOLL_DATA[BOLL_DATA_SIZE-2].mb && KLINE_DATA[KLINE_DATA_SIZE-1].closePrice > KLINE_DATA[KLINE_DATA_SIZE-2].closePrice)
			{
				//第3根开始全在mb之下 或  第3根纠缠在mb,第4,5,6根在mb之下  或 第3,4根纠缠在mb,第5,6根在mb之下
				bool good = false;
				if(KLINE_DATA[KLINE_DATA_SIZE-3].highPrice < BOLL_DATA[BOLL_DATA_SIZE-3].mb && KLINE_DATA[KLINE_DATA_SIZE-4].highPrice < BOLL_DATA[BOLL_DATA_SIZE-4].mb && KLINE_DATA[KLINE_DATA_SIZE-5].highPrice < BOLL_DATA[BOLL_DATA_SIZE-5].mb && KLINE_DATA[KLINE_DATA_SIZE-6].highPrice < BOLL_DATA[BOLL_DATA_SIZE-6].mb)
					good = true;
				else if(KLINE_DATA[KLINE_DATA_SIZE-3].highPrice > BOLL_DATA[BOLL_DATA_SIZE-3].mb && KLINE_DATA[KLINE_DATA_SIZE-3].lowPrice < BOLL_DATA[BOLL_DATA_SIZE-3].mb &&
						KLINE_DATA[KLINE_DATA_SIZE-4].highPrice < BOLL_DATA[BOLL_DATA_SIZE-4].mb && KLINE_DATA[KLINE_DATA_SIZE-5].highPrice < BOLL_DATA[BOLL_DATA_SIZE-5].mb && KLINE_DATA[KLINE_DATA_SIZE-6].highPrice < BOLL_DATA[BOLL_DATA_SIZE-6].mb)
					good = true;
				else if(KLINE_DATA[KLINE_DATA_SIZE-3].highPrice > BOLL_DATA[BOLL_DATA_SIZE-3].mb && KLINE_DATA[KLINE_DATA_SIZE-3].lowPrice < BOLL_DATA[BOLL_DATA_SIZE-3].mb &&
						KLINE_DATA[KLINE_DATA_SIZE-4].highPrice > BOLL_DATA[BOLL_DATA_SIZE-4].mb && KLINE_DATA[KLINE_DATA_SIZE-4].lowPrice < BOLL_DATA[BOLL_DATA_SIZE-4].mb &&
						KLINE_DATA[KLINE_DATA_SIZE-5].highPrice < BOLL_DATA[BOLL_DATA_SIZE-5].mb && KLINE_DATA[KLINE_DATA_SIZE-6].highPrice < BOLL_DATA[BOLL_DATA_SIZE-6].mb)
					good = true;
				if(good)
				{
					if(__CheckCanTrade(eFuturesTradeType_OpenBull))
					{
						m_nLastTradeBarByShouKouChannel = KLINE_DATA_SIZE-1;
						double fprice = m_curTickData.buy;
						if(m_bTest)
						{
							std::string strClientOrderID = CFuncCommon::GenUUID();
							std::string price = CFuncCommon::Double2String(fprice + DOUBLE_PRECISION, m_nPriceDecimal);
							SFuturesTradePairInfo info;
							info.open.strClientOrderID = strClientOrderID;
							info.open.timeStamp = time(NULL);
							info.open.filledQTY = m_strFuturesTradeSize;
							info.open.orderID = CFuncCommon::GenUUID();
							info.open.price = fprice;
							info.open.priceAvg = fprice;
							info.open.status = 2;
							info.open.tradeType = eFuturesTradeType_OpenBull;
							info.open.size = m_strFuturesTradeSize;
							info.open.triggerType = 1;
							m_listTradePairInfo.push_back(info);
							CActionLog("trade", "[%s]开穿刺多单%s张 price=%s, client_oid=%s", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), m_strFuturesTradeSize.c_str(), price.c_str(), strClientOrderID.c_str());
							CActionLog("trade", "[%s]http更新订单信息 client_order=%s, order=%s, filledQTY=%s, price=%s, status=%s, tradeType=2", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), strClientOrderID.c_str(), info.open.orderID.c_str(), info.open.filledQTY.c_str(), price.c_str(), "2");
						}
						else
						{
							std::string strClientOrderID = CFuncCommon::GenUUID();
							std::string price = CFuncCommon::Double2String(fprice + DOUBLE_PRECISION, m_nPriceDecimal);
							OKEX_HTTP->API_FuturesTrade(m_bSwapFutures, eFuturesTradeType_OpenBull, m_strCoinType, m_strFuturesCycle, price, m_strFuturesTradeSize, m_strLeverage, strClientOrderID);
							SFuturesTradePairInfo info;
							info.open.strClientOrderID = strClientOrderID;
							info.open.waitClientOrderIDTime = time(NULL);
							info.open.tradeType = eFuturesTradeType_OpenBull;
							info.open.triggerType = 1;
							m_listTradePairInfo.push_back(info);
							CActionLog("trade", "开穿刺多单%s张 price=%s, client_oid=%s", m_strFuturesTradeSize.c_str(), price.c_str(), strClientOrderID.c_str());
						}
						m_bCanLogCheckCanTrade = true;
						m_bCanStopProfit = true;
					}

				}
			}
		}//第1根在mb下,并且是红柱
		else if(KLINE_DATA[KLINE_DATA_SIZE-1].highPrice < BOLL_DATA[BOLL_DATA_SIZE-1].mb && KLINE_DATA[KLINE_DATA_SIZE-1].closePrice < KLINE_DATA[KLINE_DATA_SIZE-1].openPrice)
		{
			//第2根在mb下,并且第1根收价低于第2根
			if(KLINE_DATA[KLINE_DATA_SIZE-2].highPrice < BOLL_DATA[BOLL_DATA_SIZE-2].mb && KLINE_DATA[KLINE_DATA_SIZE-1].closePrice < KLINE_DATA[KLINE_DATA_SIZE-2].closePrice)
			{
				//第3根开始全在mb之上 或  第3根纠缠在mb,第4,5,6根在mb之上  或 第3,4根纠缠在mb,第5,6根在mb之上
				bool good = false;
				if(KLINE_DATA[KLINE_DATA_SIZE-3].lowPrice > BOLL_DATA[BOLL_DATA_SIZE-3].mb && KLINE_DATA[KLINE_DATA_SIZE-4].lowPrice > BOLL_DATA[BOLL_DATA_SIZE-4].mb && KLINE_DATA[KLINE_DATA_SIZE-5].lowPrice > BOLL_DATA[BOLL_DATA_SIZE-5].mb && KLINE_DATA[KLINE_DATA_SIZE-6].lowPrice > BOLL_DATA[BOLL_DATA_SIZE-6].mb)
					good = true;
				else if(KLINE_DATA[KLINE_DATA_SIZE-3].highPrice > BOLL_DATA[BOLL_DATA_SIZE-3].mb && KLINE_DATA[KLINE_DATA_SIZE-3].lowPrice < BOLL_DATA[BOLL_DATA_SIZE-3].mb &&
					KLINE_DATA[KLINE_DATA_SIZE-4].lowPrice > BOLL_DATA[BOLL_DATA_SIZE-4].mb && KLINE_DATA[KLINE_DATA_SIZE-5].lowPrice > BOLL_DATA[BOLL_DATA_SIZE-5].mb && KLINE_DATA[KLINE_DATA_SIZE-6].lowPrice > BOLL_DATA[BOLL_DATA_SIZE-6].mb)
					good = true;
				else if(KLINE_DATA[KLINE_DATA_SIZE-3].highPrice > BOLL_DATA[BOLL_DATA_SIZE-3].mb && KLINE_DATA[KLINE_DATA_SIZE-3].lowPrice < BOLL_DATA[BOLL_DATA_SIZE-3].mb &&
					KLINE_DATA[KLINE_DATA_SIZE-4].highPrice > BOLL_DATA[BOLL_DATA_SIZE-4].mb && KLINE_DATA[KLINE_DATA_SIZE-4].lowPrice < BOLL_DATA[BOLL_DATA_SIZE-4].mb &&
					KLINE_DATA[KLINE_DATA_SIZE-5].lowPrice > BOLL_DATA[BOLL_DATA_SIZE-5].mb && KLINE_DATA[KLINE_DATA_SIZE-6].lowPrice > BOLL_DATA[BOLL_DATA_SIZE-6].mb)
					good = true;
				if(good)
				{
					if(__CheckCanTrade(eFuturesTradeType_OpenBear))
					{
						m_nLastTradeBarByShouKouChannel = KLINE_DATA_SIZE-1;
						double fprice = m_curTickData.sell;
						if(m_bTest)
						{
							std::string strClientOrderID = CFuncCommon::GenUUID();
							std::string price = CFuncCommon::Double2String(fprice + DOUBLE_PRECISION, m_nPriceDecimal);
							SFuturesTradePairInfo info;
							info.open.strClientOrderID = strClientOrderID;
							info.open.timeStamp = time(NULL);
							info.open.filledQTY = m_strFuturesTradeSize;
							info.open.orderID = CFuncCommon::GenUUID();
							info.open.price = fprice;
							info.open.priceAvg = fprice;
							info.open.status = 2;
							info.open.tradeType = eFuturesTradeType_OpenBear;
							info.open.size = m_strFuturesTradeSize;
							info.open.triggerType = 1;
							m_listTradePairInfo.push_back(info);
							CActionLog("trade", "[%s]开穿刺空单%s张 price=%s, client_oid=%s", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), m_strFuturesTradeSize.c_str(), price.c_str(), strClientOrderID.c_str());
							CActionLog("trade", "[%s]http更新订单信息 client_order=%s, order=%s, filledQTY=%s, price=%s, status=%s, tradeType=2", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), strClientOrderID.c_str(), info.open.orderID.c_str(), info.open.filledQTY.c_str(), price.c_str(), "2");
						}
						else
						{
							std::string strClientOrderID = CFuncCommon::GenUUID();
							std::string price = CFuncCommon::Double2String(fprice + DOUBLE_PRECISION, m_nPriceDecimal);
							OKEX_HTTP->API_FuturesTrade(m_bSwapFutures, eFuturesTradeType_OpenBear, m_strCoinType, m_strFuturesCycle, price, m_strFuturesTradeSize, m_strLeverage, strClientOrderID);
							SFuturesTradePairInfo info;
							info.open.strClientOrderID = strClientOrderID;
							info.open.waitClientOrderIDTime = time(NULL);
							info.open.tradeType = eFuturesTradeType_OpenBear;
							info.open.triggerType = 1;
							m_listTradePairInfo.push_back(info);
							CActionLog("trade", "开穿刺空单%s张 price=%s, client_oid=%s", m_strFuturesTradeSize.c_str(), price.c_str(), strClientOrderID.c_str());
						}
						m_bCanLogCheckCanTrade = true;
						m_bCanStopProfit = true;
					}
				}
			}
		}
	}
}

void COKExFuturesDlg::__CheckTradeOrder()
{
	if(!m_bSuccessLogin)
		return;
	std::list<SFuturesTradePairInfo>::iterator itB = m_listTradePairInfo.begin();
	std::list<SFuturesTradePairInfo>::iterator itE = m_listTradePairInfo.end();
	//先统计当前未平仓的多空数量
	bool bUpdate = false;
	bool bStopProfitFail = false;
	while(itB != itE)
	{
		//如果交易完成就删除
		bool bOpenFinish = ((itB->open.status == "-1") || (itB->open.status == "2"));
		bool bCloseFinish = (itB->close.status == "2");
		if(bOpenFinish && bCloseFinish)
		{
			std::string openPrice = CFuncCommon::Double2String(itB->open.priceAvg+DOUBLE_PRECISION, m_nPriceDecimal);
			std::string closePrice = CFuncCommon::Double2String(itB->close.priceAvg+DOUBLE_PRECISION, m_nPriceDecimal);
			bool bWin = false;
			if(itB->open.tradeType == eFuturesTradeType_OpenBull)
				bWin = (itB->close.priceAvg>itB->open.priceAvg);
			else if(itB->open.tradeType == eFuturesTradeType_OpenBear)
				bWin = (itB->close.priceAvg<itB->open.priceAvg);
			CActionLog("finish_trade", "删除已完成%s%s交易对[%s] open_price=%s, open_num=%s, open_order=%s, close_price=%s, close_num=%s, close_order=%s", ((itB->open.triggerType==0) ? "布林" : "穿刺"), ((itB->open.tradeType == eFuturesTradeType_OpenBull) ? "多单" : "空单"), (bWin ? "赚" : "亏"), openPrice.c_str(), itB->open.filledQTY.c_str(), itB->open.orderID.c_str(), closePrice.c_str(), itB->close.filledQTY.c_str(), itB->close.orderID.c_str());
			itB = m_listTradePairInfo.erase(itB);
			m_bCanLogCheckCanTrade = true;
			m_bCanStopProfit = true;
			_UpdateTradeShow();
			bUpdate = true;
			continue;
		}
		if(bOpenFinish && itB->open.filledQTY == "0" && itB->close.strClientOrderID == "")
		{
			CActionLog("trade", "删除未完成交易对 order=%s", itB->open.orderID.c_str());
			itB = m_listTradePairInfo.erase(itB);
			m_bCanLogCheckCanTrade = true;
			m_bCanStopProfit = true;
			_UpdateTradeShow();
			bUpdate = true;
			continue;
		}
		time_t tNow = time(NULL);
		//超时订单删除
		if(itB->open.orderID != "" && itB->open.status == "0" && itB->open.timeStamp && tNow-itB->open.timeStamp>60*10 && !CFuncCommon::CheckEqual(itB->open.price, 0.0))
		{
			if(!m_bTest)
				OKEX_HTTP->API_FuturesCancelOrder(m_bSwapFutures, m_strCoinType, m_strFuturesCycle, itB->open.orderID);
			CActionLog("trade", "删除超时交易对 order=%s", itB->open.orderID.c_str());
			itB = m_listTradePairInfo.erase(itB);
			m_bCanLogCheckCanTrade = true;
			m_bCanStopProfit = true;
			_UpdateTradeShow();
			bUpdate = true;
			continue;
		}
		if(itB->close.orderID != "" && itB->close.status == "0" && itB->close.timeStamp && tNow-itB->close.timeStamp>60*10 && !CFuncCommon::CheckEqual(itB->close.price, 0.0))
		{
			CActionLog("trade", "删除超时平仓单 order=%s", itB->close.orderID.c_str());
			if(!m_bTest)
				OKEX_HTTP->API_FuturesCancelOrder(m_bSwapFutures, m_strCoinType, m_strFuturesCycle, itB->close.orderID);
			itB->open.stopProfit = 0;
			itB->close.Reset();
			_UpdateTradeShow();
			bUpdate = true;
		}
		if(itB->stopLoss.orderID != "" && itB->stopLoss.status == "0" && itB->stopLoss.timeStamp && tNow - itB->stopLoss.timeStamp > 60 * 10 && !CFuncCommon::CheckEqual(itB->stopLoss.price, 0.0))
		{
			CActionLog("trade", "删除超时止损单 order=%s", itB->stopLoss.orderID.c_str());
			if(!m_bTest)
				OKEX_HTTP->API_FuturesCancelOrder(m_bSwapFutures, m_strCoinType, m_strFuturesCycle, itB->stopLoss.orderID);
			itB->stopLoss.Reset();
			_UpdateTradeShow();
			bUpdate = true;
		}
		//如果已进行平仓交易,等待平仓完成
		//否则判断开仓的盈亏
		if(itB->close.orderID != "" || itB->close.strClientOrderID != "")
		{
		
		}
		else 
		{
			if(itB->stopLoss.orderID != "" && !CFuncCommon::CheckEqual(itB->stopLoss.priceAvg, 0.0) && m_curTickData.bValid)
			{
				//止损单也要做止盈操作
				if(itB->stopLoss.tradeType == eFuturesTradeType_OpenBull)
				{
					if(m_curTickData.last > itB->stopLoss.priceAvg)
					{
						if(itB->stopLoss.stopProfit)
						{
							double half = m_moveStopProfit/2;
							if(m_curTickData.last <= itB->stopLoss.priceAvg*(1+itB->stopLoss.stopProfit*m_moveStopProfit+half))
							{
								//如果stopLoss未交易完,先撤单
								if(itB->stopLoss.status == "1")
								{
									if(m_bTest)
									{
										itB->stopLoss.status = "-1";
										CActionLog("trade", "[%s]撤消订单成功 order=%s", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), itB->stopLoss.orderID.c_str());
									}
									else
										OKEX_HTTP->API_FuturesCancelOrder(m_bSwapFutures, m_strCoinType, m_strFuturesCycle, itB->stopLoss.orderID);
								}
								if(itB->stopLoss.filledQTY != "0")
								{
									std::string price = "-1";
									if(m_bTest)
									{
										CActionLog("trade", "[%s]止盈平多损 stopClientOrder=%s, order=%s, price=%s, filledQTY=%s", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), itB->stopLoss.strClientOrderID.c_str(), itB->stopLoss.orderID.c_str(), price.c_str(), itB->stopLoss.filledQTY.c_str());
									}
									else
									{
										std::string strClientOrderID = CFuncCommon::GenUUID();
										OKEX_HTTP->API_FuturesTrade(m_bSwapFutures, eFuturesTradeType_CloseBull, m_strCoinType, m_strFuturesCycle, price, itB->stopLoss.filledQTY, m_strLeverage, strClientOrderID);
										CActionLog("trade", "止盈平多损 stopClientOrder=%s, order=%s, price=%s, filledQTY=%s", itB->stopLoss.strClientOrderID.c_str(), itB->stopLoss.orderID.c_str(), price.c_str(), itB->stopLoss.filledQTY.c_str());
									}
								}
								itB->open.lastSellStopLossPrice = m_curTickData.last - itB->stopLoss.priceAvg*(m_stopLoss/2+0.0025);
								itB->stopLoss.Reset();
								bUpdate = true;
							}
							else
							{
								double up = (m_curTickData.last - itB->stopLoss.priceAvg) / itB->stopLoss.priceAvg;
								int nowStep = int(up / m_moveStopProfit);
								if(nowStep - itB->stopLoss.stopProfit >= 2)
								{
									itB->stopLoss.stopProfit = nowStep - 1;
									bUpdate = true;
								}
							}
						}
						else
						{
							double up = (m_curTickData.last - itB->stopLoss.priceAvg) / itB->stopLoss.priceAvg;
							int nowStep = int(up / m_moveStopProfit);
							if(nowStep - itB->stopLoss.stopProfit >= 2)
							{
								itB->stopLoss.stopProfit = nowStep - 1;
								bUpdate = true;
							}
						}
					}
				}
				else if(itB->stopLoss.tradeType == eFuturesTradeType_OpenBear)
				{
					if(m_curTickData.last < itB->stopLoss.priceAvg)
					{
						if(itB->stopLoss.stopProfit)
						{
							double half = m_moveStopProfit / 2;
							//平仓
							if(m_curTickData.last >= itB->stopLoss.priceAvg*(1 - itB->stopLoss.stopProfit*m_moveStopProfit - half))
							{
								//如果stopLoss未交易完,先撤单
								if(itB->stopLoss.status == "1")
								{
									if(m_bTest)
									{
										itB->stopLoss.status = "-1";
										CActionLog("trade", "[%s]撤消订单成功 order=%s", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), itB->stopLoss.orderID.c_str());
									}
									else
										OKEX_HTTP->API_FuturesCancelOrder(m_bSwapFutures, m_strCoinType, m_strFuturesCycle, itB->stopLoss.orderID);
								}
								if(itB->stopLoss.filledQTY != "0")
								{
									std::string price = "-1";
									if(m_bTest)
									{
										CActionLog("trade", "[%s]止盈平空损 order=%s, price=%s, filledQTY=%s", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), itB->stopLoss.orderID.c_str(), price.c_str(), itB->stopLoss.filledQTY.c_str());
									}
									else
									{
										std::string strClientOrderID = CFuncCommon::GenUUID();
										OKEX_HTTP->API_FuturesTrade(m_bSwapFutures, eFuturesTradeType_CloseBear, m_strCoinType, m_strFuturesCycle, price, itB->stopLoss.filledQTY, m_strLeverage, strClientOrderID);
										CActionLog("trade", "止盈平空损 order=%s, price=%s, filledQTY=%s", itB->stopLoss.orderID.c_str(), price.c_str(), itB->stopLoss.filledQTY.c_str());
									}
								}
								itB->open.lastSellStopLossPrice = m_curTickData.last + itB->stopLoss.priceAvg*(m_stopLoss/2+0.0025);
								itB->stopLoss.Reset();
								bUpdate = true;
							}
							else
							{
								double up = (itB->stopLoss.priceAvg - m_curTickData.last) / itB->stopLoss.priceAvg;
								int nowStep = int(up / m_moveStopProfit);
								if(nowStep - itB->stopLoss.stopProfit >= 2)
								{
									itB->stopLoss.stopProfit = nowStep - 1;
									bUpdate = true;
								}
							}
						}
						else
						{
							double up = (itB->stopLoss.priceAvg - m_curTickData.last) / itB->stopLoss.priceAvg;
							int nowStep = int(up / m_moveStopProfit);
							if(nowStep - itB->stopLoss.stopProfit >= 2)
							{
								itB->stopLoss.stopProfit = nowStep - 1;
								bUpdate = true;
							}
						}
					}
				}
				
			}
			if(itB->open.orderID != "" && !CFuncCommon::CheckEqual(itB->open.priceAvg, 0.0))
			{
				//多仓
				if(itB->open.tradeType == eFuturesTradeType_OpenBull)
				{
					if(m_curTickData.bValid)
					{
						//超过止损线 平仓
						double checkPrice = itB->open.priceAvg;
						if(!CFuncCommon::CheckEqual(itB->open.lastSellStopLossPrice, 0.0))
						{
							if(((itB->open.priceAvg-m_curTickData.last)/itB->open.priceAvg) <= m_stopLoss/2)
							{
								itB->open.lastSellStopLossPrice = 0.0;
								bUpdate = true;
							}
							else
								checkPrice = itB->open.lastSellStopLossPrice;
						}
						double delPrice = itB->open.priceAvg*(m_stopLoss-0.0025);
						if(m_curTickData.last < checkPrice && (checkPrice-m_curTickData.last >= delPrice) && tNow-itB->open.timeStamp>m_nKlineCycle*5 && itB->stopLoss.orderID == "" && itB->stopLoss.strClientOrderID == "")
						{
							std::string price = "-1";
							if(m_bTest)
							{
								itB->stopLoss.strClientOrderID = CFuncCommon::GenUUID();
								itB->stopLoss.timeStamp = time(NULL);
								itB->stopLoss.filledQTY = itB->open.filledQTY;
								itB->stopLoss.orderID = CFuncCommon::GenUUID();
								itB->stopLoss.price = m_curTickData.buy;
								itB->stopLoss.priceAvg = m_curTickData.buy;
								itB->stopLoss.status = 2;
								itB->stopLoss.size = itB->open.filledQTY;
								itB->stopLoss.tradeType = eFuturesTradeType_OpenBear;
								CActionLog("trade", "[%s]止损开空 order=%s, price=%s, filledQTY=%s, client_oid=%s", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), itB->open.orderID.c_str(), price.c_str(), itB->open.filledQTY.c_str(), itB->stopLoss.strClientOrderID.c_str());
							}
							else
							{
								std::string strClientOrderID = CFuncCommon::GenUUID();
								OKEX_HTTP->API_FuturesTrade(m_bSwapFutures, eFuturesTradeType_OpenBear, m_strCoinType, m_strFuturesCycle, price, itB->open.filledQTY, m_strLeverage, strClientOrderID);
								itB->stopLoss.strClientOrderID = strClientOrderID;
								itB->stopLoss.waitClientOrderIDTime = time(NULL);
								itB->stopLoss.tradeType = eFuturesTradeType_OpenBear;
								CActionLog("trade", "止损开空 order=%s, price=%s, filledQTY=%s, client_oid=%s", itB->open.orderID.c_str(), price.c_str(), itB->open.filledQTY.c_str(), strClientOrderID.c_str());
							}
						}
						//盈利达到2倍移动平均线后开始设置止盈线, 回撤破止盈线就平仓
						if(m_curTickData.last > itB->open.priceAvg)
						{
							if(itB->open.stopProfit)
							{
								//平仓
								bool _bCanStopProfit = __CheckCanStopProfit(eFuturesTradeType_OpenBull, itB->open.orderID);
								if(!_bCanStopProfit)
									bStopProfitFail = true;
								double half = m_moveStopProfit/2;
								if((m_curTickData.last <= (itB->open.priceAvg*(1+itB->open.stopProfit*m_moveStopProfit+half)) && _bCanStopProfit))
								{
									bool bInStopLoss = (itB->stopLoss.orderID != "");
									//如果open未交易完,先撤单
									if(itB->open.status == "1")
									{
										if(m_bTest)
										{
											itB->open.status = "-1";
											CActionLog("trade", "[%s]撤消订单成功 order=%s", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), itB->open.orderID.c_str());
										}
										else
											OKEX_HTTP->API_FuturesCancelOrder(m_bSwapFutures, m_strCoinType, m_strFuturesCycle, itB->open.orderID);
									}
									if(itB->open.filledQTY != "0")
									{
										double checkPrice = itB->open.priceAvg*(1+itB->open.stopProfit*m_moveStopProfit);
										if(m_curTickData.sell < checkPrice)
											checkPrice = m_curTickData.sell;
										std::string price = CFuncCommon::Double2String(checkPrice + DOUBLE_PRECISION, m_nPriceDecimal);
										if(itB->open.stopProfit >= 6 || bInStopLoss)
											price = "-1";
										if(m_bTest)
										{
											if(!bInStopLoss)
											{
												itB->close.strClientOrderID = CFuncCommon::GenUUID();
												itB->close.timeStamp = time(NULL);
												itB->close.filledQTY = itB->open.filledQTY;
												itB->close.orderID = CFuncCommon::GenUUID();
												itB->close.price = checkPrice;
												itB->close.priceAvg = checkPrice;
												itB->close.status = 2;
												itB->close.tradeType = eFuturesTradeType_CloseBull;
												itB->close.size = itB->open.filledQTY;
												CActionLog("trade", "[%s]止盈平多 openClientOrder=%s, order=%s, price=%s, filledQTY=%s, client_oid=%s", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), itB->open.strClientOrderID.c_str(), itB->open.orderID.c_str(), price.c_str(), itB->open.filledQTY.c_str(), itB->close.strClientOrderID.c_str());
											}
											else
											{
												CActionLog("trade", "[%s]止盈平多止损转开单 openClientOrder=%s, order=%s, price=%s, filledQTY=%s", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), itB->open.strClientOrderID.c_str(), itB->open.orderID.c_str(), price.c_str(), itB->open.filledQTY.c_str());
												itB->open = itB->stopLoss;
												itB->open.stopProfit = 0;
												itB->open.lastSellStopLossPrice = m_curTickData.last - itB->stopLoss.priceAvg*(m_stopLoss / 2 + 0.0025);
												itB->stopLoss.Reset();
												bUpdate = true;
											}
											
										}
										else
										{
											
											std::string strClientOrderID = CFuncCommon::GenUUID();
											OKEX_HTTP->API_FuturesTrade(m_bSwapFutures, eFuturesTradeType_CloseBull, m_strCoinType, m_strFuturesCycle, price, itB->open.filledQTY, m_strLeverage, strClientOrderID);
											if(!bInStopLoss)
											{
												itB->close.strClientOrderID = strClientOrderID;
												itB->close.waitClientOrderIDTime = time(NULL);
												itB->close.tradeType = eFuturesTradeType_CloseBull;
												CActionLog("trade", "止盈平多 openClientOrder=%s, order=%s, price=%s, filledQTY=%s, client_oid=%s", itB->open.strClientOrderID.c_str(), itB->open.orderID.c_str(), price.c_str(), itB->open.filledQTY.c_str(), strClientOrderID.c_str());
											}
											else
											{
												CActionLog("trade", "止盈平多止损转开单 openClientOrder=%s, order=%s, price=%s, filledQTY=%s", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), itB->open.strClientOrderID.c_str(), itB->open.orderID.c_str(), price.c_str(), itB->open.filledQTY.c_str());
												itB->open = itB->stopLoss;
												itB->open.stopProfit = 0;
												itB->open.lastSellStopLossPrice = m_curTickData.last - itB->stopLoss.priceAvg*(m_stopLoss / 2 + 0.0025);
												itB->stopLoss.Reset();
												bUpdate = true;
											}
										}
									}
								}
								else
								{
									double up = (m_curTickData.last - itB->open.priceAvg)/itB->open.priceAvg;
									int nowStep = int(up/m_moveStopProfit);
									if(nowStep - itB->open.stopProfit >= 2)
									{
										itB->open.stopProfit = nowStep-1;
										bUpdate = true;
									}
								}
							}
							else
							{
								double up = (m_curTickData.last - itB->open.priceAvg)/itB->open.priceAvg;
								int nowStep = int(up/m_moveStopProfit);
								if(nowStep >= 2)
								{
									itB->open.stopProfit = nowStep-1;
									bUpdate = true;
								}
							}
						}
					}
				}
				else if(itB->open.tradeType == eFuturesTradeType_OpenBear)
				{
					if(m_curTickData.bValid)
					{
						//超过止损线 平仓
						double checkPrice = itB->open.priceAvg;
						if(!CFuncCommon::CheckEqual(itB->open.lastSellStopLossPrice, 0.0))
						{
							if(((m_curTickData.last-itB->open.priceAvg) / itB->open.priceAvg) <= m_stopLoss/2)
							{
								itB->open.lastSellStopLossPrice = 0.0;
								bUpdate = true;
							}
							else
								checkPrice = itB->open.lastSellStopLossPrice;
						}
						double delPrice = itB->open.priceAvg*(m_stopLoss-0.0025);
						if(m_curTickData.last > checkPrice && (m_curTickData.last-checkPrice >= delPrice) && tNow-itB->open.timeStamp>m_nKlineCycle*5 && itB->stopLoss.orderID == "" && itB->stopLoss.strClientOrderID == "")
						{
							std::string price = "-1";
							if(m_bTest)
							{
								itB->stopLoss.strClientOrderID = CFuncCommon::GenUUID();
								itB->stopLoss.timeStamp = time(NULL);
								itB->stopLoss.filledQTY = itB->open.filledQTY;
								itB->stopLoss.orderID = CFuncCommon::GenUUID();
								itB->stopLoss.price = m_curTickData.sell;
								itB->stopLoss.priceAvg = m_curTickData.sell;
								itB->stopLoss.status = 2;
								itB->stopLoss.tradeType = eFuturesTradeType_OpenBull;
								itB->stopLoss.size = itB->open.filledQTY;
								CActionLog("trade", "[%s]止损开多 openClientOrder=%s, order=%s, price=%s, filledQTY=%s, client_oid=%s", itB->open.strClientOrderID.c_str(), CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), itB->open.orderID.c_str(), price.c_str(), itB->open.filledQTY.c_str(), itB->stopLoss.strClientOrderID.c_str());
							}
							else
							{
								std::string strClientOrderID = CFuncCommon::GenUUID();
								OKEX_HTTP->API_FuturesTrade(m_bSwapFutures, eFuturesTradeType_OpenBull, m_strCoinType, m_strFuturesCycle, price, itB->open.filledQTY, m_strLeverage, strClientOrderID);
								itB->stopLoss.strClientOrderID = strClientOrderID;
								itB->stopLoss.waitClientOrderIDTime = time(NULL);
								itB->stopLoss.tradeType = eFuturesTradeType_OpenBull;
								CActionLog("trade", "止损开多 openClientOrder=%s, order=%s, price=%s, filledQTY=%s, client_oid=%s", itB->open.strClientOrderID.c_str(), itB->open.orderID.c_str(), price.c_str(), itB->open.filledQTY.c_str(), strClientOrderID.c_str());
							}
						}
						//盈利达到2倍移动平均线后开始移动止赢
						if(m_curTickData.last < itB->open.priceAvg)
						{
							if(itB->open.stopProfit)
							{
								bool _bCanStopProfit = __CheckCanStopProfit(eFuturesTradeType_OpenBear, itB->open.orderID);
								if(!_bCanStopProfit)
									bStopProfitFail = true;
								double half = m_moveStopProfit/2;
								//平仓
								if((m_curTickData.last >= (itB->open.priceAvg*(1 - itB->open.stopProfit*m_moveStopProfit - half)) && _bCanStopProfit))
								{
									bool bInStopLoss = (itB->stopLoss.orderID != "");
									//如果open未交易完,先撤单
									if(itB->open.status == "1")
									{
										if(m_bTest)
										{
											itB->open.status = "-1";
											CActionLog("trade", "[%s]撤消订单成功 order=%s", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), itB->open.orderID.c_str());
										}
										else
											OKEX_HTTP->API_FuturesCancelOrder(m_bSwapFutures, m_strCoinType, m_strFuturesCycle, itB->open.orderID);
									}
									if(itB->open.filledQTY != "0")
									{
										double checkPrice = itB->open.priceAvg*(1 - itB->open.stopProfit*m_moveStopProfit);
										if(m_curTickData.buy > checkPrice)
											checkPrice = m_curTickData.buy;
										std::string price = CFuncCommon::Double2String(checkPrice + DOUBLE_PRECISION, m_nPriceDecimal);
										if(itB->open.stopProfit >= 6 || bInStopLoss)
											price = "-1";
										if(m_bTest)
										{
											if(!bInStopLoss)
											{
												itB->close.strClientOrderID = CFuncCommon::GenUUID();
												itB->close.timeStamp = time(NULL);
												itB->close.filledQTY = itB->open.filledQTY;
												itB->close.orderID = CFuncCommon::GenUUID();
												itB->close.price = checkPrice;
												itB->close.priceAvg = checkPrice;
												itB->close.status = 2;
												itB->close.tradeType = eFuturesTradeType_CloseBear;
												itB->close.size = itB->open.filledQTY;
												CActionLog("trade", "[%s]止盈平空 order=%s, price=%s, filledQTY=%s, client_oid=%s", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), itB->open.orderID.c_str(), price.c_str(), itB->open.filledQTY.c_str(), itB->close.strClientOrderID.c_str());
											}
											else
											{
												CActionLog("trade", "[%s]止盈平空损转开单 order=%s, price=%s, filledQTY=%s", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), itB->open.orderID.c_str(), price.c_str(), itB->open.filledQTY.c_str());
												itB->open = itB->stopLoss;
												itB->open.stopProfit = 0;
												itB->open.lastSellStopLossPrice = m_curTickData.last + itB->stopLoss.priceAvg*(m_stopLoss / 2 + 0.0025);
												itB->stopLoss.Reset();
												bUpdate = true;
											}
										}
										else
										{
											if(!bInStopLoss)
											{
												std::string strClientOrderID = CFuncCommon::GenUUID();
												OKEX_HTTP->API_FuturesTrade(m_bSwapFutures, eFuturesTradeType_CloseBear, m_strCoinType, m_strFuturesCycle, price, itB->open.filledQTY, m_strLeverage, strClientOrderID);
												itB->close.strClientOrderID = strClientOrderID;
												itB->close.waitClientOrderIDTime = time(NULL);
												itB->close.tradeType = eFuturesTradeType_CloseBear;
												CActionLog("trade", "止盈平空 order=%s, price=%s, filledQTY=%s, client_oid=%s", itB->open.orderID.c_str(), price.c_str(), itB->open.filledQTY.c_str(), strClientOrderID.c_str());
											}
											else
											{
												CActionLog("trade", "止盈平空损转开单 order=%s, price=%s, filledQTY=%s", itB->open.orderID.c_str(), price.c_str(), itB->open.filledQTY.c_str());
												itB->open = itB->stopLoss;
												itB->open.stopProfit = 0;
												itB->open.lastSellStopLossPrice = m_curTickData.last + itB->stopLoss.priceAvg*(m_stopLoss/2+0.0025);
												itB->stopLoss.Reset();
												bUpdate = true;
											}
										}
									}
								}
								else
								{
									double up = (itB->open.priceAvg - m_curTickData.last) / itB->open.priceAvg;
									int nowStep = int(up / m_moveStopProfit);
									if(nowStep - itB->open.stopProfit >= 2)
									{
										itB->open.stopProfit = nowStep - 1;
										bUpdate = true;
									}
								}
							}
							else
							{
								double up = (itB->open.priceAvg - m_curTickData.last) / itB->open.priceAvg;
								int nowStep = int(up/m_moveStopProfit);
								if(nowStep >= 2)
								{
									itB->open.stopProfit = nowStep-1;
									bUpdate = true;
								}
							}
						}
					}
				}
			}
		}
		++itB;
	}
	if(bUpdate)
		_SaveData();
	if(bStopProfitFail)
		m_bCanStopProfit = false;
}

void COKExFuturesDlg::OnLoginSuccess()
{
	OKEX_WEB_SOCKET->API_FuturesOrderInfo(true, m_bSwapFutures, m_strCoinType, m_strFuturesCycle);
	m_listTradePairInfo.clear();
	std::string strFilePath = "./save.txt";
	std::ifstream stream(strFilePath);
	if(!stream.is_open())
		return;
	char lineBuffer[4096] = {0};
	if(stream.fail())
		return;
	while(!stream.eof())
	{
		stream.getline(lineBuffer, sizeof(lineBuffer));
		if(*lineBuffer == 0 || (lineBuffer[0] == '/' && lineBuffer[1] == '/') || (lineBuffer[0] == '-' && lineBuffer[1] == '-'))
			continue;
		std::stringstream lineStream(lineBuffer, std::ios_base::in);
		char szOpenClientID[128] = {0};
		char szOpenOrderID[128] = {0};
		char szCloseClientID[128] = {0};
		char szCloseOrderID[128] = {0};
		char szStopLossClientID[128] = { 0 };
		char szStopLossOrderID[128] = { 0 };
		double openMin = 0.0;
		double openMax = 0.0;
		double closeMin = 0.0;
		double closeMax = 0.0;
		double stopLossMin = 0.0;
		double stopLossMax = 0.0;
		double lastSellStopLossPrice = 0.0;
		int triggerType = 0;
		int openStopProfit = 0;
		int stopLossStopProfit = 0;
		lineStream >> szOpenOrderID >> szOpenClientID >> triggerType >> lastSellStopLossPrice >> openMin >> openMax >> openStopProfit >> szCloseOrderID >> szCloseClientID >> 
					closeMin >> closeMax >> szStopLossOrderID >> szStopLossClientID >> stopLossMin >> stopLossMax >> stopLossStopProfit;
		SFuturesTradePairInfo info;
		if(strcmp(szOpenOrderID, "0") != 0)
		{
			info.open.strClientOrderID = szOpenClientID;
			info.open.orderID = szOpenOrderID;
			info.open.lastSellStopLossPrice = lastSellStopLossPrice;
			info.open.triggerType = triggerType;
			SHttpResponse resInfo;
			OKEX_HTTP->API_FuturesOrderInfo(false, m_bSwapFutures, m_strCoinType, m_strFuturesCycle, info.open.orderID, &resInfo);
			Json::Value& retObj = resInfo.retObj;
			if(retObj.isObject() && retObj["order_id"].isString())
			{
				info.open.timeStamp = CFuncCommon::ISO8601ToTime(retObj["timestamp"].asString());
				info.open.filledQTY = CFuncCommon::ToString(stoi(retObj["filled_qty"].asString()));
				info.open.price = stod(retObj["price"].asString());
				info.open.priceAvg = stod(retObj["price_avg"].asString());
				info.open.status = retObj["status"].asString();
				info.open.size = CFuncCommon::ToString(stoi(retObj["size"].asString()));
				std::string tradeType = retObj["type"].asString();
				if(tradeType == "1")
					info.open.tradeType = eFuturesTradeType_OpenBull;
				else if(tradeType == "2")
					info.open.tradeType = eFuturesTradeType_OpenBear;
				else if(tradeType == "3")
					info.open.tradeType = eFuturesTradeType_CloseBull;
				else if(tradeType == "4")
					info.open.tradeType = eFuturesTradeType_CloseBear;
				if(info.open.status != "0")
				{
					info.open.maxPrice = openMax;
					info.open.minPrice = openMin;
					if(CFuncCommon::CheckEqual(info.open.maxPrice, 0.0))
						info.open.maxPrice = info.open.priceAvg;
					if(CFuncCommon::CheckEqual(info.open.minPrice, 0.0))
						info.open.minPrice = info.open.priceAvg;
					info.open.stopProfit = openStopProfit;
				}
			}
		}
		if(strcmp(szCloseOrderID, "0") != 0)
		{
			info.close.strClientOrderID = szCloseClientID;
			info.close.orderID = szCloseOrderID;
			SHttpResponse resInfo;
			OKEX_HTTP->API_FuturesOrderInfo(false, m_bSwapFutures, m_strCoinType, m_strFuturesCycle, info.close.orderID, &resInfo);
			Json::Value& retObj = resInfo.retObj;
			if(retObj.isObject() && retObj["order_id"].isString())
			{
				info.close.timeStamp = CFuncCommon::ISO8601ToTime(retObj["timestamp"].asString());
				info.close.filledQTY = CFuncCommon::ToString(stoi(retObj["filled_qty"].asString()));
				info.close.price = stod(retObj["price"].asString());
				info.close.priceAvg = stod(retObj["price_avg"].asString());
				info.close.status = retObj["status"].asString();
				info.close.size = CFuncCommon::ToString(stoi(retObj["size"].asString()));
				std::string tradeType = retObj["type"].asString();
				if(tradeType == "1")
					info.close.tradeType = eFuturesTradeType_OpenBull;
				else if(tradeType == "2")
					info.close.tradeType = eFuturesTradeType_OpenBear;
				else if(tradeType == "3")
					info.close.tradeType = eFuturesTradeType_CloseBull;
				else if(tradeType == "4")
					info.close.tradeType = eFuturesTradeType_CloseBear;
				if(info.close.status != "0")
				{
					info.close.maxPrice = closeMax;
					info.close.minPrice = closeMin;
					if(CFuncCommon::CheckEqual(info.close.maxPrice, 0.0))
						info.close.maxPrice = info.close.priceAvg;
					if(CFuncCommon::CheckEqual(info.close.minPrice, 0.0))
						info.close.minPrice = info.close.priceAvg;
				}
			}
		}
		if(strcmp(szStopLossOrderID, "0") != 0)
		{
			info.stopLoss.strClientOrderID = szStopLossClientID;
			info.stopLoss.orderID = szStopLossOrderID;
			SHttpResponse resInfo;
			OKEX_HTTP->API_FuturesOrderInfo(false, m_bSwapFutures, m_strCoinType, m_strFuturesCycle, info.stopLoss.orderID, &resInfo);
			Json::Value& retObj = resInfo.retObj;
			if(retObj.isObject() && retObj["order_id"].isString())
			{
				info.stopLoss.timeStamp = CFuncCommon::ISO8601ToTime(retObj["timestamp"].asString());
				info.stopLoss.filledQTY = CFuncCommon::ToString(stoi(retObj["filled_qty"].asString()));
				info.stopLoss.price = stod(retObj["price"].asString());
				info.stopLoss.priceAvg = stod(retObj["price_avg"].asString());
				info.stopLoss.status = retObj["status"].asString();
				info.stopLoss.size = CFuncCommon::ToString(stoi(retObj["size"].asString()));
				std::string tradeType = retObj["type"].asString();
				if(tradeType == "1")
					info.stopLoss.tradeType = eFuturesTradeType_OpenBull;
				else if(tradeType == "2")
					info.stopLoss.tradeType = eFuturesTradeType_OpenBear;
				else if(tradeType == "3")
					info.stopLoss.tradeType = eFuturesTradeType_CloseBull;
				else if(tradeType == "4")
					info.stopLoss.tradeType = eFuturesTradeType_CloseBear;
				if(info.stopLoss.status != "0")
				{
					info.stopLoss.maxPrice = stopLossMax;
					info.stopLoss.minPrice = stopLossMin;
					if(CFuncCommon::CheckEqual(info.stopLoss.maxPrice, 0.0))
						info.stopLoss.maxPrice = info.stopLoss.priceAvg;
					if(CFuncCommon::CheckEqual(info.stopLoss.minPrice, 0.0))
						info.stopLoss.minPrice = info.stopLoss.priceAvg;
					info.stopLoss.stopProfit = stopLossStopProfit;
				}
			}
		}
		if(strcmp(szOpenOrderID, "0") != 0)
		{
			m_listTradePairInfo.push_back(info);
			m_bCanLogCheckCanTrade = true;
			m_bCanStopProfit = true;
		}
	}
	stream.close();
	_UpdateTradeShow();
	//OKEX_WEB_SOCKET->API_FuturesAccountInfoByCurrency(true, m_bSwapFutures, m_strCoinType);
}

bool COKExFuturesDlg::__CheckCanTrade(eFuturesTradeType eType)
{
	switch(eType)
	{
	case eFuturesTradeType_OpenBull:
		{
			if((int)m_listTradePairInfo.size() >= m_nMaxTradeCnt)
				return false;
			std::list<SFuturesTradePairInfo>::reverse_iterator itBegin = m_listTradePairInfo.rbegin();
			std::list<SFuturesTradePairInfo>::reverse_iterator itEnd = m_listTradePairInfo.rend();
			int cnt = 0;
			while(itBegin != itEnd)
			{
				if(itBegin->open.orderID != "" && itBegin->open.tradeType == eFuturesTradeType_OpenBull && itBegin->stopLoss.orderID == "")
					cnt++;
				itBegin++;
			}
			if(cnt >= m_nMaxDirTradeCnt)
			{
				if(m_bCanLogCheckCanTrade)
				{
					m_bCanLogCheckCanTrade = false;
					CActionLog("trade", "多单超上限,检测开单失败");
				}
				return false;
			}
		}
		break;
	case eFuturesTradeType_OpenBear:
		{
			if((int)m_listTradePairInfo.size() >= m_nMaxTradeCnt)
				return false;
			std::list<SFuturesTradePairInfo>::reverse_iterator itBegin = m_listTradePairInfo.rbegin();
			std::list<SFuturesTradePairInfo>::reverse_iterator itEnd = m_listTradePairInfo.rend();
			int cnt = 0;
			while(itBegin != itEnd)
			{
				if(itBegin->open.orderID != "" && itBegin->open.tradeType == eFuturesTradeType_OpenBear && itBegin->stopLoss.orderID == "")
					cnt++;
				itBegin++;
			}
			if(cnt >= m_nMaxDirTradeCnt)
			{
				if(m_bCanLogCheckCanTrade)
				{
					m_bCanLogCheckCanTrade = false;
					CActionLog("trade", "空单超上限,检测开单失败");
				}
				return false;
			}
		}
		break;
	default:
		break;
	}
	return true;
}

void COKExFuturesDlg::OnTradeFail(std::string& clientOrderID)
{
	std::list<SFuturesTradePairInfo>::iterator itB = m_listTradePairInfo.begin();
	std::list<SFuturesTradePairInfo>::iterator itE = m_listTradePairInfo.end();
	while(itB != itE)
	{
		if(itB->open.strClientOrderID == clientOrderID && itB->open.waitClientOrderIDTime)
		{
			m_listTradePairInfo.erase(itB);
			m_bCanLogCheckCanTrade = true;
			m_bCanStopProfit = true;
			return;
		}
		else if(itB->close.strClientOrderID == clientOrderID && itB->close.waitClientOrderIDTime)
		{
			itB->close.Reset();
			return;
		}
		else if(itB->stopLoss.strClientOrderID == clientOrderID && itB->stopLoss.waitClientOrderIDTime)
		{
			itB->stopLoss.Reset();
			return;
		}
		++itB;
	}
}

void COKExFuturesDlg::OnTradeSuccess(std::string& clientOrderID, std::string& serverOrderID)
{
	std::list<SFuturesTradePairInfo>::iterator itB = m_listTradePairInfo.begin();
	std::list<SFuturesTradePairInfo>::iterator itE = m_listTradePairInfo.end();
	bool bUpdate = false;
	while(itB != itE)
	{
		if(itB->open.strClientOrderID == clientOrderID)
		{
			itB->open.orderID = serverOrderID;
			itB->open.waitClientOrderIDTime = 0;
			CActionLog("trade", "http下单成功 client_order=%s, order=%s", itB->open.strClientOrderID.c_str(), itB->open.orderID.c_str());
			OKEX_HTTP->API_FuturesOrderInfo(true, m_bSwapFutures, m_strCoinType, m_strFuturesCycle, serverOrderID);
			bUpdate = true;
			if(m_eBollState == eBollTrend_ZhangKou)
				m_lastZhangKouOrder = serverOrderID;
			break;
		}
		if(itB->close.strClientOrderID == clientOrderID)
		{
			itB->close.orderID = serverOrderID;
			itB->close.waitClientOrderIDTime = 0;
			CActionLog("trade", "http下单成功 client_order=%s, order=%s", itB->close.strClientOrderID.c_str(), itB->close.orderID.c_str());
			OKEX_HTTP->API_FuturesOrderInfo(true, m_bSwapFutures, m_strCoinType, m_strFuturesCycle, serverOrderID);
			bUpdate = true;
			break;
		}
		if(itB->stopLoss.strClientOrderID == clientOrderID)
		{
			itB->stopLoss.orderID = serverOrderID;
			itB->stopLoss.waitClientOrderIDTime = 0;
			CActionLog("trade", "http下单成功 client_order=%s, order=%s", itB->stopLoss.strClientOrderID.c_str(), itB->stopLoss.orderID.c_str());
			OKEX_HTTP->API_FuturesOrderInfo(true, m_bSwapFutures, m_strCoinType, m_strFuturesCycle, serverOrderID);
			bUpdate = true;
			break;
		}
		++itB;
	}
	if(bUpdate)
		_SaveData();
}

void COKExFuturesDlg::_SaveData()
{
	std::string strFilePath = "./save.txt";
	std::ofstream stream(strFilePath);
	if(!stream.is_open())
		return;
	std::list<SFuturesTradePairInfo>::iterator itB = m_listTradePairInfo.begin();
	std::list<SFuturesTradePairInfo>::iterator itE = m_listTradePairInfo.end();
	while(itB != itE)
	{
		if(itB->open.orderID != "")
			stream << itB->open.orderID << "	" << itB->open.strClientOrderID	<< "	" << itB->open.triggerType << "	" << itB->open.lastSellStopLossPrice << "	" << itB->open.minPrice << "	" << itB->open.maxPrice << "	" << itB->open.stopProfit;
		if(itB->close.orderID != "")
			stream  << "	" << itB->close.orderID << "	" << itB->close.strClientOrderID << "	"	<<	itB->close.minPrice << "	" << itB->close.maxPrice;
		else
			stream << "	0	0	0	0";
		if(itB->stopLoss.orderID != "")
			stream  << "	" << itB->stopLoss.orderID << "	" << itB->stopLoss.strClientOrderID << "	" << itB->stopLoss.minPrice << "	" << itB->stopLoss.maxPrice << "	" << itB->stopLoss.stopProfit;
		else
			stream << "	0	0	0	0	0";
		stream << std::endl;
		++itB;
	}
	stream.close();
}

void COKExFuturesDlg::UpdateTradeInfo(SFuturesTradeInfo& info)
{
	std::list<SFuturesTradePairInfo>::iterator itB = m_listTradePairInfo.begin();
	std::list<SFuturesTradePairInfo>::iterator itE = m_listTradePairInfo.end();
	while(itB != itE)
	{
		if(info.tradeType == itB->open.tradeType && itB->open.orderID == info.orderID)
		{
			if(itB->open.status == "" && (info.status == "0" || info.status == "1" || info.status == "2") && m_curTickData.bValid)
			{
				itB->open.minPrice = m_curTickData.last;
				itB->open.maxPrice = m_curTickData.last;
			}
			itB->open.timeStamp = info.timeStamp;
			itB->open.filledQTY = info.filledQTY;
			itB->open.price = info.price;
			itB->open.priceAvg = info.priceAvg;
			itB->open.status = info.status;
			itB->open.tradeType = info.tradeType;
			itB->open.size = info.size;
			itB->open.tLastUpdate = time(NULL);
			info.strClientOrderID = itB->open.strClientOrderID;
			break;
		}
		else if(info.tradeType == itB->close.tradeType && itB->close.orderID == info.orderID)
		{
			itB->close.timeStamp = info.timeStamp;
			itB->close.filledQTY = info.filledQTY;
			itB->close.price = info.price;
			itB->close.priceAvg = info.priceAvg;
			itB->close.status = info.status;
			itB->close.tradeType = info.tradeType;
			itB->close.size = info.size;
			itB->close.tLastUpdate = time(NULL);
			info.strClientOrderID = itB->close.strClientOrderID;
			break;
		}
		else if(info.tradeType == itB->stopLoss.tradeType && itB->stopLoss.orderID == info.orderID)
		{
			if(itB->stopLoss.status == "" && (info.status == "0" || info.status == "1" || info.status == "2") && m_curTickData.bValid)
			{
				itB->stopLoss.minPrice = m_curTickData.last;
				itB->stopLoss.maxPrice = m_curTickData.last;
			}
			itB->stopLoss.timeStamp = info.timeStamp;
			itB->stopLoss.filledQTY = info.filledQTY;
			itB->stopLoss.price = info.price;
			itB->stopLoss.priceAvg = info.priceAvg;
			itB->stopLoss.status = info.status;
			itB->stopLoss.tradeType = info.tradeType;
			itB->stopLoss.size = info.size;
			itB->stopLoss.tLastUpdate = time(NULL);
			info.strClientOrderID = itB->stopLoss.strClientOrderID;
			break;
		}
		++itB;
	}
	_UpdateTradeShow();
}

void COKExFuturesDlg::_UpdateTradeShow()
{
	m_listCtrlOrderOpen.DeleteAllItems();
	m_listCtrlOrderClose.DeleteAllItems();
	CString szFormat;
	std::list<SFuturesTradePairInfo>::iterator itB = m_listTradePairInfo.begin();
	std::list<SFuturesTradePairInfo>::iterator itE = m_listTradePairInfo.end();
	int i=0;
	double totalProfit = 0.0;
	while(itB != itE)
	{
		if(itB->open.orderID != "")
		{
			m_listCtrlOrderOpen.InsertItem(i, "");
			m_listCtrlOrderClose.InsertItem(i, "");
			if(CFuncCommon::CheckEqual(itB->open.priceAvg, 0.0))
				szFormat.Format("%s[%d-%d]", CFuncCommon::Double2String(itB->open.price+DOUBLE_PRECISION, m_nPriceDecimal).c_str(), itB->open.triggerType, itB->open.stopProfit);
			else
				szFormat.Format("%s[%d-%d]", CFuncCommon::Double2String(itB->open.priceAvg+DOUBLE_PRECISION, m_nPriceDecimal).c_str(), itB->open.triggerType, itB->open.stopProfit);
			m_listCtrlOrderOpen.SetItemText(i, 0, szFormat);
			switch(itB->open.tradeType)
			{
			case eFuturesTradeType_OpenBull:
				m_listCtrlOrderOpen.SetItemText(i, 1, "开多");
				break;
			case eFuturesTradeType_OpenBear:
				m_listCtrlOrderOpen.SetItemText(i, 1, "开空");
				break;
			case eFuturesTradeType_CloseBull:
				m_listCtrlOrderOpen.SetItemText(i, 1, "平多");
				break;
			case eFuturesTradeType_CloseBear:
				m_listCtrlOrderOpen.SetItemText(i, 1, "平空");
				break;
			default:
				break;
			}
			szFormat.Format("%s/%s", itB->open.filledQTY.c_str(), itB->open.size.c_str());
			m_listCtrlOrderOpen.SetItemText(i, 2, szFormat);
			if(itB->open.status == "-1")
				m_listCtrlOrderOpen.SetItemText(i, 3, "cancelled");
			else if(itB->open.status == "0")
				m_listCtrlOrderOpen.SetItemText(i, 3, "open");
			else if(itB->open.status == "1")
				m_listCtrlOrderOpen.SetItemText(i, 3, "part_filled");
			else if(itB->open.status == "2")
				m_listCtrlOrderOpen.SetItemText(i, 3, "filled");
			szFormat.Format("%s", CFuncCommon::Double2String(itB->open.minPrice+DOUBLE_PRECISION, m_nPriceDecimal).c_str());
			m_listCtrlOrderOpen.SetItemText(i, 4, szFormat.GetBuffer());
			szFormat.Format("%s", CFuncCommon::Double2String(itB->open.maxPrice+DOUBLE_PRECISION, m_nPriceDecimal).c_str());
			m_listCtrlOrderOpen.SetItemText(i, 5, szFormat.GetBuffer());
			if(itB->open.filledQTY != "0")
			{
				int count = stoi(itB->open.filledQTY);
				int sizePrice = 10;
				if(m_strCoinType == "BTC")
					sizePrice = 100;
				double baozhengjin = (sizePrice*count/itB->open.priceAvg)/m_nLeverage;
				if(itB->open.tradeType == eFuturesTradeType_OpenBull)
				{
					double calcuPrice = m_curTickData.last;
					if(itB->close.orderID != "" && itB->close.status == "2")
						calcuPrice = itB->close.priceAvg;
					if(calcuPrice >= itB->open.priceAvg)
					{
						double profitPersent = (calcuPrice-itB->open.priceAvg)/itB->open.priceAvg;
						double profit = profitPersent*m_nLeverage*baozhengjin;
						totalProfit += profit;
						szFormat.Format("%s(%s%%)", CFuncCommon::Double2String(profit+DOUBLE_PRECISION, 5).c_str(), CFuncCommon::Double2String(profitPersent*100+DOUBLE_PRECISION, 2).c_str());
						m_listCtrlOrderOpen.SetItemText(i, 6, szFormat.GetBuffer());
					}
					else
					{
						double profitPersent = (itB->open.priceAvg-calcuPrice)/itB->open.priceAvg;
						double profit = profitPersent*m_nLeverage*baozhengjin;
						totalProfit -= profit;
						szFormat.Format("-%s(-%s%%)", CFuncCommon::Double2String(profit+DOUBLE_PRECISION, 5).c_str(), CFuncCommon::Double2String(profitPersent*100+DOUBLE_PRECISION, 2).c_str());
						m_listCtrlOrderOpen.SetItemText(i, 6, szFormat.GetBuffer());
					}

				}
				else if(itB->open.tradeType == eFuturesTradeType_OpenBear)
				{
					double calcuPrice = m_curTickData.last;
					if(itB->close.orderID != "" && itB->close.status == "2")
						calcuPrice = itB->close.priceAvg;
					if(calcuPrice <= itB->open.priceAvg)
					{
						double profitPersent = (itB->open.priceAvg-calcuPrice)/itB->open.priceAvg;
						double profit = profitPersent*m_nLeverage*baozhengjin;
						totalProfit += profit;
						szFormat.Format("%s(%s%%)", CFuncCommon::Double2String(profit+DOUBLE_PRECISION, 5).c_str(), CFuncCommon::Double2String(profitPersent*100+DOUBLE_PRECISION, 2).c_str());
						m_listCtrlOrderOpen.SetItemText(i, 6, szFormat.GetBuffer());
					}
					else
					{
						double profitPersent = (calcuPrice-itB->open.priceAvg)/itB->open.priceAvg;
						double profit = profitPersent*m_nLeverage*baozhengjin;
						totalProfit -= profit;
						szFormat.Format("-%s(-%s%%)", CFuncCommon::Double2String(profit+DOUBLE_PRECISION, 5).c_str(), CFuncCommon::Double2String(profitPersent*100+DOUBLE_PRECISION, 2).c_str());
						m_listCtrlOrderOpen.SetItemText(i, 6, szFormat.GetBuffer());
					}
				}
			}
			tm _tm;
			localtime_s(&_tm, ((const time_t*)&(itB->open.timeStamp)));
			szFormat.Format("%02d-%02d %02d:%02d:%02d", _tm.tm_mon+1, _tm.tm_mday, _tm.tm_hour, _tm.tm_min, _tm.tm_sec);
			m_listCtrlOrderOpen.SetItemText(i, 7, szFormat.GetBuffer());
		}
		if(itB->close.orderID != "")
		{
			if(CFuncCommon::CheckEqual(itB->close.priceAvg, 0.0))
				szFormat.Format("%s", CFuncCommon::Double2String(itB->close.price+DOUBLE_PRECISION, m_nPriceDecimal).c_str());
			else
				szFormat.Format("%s", CFuncCommon::Double2String(itB->close.priceAvg+DOUBLE_PRECISION, m_nPriceDecimal).c_str());
			m_listCtrlOrderClose.SetItemText(i, 0, szFormat);
			szFormat.Format("%s/%s", itB->close.filledQTY.c_str(), itB->close.size.c_str());
			m_listCtrlOrderClose.SetItemText(i, 1, szFormat);
			if(itB->close.status == "-1")
				m_listCtrlOrderClose.SetItemText(i, 2, "cancelled");
			else if(itB->close.status == "0")
				m_listCtrlOrderClose.SetItemText(i, 2, "open");
			else if(itB->close.status == "1")
				m_listCtrlOrderClose.SetItemText(i, 2, "part_filled");
			else if(itB->close.status == "2")
				m_listCtrlOrderClose.SetItemText(i, 2, "filled");
		}
		else if(itB->stopLoss.orderID != "")
		{
			if(CFuncCommon::CheckEqual(itB->stopLoss.priceAvg, 0.0))
				szFormat.Format("%s[止%d]", CFuncCommon::Double2String(itB->stopLoss.price + DOUBLE_PRECISION, m_nPriceDecimal).c_str(), itB->stopLoss.stopProfit);
			else
				szFormat.Format("%s[止%d]", CFuncCommon::Double2String(itB->stopLoss.priceAvg + DOUBLE_PRECISION, m_nPriceDecimal).c_str(), itB->stopLoss.stopProfit);
			m_listCtrlOrderClose.SetItemText(i, 0, szFormat);
			szFormat.Format("%s/%s", itB->stopLoss.filledQTY.c_str(), itB->stopLoss.size.c_str());
			m_listCtrlOrderClose.SetItemText(i, 1, szFormat);
			if(itB->stopLoss.status == "-1")
				m_listCtrlOrderClose.SetItemText(i, 2, "cancelled");
			else if(itB->stopLoss.status == "0")
				m_listCtrlOrderClose.SetItemText(i, 2, "open");
			else if(itB->stopLoss.status == "1")
				m_listCtrlOrderClose.SetItemText(i, 2, "part_filled");
			else if(itB->stopLoss.status == "2")
				m_listCtrlOrderClose.SetItemText(i, 2, "filled");
		}
		++itB;
		++i;
	}
	if(i != 0)
	{
		m_listCtrlOrderOpen.InsertItem(i, "");
		m_listCtrlOrderOpen.SetItemText(i, 0, "统计");
		szFormat.Format("%s", CFuncCommon::Double2String(totalProfit + DOUBLE_PRECISION, 5).c_str());
		m_listCtrlOrderOpen.SetItemText(i, 6, szFormat.GetBuffer());
	}
	
}


void COKExFuturesDlg::__InitConfigCtrl()
{
	std::string strTemp = m_config.get("futures", "coinType", "");
	if(strTemp == "BTC")
		m_combCoinType.SetCurSel(0);
	else if(strTemp == "LTC")
		m_combCoinType.SetCurSel(1);
	else if(strTemp == "ETH")
		m_combCoinType.SetCurSel(2);
	else if(strTemp == "ETC")
		m_combCoinType.SetCurSel(3);
	else if(strTemp == "BTG")
		m_combCoinType.SetCurSel(4);
	else if(strTemp == "XRP")
		m_combCoinType.SetCurSel(5);
	else if(strTemp == "EOS")
		m_combCoinType.SetCurSel(6);
	else if(strTemp == "BCH")
		m_combCoinType.SetCurSel(7);
	else if(strTemp == "TRX")
		m_combCoinType.SetCurSel(8);
	strTemp = m_config.get("futures", "futuresCycle", "");
	m_editFuturesCycle.SetWindowText(strTemp.c_str());

	strTemp = m_config.get("futures", "futuresTradeSize", "");
	m_editFuturesTradeSize.SetWindowText(strTemp.c_str());

	strTemp = m_config.get("futures", "leverage", "");
	if(strTemp == "10")
		m_combLeverage.SetCurSel(0);
	else if(strTemp == "20")
		m_combLeverage.SetCurSel(1);

	strTemp = m_config.get("futures", "futuresType", "");
	if(strTemp == "交割合约")
		m_combFuturesType.SetCurSel(0);
	else if(strTemp == "永续合约")
		m_combFuturesType.SetCurSel(1);

	strTemp = m_config.get("futures", "stopLoss", "");
	m_editStopLoss.SetWindowText(strTemp.c_str());

	strTemp = m_config.get("futures", "moveStopProfit", "");
	m_editMoveStopProfit.SetWindowText(strTemp.c_str());
	
	strTemp = m_config.get("futures", "maxTradeCnt", "");
	m_editMaxTradeCnt.SetWindowText(strTemp.c_str());
	
	strTemp = m_config.get("futures", "maxDirTradeCnt", "");
	m_editMaxDirTradeCnt.SetWindowText(strTemp.c_str());

	strTemp = m_config.get("futures", "beginMoney", "");
	m_editCapital.SetWindowText(strTemp.c_str());

	strTemp = m_config.get("futures", "todayBeginMoney", "");
	m_editCapitalToday.SetWindowText(strTemp.c_str());

	strTemp = m_config.get("futures", "tradeMoment", "");
	if(strTemp == "趋势出现正向")
		m_combTradeMoment.SetCurSel(0);
	else if(strTemp == "趋势出现反向")
		m_combTradeMoment.SetCurSel(1);
}

bool COKExFuturesDlg::__SaveConfigCtrl()
{
	CString strFuturesType;
	m_combFuturesType.GetWindowText(strFuturesType);
	if(strFuturesType == "")
	{
		MessageBox("未选择合约类型");
		return false;
	}

	CString strCoinType;
	m_combCoinType.GetWindowText(strCoinType);
	if(strCoinType == "")
	{
		MessageBox("未选择合约货币");
		return false;
	}
	CString strFuturesCycle;
	m_editFuturesCycle.GetWindowText(strFuturesCycle);
	if(strFuturesCycle == "")
	{
		MessageBox("未填写合约交割期");
		return false;
	}
	CString strFuturesTradeSize;
	m_editFuturesTradeSize.GetWindowText(strFuturesTradeSize);
	if(strFuturesTradeSize == "")
	{
		MessageBox("未填写下单张数");
		return false;
	}
	CString strLeverage;
	m_combLeverage.GetWindowText(strLeverage);
	if(strLeverage == "")
	{
		MessageBox("未选择杠杆");
		return false;
	}
	CString strStopLoss;
	m_editStopLoss.GetWindowText(strStopLoss);
	if(strStopLoss == "")
	{
		MessageBox("未填写止损点数");
		return false;
	}
	CString strStopProfit;
	m_editMoveStopProfit.GetWindowText(strStopProfit);
	if(strStopProfit == "")
	{
		MessageBox("未填写移动止盈点数");
		return false;
	}
	CString strMaxTradeCnt;
	m_editMaxTradeCnt.GetWindowText(strMaxTradeCnt);
	if(strMaxTradeCnt == "")
	{
		MessageBox("未填写最大同时下单数");
		return false;
	}
	CString strMaxDirTradeCnt;
	m_editMaxDirTradeCnt.GetWindowText(strMaxDirTradeCnt);
	if(strMaxDirTradeCnt == "")
	{
		MessageBox("未填写单向最大同时下单数");
		return false;
	}
	CString strTradeMoment;
	m_combTradeMoment.GetWindowText(strTradeMoment);
	if(strTradeMoment == "")
	{
		MessageBox("未选择交易时机");
		return false;
	}

	CString szCost = "";
	m_editCapital.GetWindowText(szCost);

	CString szTodayCost = "";
	m_editCapitalToday.GetWindowText(szTodayCost);

	m_strCoinType = strCoinType.GetBuffer();
	m_strFuturesCycle = strFuturesCycle.GetBuffer();
	m_strFuturesTradeSize = strFuturesTradeSize.GetBuffer();
	m_strLeverage = strLeverage.GetBuffer();
	m_nLeverage = stoi(m_strLeverage);
	m_stopLoss = stod(strStopLoss.GetBuffer());
	m_moveStopProfit = stod(strStopProfit.GetBuffer());
	m_nMaxTradeCnt = stoi(strMaxTradeCnt.GetBuffer());
	m_nMaxDirTradeCnt = stoi(strMaxDirTradeCnt.GetBuffer());
	if(strFuturesType == "永续合约")
		m_bSwapFutures = true;
	else
		m_bSwapFutures = false;
	if(strTradeMoment == "趋势出现正向")
		m_tradeMoment = 0;
	else
		m_tradeMoment = 1;
	if(szCost != "")
		m_beginMoney = stod(szCost.GetBuffer());
	if(szTodayCost != "")
		m_todayBeginMoney = stod(szTodayCost.GetBuffer());
	m_config.set_value("futures", "coinType", m_strCoinType.c_str());
	m_config.set_value("futures", "futuresCycle", m_strFuturesCycle.c_str());
	m_config.set_value("futures", "futuresTradeSize", m_strFuturesTradeSize.c_str());
	m_config.set_value("futures", "leverage", m_strLeverage.c_str());
	m_config.set_value("futures", "stopLoss", strStopLoss.GetBuffer());
	m_config.set_value("futures", "moveStopProfit", strStopProfit.GetBuffer());
	m_config.set_value("futures", "maxTradeCnt", strMaxTradeCnt.GetBuffer());
	m_config.set_value("futures", "maxDirTradeCnt", strMaxDirTradeCnt.GetBuffer());
	m_config.set_value("futures", "futuresType", strFuturesType.GetBuffer());
	m_config.set_value("futures", "beginMoney", szCost.GetBuffer());
	m_config.set_value("futures", "todayBeginMoney", szTodayCost.GetBuffer());
	m_config.set_value("futures", "tradeMoment", strTradeMoment.GetBuffer());
	m_config.save("./config.ini");
	return true;
}

void COKExFuturesDlg::SetHScroll()
{
	CDC* dc = GetDC();
	SIZE s;
	int index;
	CString str;
	long temp;
	for(index = 0; index< m_ctrlListLog.GetCount(); index++)
	{
		m_ctrlListLog.GetText(index, str);
		s = dc->GetTextExtent(str, str.GetLength() + 1);

		temp = (long)SendDlgItemMessage(IDC_LIST2, LB_GETHORIZONTALEXTENT, 0, 0);
		if(s.cx > temp)
			SendDlgItemMessage(IDC_LIST2, LB_SETHORIZONTALEXTENT, (WPARAM)s.cx, 0);
	}
	ReleaseDC(dc);
}

void COKExFuturesDlg::OnBnClickedButtonStopWhenFinish()
{
	m_bStopWhenFinish = true;
}

void COKExFuturesDlg::ComplementedKLine(time_t tNowKlineTick, int kLineCnt)
{
	time_t endTick = tNowKlineTick - m_nKlineCycle;
	time_t beginTick = endTick - (kLineCnt-1)*m_nKlineCycle;
	std::string strFrom = CFuncCommon::LocaltimeToISO8601(beginTick);
	std::string strTo = CFuncCommon::LocaltimeToISO8601(endTick);
	std::string strKlineCycle = CFuncCommon::ToString(m_nKlineCycle);
	SHttpResponse resInfo;
	OKEX_HTTP->API_GetFuturesSomeKline(false, m_bSwapFutures, m_strCoinType, m_strFuturesCycle, strKlineCycle, strFrom, strTo, &resInfo);
	Json::Value& retObj = resInfo.retObj;
	if(retObj.isArray())
	{
		for(int i = retObj.size()-1; i>=0; --i)
		{
			SKlineData data;
			if(m_bSwapFutures)
			{
				data.time = CFuncCommon::ISO8601ToTime(retObj[i][0].asString());
				data.openPrice = stod(retObj[i][1].asString());
				data.highPrice = stod(retObj[i][2].asString());
				data.lowPrice = stod(retObj[i][3].asString());
				data.closePrice = stod(retObj[i][4].asString());
				data.volume = stoi(retObj[i][5].asString());
				data.volumeByCurrency = stod(retObj[i][6].asString());
			}
			else
			{
				data.time = CFuncCommon::ISO8601ToTime(retObj[i][0].asString());
				data.openPrice = stod(retObj[i][1].asString());
				data.highPrice = stod(retObj[i][2].asString());
				data.lowPrice = stod(retObj[i][3].asString());
				data.closePrice = stod(retObj[i][4].asString());
				data.volume = stoi(retObj[i][5].asString());
				data.volumeByCurrency = stod(retObj[i][6].asString());
			}
			CString strlocalLog;
			if(m_bSwapFutures)
			{
				strlocalLog.Format("{\"table\":\"swap/%s\",\"data\":[{\"candle\":[\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%d\",\"%s\"],\"instrument_id\":\"%s-USD-SWAP\"}]}",
				m_strKlineCycle.c_str(), CFuncCommon::LocaltimeToISO8601(data.time).c_str(),
				CFuncCommon::Double2String(data.openPrice + DOUBLE_PRECISION, m_nPriceDecimal).c_str(),
				CFuncCommon::Double2String(data.highPrice + DOUBLE_PRECISION, m_nPriceDecimal).c_str(),
				CFuncCommon::Double2String(data.lowPrice + DOUBLE_PRECISION, m_nPriceDecimal).c_str(),
				CFuncCommon::Double2String(data.closePrice + DOUBLE_PRECISION, m_nPriceDecimal).c_str(),
				data.volume,
				CFuncCommon::Double2String(data.volumeByCurrency + DOUBLE_PRECISION, m_nPriceDecimal).c_str(),
				m_strCoinType.c_str());
			}
			else
			{
				strlocalLog.Format("{\"table\":\"futures/%s\",\"data\":[{\"candle\":[\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%d\",\"%s\"],\"instrument_id\":\"%s-USD-%s\"}]}", 
				m_strKlineCycle.c_str(), CFuncCommon::LocaltimeToISO8601(data.time).c_str(),
				CFuncCommon::Double2String(data.openPrice+DOUBLE_PRECISION, m_nPriceDecimal).c_str(),
				CFuncCommon::Double2String(data.highPrice+DOUBLE_PRECISION, m_nPriceDecimal).c_str(),
				CFuncCommon::Double2String(data.lowPrice+DOUBLE_PRECISION, m_nPriceDecimal).c_str(),
				CFuncCommon::Double2String(data.closePrice+DOUBLE_PRECISION, m_nPriceDecimal).c_str(),
				data.volume,
				CFuncCommon::Double2String(data.volumeByCurrency+DOUBLE_PRECISION, m_nPriceDecimal).c_str(),
				m_strCoinType.c_str(),
				m_strFuturesCycle.c_str());
			}
			CActionLog("market", "%s", strlocalLog.GetBuffer());
			g_pDlg->AddKlineData(data);
		}
	}
}

void COKExFuturesDlg::OnBnClickedButtonUpdateTradeSize()
{
	CString strFuturesTradeSize;
	m_editFuturesTradeSize.GetWindowText(strFuturesTradeSize);
	if(strFuturesTradeSize == "")
	{
		MessageBox("未填写下单张数");
		return;
	}
	m_strFuturesTradeSize = strFuturesTradeSize.GetBuffer();
	m_config.set_value("futures", "futuresTradeSize", m_strFuturesTradeSize.c_str());
	m_config.save("./config.ini");
}

void COKExFuturesDlg::UpdateAccountInfo(SFuturesAccountInfo& info)
{
	m_accountInfo = info;
	m_accountInfo.bValid = true;
	_UpdateAccountShow();
}

void COKExFuturesDlg::_UpdateAccountShow()
{
	if(m_accountInfo.bValid)
	{
		m_staticAccountInfo.SetWindowText(m_accountInfo.equity.c_str());
		time_t tNow = time(NULL);
		tm* pTM = localtime(&tNow);
		if(pTM->tm_yday != m_nLastUpdateDay)
		{
			double equity = stod(m_accountInfo.equity);
			if(m_nLastUpdateDay != -1)
				CFixActionLog("profit", "今日收益%.2f%%", (equity - m_todayBeginMoney) / m_todayBeginMoney * 100);
			
			m_todayBeginMoney = equity;
			m_nLastUpdateDay = pTM->tm_yday;
			m_editCapitalToday.SetWindowText(m_accountInfo.equity.c_str());
			m_config.set_value("futures", "lastUpdateDay", CFuncCommon::ToString(m_nLastUpdateDay));
			m_config.set_value("futures", "todayBeginMoney", m_accountInfo.equity.c_str());
			m_config.save("./config.ini");
		}
		if(!CFuncCommon::CheckEqual(m_beginMoney, 0.0))
		{
			double equity = stod(m_accountInfo.equity);
			CString strTemp;
			if(equity >= m_beginMoney)
				strTemp.Format("%.4f(%.2f%%)", equity - m_beginMoney, (equity - m_beginMoney) / m_beginMoney * 100);
			else
				strTemp.Format("-%.4f(-%.2f%%)", m_beginMoney - equity, (m_beginMoney - equity) / m_beginMoney * 100);
			m_staticProfit.SetWindowText(strTemp);
		}
		if(!CFuncCommon::CheckEqual(m_todayBeginMoney, 0.0))
		{
			double equity = stod(m_accountInfo.equity);
			CString strTemp;
			if(equity >= m_todayBeginMoney)
				strTemp.Format("%.4f(%.2f%%)", equity - m_todayBeginMoney, (equity - m_todayBeginMoney) / m_todayBeginMoney * 100);
			else
				strTemp.Format("-%.4f(-%.2f%%)", m_todayBeginMoney - equity, (m_todayBeginMoney - equity) / m_todayBeginMoney * 100);
			m_staticTodayProfit.SetWindowText(strTemp);
		}
	}
}

void COKExFuturesDlg::OnBnClickedButtonUpdateCost()
{
	m_beginMoney = 0.0;
	CString szCost;
	m_editCapital.GetWindowText(szCost);
	m_beginMoney = stod(szCost.GetBuffer());
	m_config.set_value("futures", "beginMoney", szCost.GetBuffer());
	m_config.save("./config.ini");
}

void COKExFuturesDlg::OnBnClickedButtonUpdateTodayCost()
{
	m_todayBeginMoney = 0.0;
	CString szCost;
	m_editCapitalToday.GetWindowText(szCost);
	m_todayBeginMoney = stod(szCost.GetBuffer());
	m_config.set_value("futures", "todayBeginMoney", szCost.GetBuffer());
	m_config.save("./config.ini");
}



void COKExFuturesDlg::OnBnClickedButtonUpdateTradeMoment()
{
	CString strTradeMoment;
	m_combTradeMoment.GetWindowText(strTradeMoment);
	if(strTradeMoment == "")
	{
		MessageBox("未选择交易时机");
		return;
	}
	if(strTradeMoment == "趋势出现正向")
		m_tradeMoment = 0;
	else
		m_tradeMoment = 1;
	m_config.set_value("futures", "tradeMoment", strTradeMoment.GetBuffer());
	m_config.save("./config.ini");
}

bool COKExFuturesDlg::__CheckCanStopProfit(eFuturesTradeType eOpenType, std::string& orderID)
{
	return true;
	if(eOpenType == eFuturesTradeType_OpenBull && m_eBollState == eBollTrend_ZhangKou && m_bZhangKouUp)
	{
		//除开自己以外还能有足够对空单进行对冲的单子
		std::list<SFuturesTradePairInfo>::iterator itB = m_listTradePairInfo.begin();
		std::list<SFuturesTradePairInfo>::iterator itE = m_listTradePairInfo.end();
		int bearCnt = 0;
		while(itB != itE)
		{
			if(itB->open.orderID != "" && itB->close.strClientOrderID == "" && itB->open.orderID != orderID)
			{
				if(itB->open.tradeType == eFuturesTradeType_OpenBull)
					bearCnt--;
				else if(itB->open.tradeType == eFuturesTradeType_OpenBear)
					bearCnt++;
			}
			++itB;
		}
		if(bearCnt > 0)
		{
			if(m_bCanStopProfit)
				CActionLog("trade", "多单止盈失败,空单持仓对冲");
			return false;
		}
	}
	else if(eOpenType == eFuturesTradeType_OpenBear && m_eBollState == eBollTrend_ZhangKou && !m_bZhangKouUp)
	{
		std::list<SFuturesTradePairInfo>::iterator itB = m_listTradePairInfo.begin();
		std::list<SFuturesTradePairInfo>::iterator itE = m_listTradePairInfo.end();
		int bullCnt = 0;
		while(itB != itE)
		{
			if(itB->open.orderID != "" && itB->close.strClientOrderID == "" && itB->open.orderID != orderID)
			{
				if(itB->open.tradeType == eFuturesTradeType_OpenBull)
					bullCnt++;
				else if(itB->open.tradeType == eFuturesTradeType_OpenBear)
					bullCnt--;
			}
			++itB;
		}
		if(bullCnt > 0)
		{
			if(m_bCanStopProfit)
				CActionLog("trade", "空单止盈失败,多单持仓对冲");
			return false;
		}
	}
	return true;
}

void COKExFuturesDlg::ConnectServer()
{
	if(m_pNet == NULL)
		return;
	SetLoginState(false);
	// 连接账号服务器
	if(m_pServerFactory == NULL)
	{
		m_pServerFactory = new server_factory();
	}
	clib::pclient p = m_pNet->create_client(&m_netHandle, m_pServerFactory);
	if(p)
	{
		p->asyc_connect("47.52.153.233", "9191");
	}
	if(p){
		p->del_ref();
	}

}

void COKExFuturesDlg::RetryConnectServer()
{
	SetTimer(eTimerType_ConnetServer, 3000, NULL);
	LOCAL_ERROR("error, connect to server after 3 seconds ");
}

void COKExFuturesDlg::OnRecvLoginRst(int rst, time_t _time)
{
	if(rst == 0)
	{
		SetLoginState(true, _time);
		if(_time != 0)
		{
			std::string text = CFuncCommon::FormatTimeStr(_time);
			LOCAL_INFO("账号验证通过, 到期时间%s", text.c_str());
		}
		else
		{
			LOCAL_INFO("白名单账号验证通过");
		}
	}
	else
	{
		LOCAL_ERROR("账号验证失败");
	}

}

void COKExFuturesDlg::OnRecvAccountInvalid()
{
	SetLoginState(false);
	LOCAL_ERROR("账号验证失败");
}

void COKExFuturesDlg::SetLoginState(bool bSuccess, time_t passTime)
{
	m_bSuccessLogin = bSuccess;
	if(bSuccess)
	{
		std::string text = CFuncCommon::FormatTimeStr(passTime);
		CString szTemp;
		szTemp.Format("验证通过 过期时间%s", passTime==0 ? "无限制" : text.c_str());
		m_staticAccountState.SetWindowText(szTemp);
	}
	else
		m_staticAccountState.SetWindowText("验证失败");
}

void COKExFuturesDlg::OnRecvServerPong()
{
	m_tListenServerPong = 0;
}

bool COKExFuturesDlg::_FindZhangKou(int beginBarIndex, double& minValue)
{
	if(strcmp(KLINE_DATA[KLINE_DATA_SIZE-1].szTime, "2019-01-21 22:15:00") == 0)
		int a = 3;
	if(REAL_BOLL_DATA_SIZE < m_nZhangKouCheckCycle)//判断张口
		return false;
	int beginBar = 0;
	if(BOLL_DATA_SIZE - beginBarIndex >= m_nBollCycle)
		beginBar = BOLL_DATA_SIZE - m_nBollCycle;
	else
		beginBar = beginBarIndex;
	if(beginBar > 0)
	{
		//先找到min值
		int minBar = 0;
		minValue = 100000.0;
		for(int i=beginBar; i<BOLL_DATA_SIZE; ++i)
		{
			double offset = BOLL_DATA[i].up - BOLL_DATA[i].dn;
			if(offset < minValue)
			{
				minValue = offset;
				minBar = i;
			}
		}
		//大于1.4的间距才考虑进行张口判断
		double offset = BOLL_DATA[BOLL_DATA_SIZE - 1].up - BOLL_DATA[BOLL_DATA_SIZE - 1].dn;
		if(offset / minValue > 1.4)
		{
			int check = m_nZhangKouTrendCheckCycle;
			bool bDFast = _IsBollDirForward(false, check, m_bollCheckAngleFast);
			bool bDSlow = _IsBollDirForward(false, check, m_bollCheckAngleSlow);
			bool bUFast = _IsBollDirForward(true, check, m_bollCheckAngleFast);
			bool bUSlow = _IsBollDirForward(true, check, m_bollCheckAngleSlow);
			if((bDFast && bUFast) || (bDFast && bUSlow) || (bDSlow && bUFast))
				return true;
		}
	}
	return false;
}

bool COKExFuturesDlg::_IsBollDirForward(bool bUp, int checkNum, double checkAngle)
{
	double last = 0.0;
	int good = 0;
	for(int i = KLINE_DATA_SIZE - checkNum; i < KLINE_DATA_SIZE; ++i)
	{
		if(i == KLINE_DATA_SIZE - checkNum)
		{
			if(bUp)
				last = BOLL_DATA[i].up;
			else
				last = BOLL_DATA[i].dn;
		}
		else
		{
			if(bUp)
			{
				if(BOLL_DATA[i].up <= last)
					return false;
				if(BOLL_DATA[i].up / last < checkAngle)
				{
					if(checkNum == 2)
						return false;
					last = BOLL_DATA[i].up;
					continue;
				}
				last = BOLL_DATA[i].up;
			}
			else
			{
				if(BOLL_DATA[i].dn >= last)
					return false;
				if(last / BOLL_DATA[i].dn < checkAngle)
				{
					if(checkNum == 2)
						return false;
					last = BOLL_DATA[i].dn;
					continue;
				}
				last = BOLL_DATA[i].dn;
			}
			good++;
		}
	}
	return (good > 0);
}

bool COKExFuturesDlg::_FindShouKou()
{
	if(m_eBollState == eBollTrend_Normal)
	{
		if(REAL_BOLL_DATA_SIZE >= m_nShouKouCheckCycle)//判断收口
		{
			int maxBar = 0;
			double maxValue = 0.0;
			for(int i = BOLL_DATA_SIZE - 1; i >= BOLL_DATA_SIZE - m_nShouKouCheckCycle; --i)
			{
				double offset = BOLL_DATA[i].up - BOLL_DATA[i].dn;
				if(offset > maxValue)
				{
					maxValue = offset;
					maxBar = i - 1;
				}
			}
			double offset = BOLL_DATA[BOLL_DATA_SIZE - 1].up - BOLL_DATA[BOLL_DATA_SIZE - 1].dn;
			if(maxValue / offset > 3)
			{
				double avgPrice = (KLINE_DATA[KLINE_DATA_SIZE - 1].highPrice + KLINE_DATA[KLINE_DATA_SIZE - 1].lowPrice) / 2;
				if(offset / avgPrice < 0.02)
					return true;
			}
		}
	}
	else if(m_eBollState == eBollTrend_ZhangKou)
	{
		//寻找收口,从确定张口的柱子开始
		int maxBar = 0;
		double maxValue = 0.0;
		for(int i = m_nZhangKouConfirmBar; i < BOLL_DATA_SIZE; ++i)
		{
			//up或dn恢复到前面某根柱子值,并且该柱子和最后一根柱子之前间隔1根及以上柱子,表示一个反转的过程,出现收口
			if(m_bZhangKouUp)
			{
				if(BOLL_DATA[BOLL_DATA_SIZE - 1].up < BOLL_DATA[i].up && (BOLL_DATA_SIZE - 1 - i) > 1)
					return true;
			}
			else
			{

				if(BOLL_DATA[BOLL_DATA_SIZE - 1].dn > BOLL_DATA[i].dn && (BOLL_DATA_SIZE - 1 - i) > 1)
					return true;
			}
			double offset = BOLL_DATA[i].up - BOLL_DATA[i].dn;
			if(offset > maxValue)
			{
				maxValue = offset;
				maxBar = i - 1;
			}
		}
		double offset = BOLL_DATA[BOLL_DATA_SIZE - 1].up - BOLL_DATA[BOLL_DATA_SIZE - 1].dn;
		if((offset / m_nZhangKouMinValue) < 1.4)
			return true;
		if((maxValue / offset) > 2.24)
		{
			double avgPrice = (KLINE_DATA[KLINE_DATA_SIZE - 1].highPrice + KLINE_DATA[KLINE_DATA_SIZE - 1].lowPrice) / 2;
			if(offset / avgPrice < 0.02)
				return true;
		}
	}
	return false;
}