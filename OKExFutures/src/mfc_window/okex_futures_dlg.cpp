
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
#include "exchange/okex/okex_websocket_api.h"
#include "exchange/okex/okex_http_api.h"
#include "common/func_common.h"
#include "test_kline_data.h"
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
#define OKEX_WEB_SOCKET ((COkexWebsocketAPI*)pExchange->GetWebSocket())
#define OKEX_HTTP ((COkexHttpAPI*)pExchange->GetHttp())
#define OKEX_TRADE_HTTP ((COkexHttpAPI*)pExchange->GetTradeHttp())
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
void local_http_callbak_message(eHttpAPIType apiType, Json::Value& retObj, const std::string& strRet, int customData, std::string strCustomData)
{
	switch(apiType)
	{
	case eHttpAPIType_FuturesAccountInfoByCurrency:
		{
			if(retObj.isObject() && retObj["equity"].isString() && retObj["margin"].isString())
			{
				if(customData == OKEX_HTTP->m_futuresAccountInfoByCurrencyIndex)
				{
					SFuturesAccountInfo data;
					data.equity = stod(retObj["equity"].asString());
					data.margin = stod(retObj["margin"].asString());
				}
			}
			else
				LOCAL_ERROR("http type=%d ret=%s", apiType, strRet.c_str());
		}
		break;
	case eHttpAPIType_FuturesServerTime:
		{
		}
		break;
	case eHttpAPIType_FuturesTrade:
		{
			if(retObj.isObject() && retObj["error_code"].isInt() && retObj["error_code"].asInt() == 0)
			{
				if(retObj["client_oid"].isString())
					g_pDlg->OnTradeSuccess(retObj["client_oid"].asString(), retObj["order_id"].asString());
			}
			else
			{
				if(retObj["client_oid"].isString())
					g_pDlg->OnTradeFail(retObj["client_oid"].asString());
				LOCAL_ERROR("http type=%d ret=%s", apiType, strRet.c_str());
			}
		}
		break;
	case eHttpAPIType_FuturesTradeInfo:
		{
			if(retObj.isObject() && retObj["order_id"].isString())
			{
				SFuturesTradeInfo info;
				info.strClientOrderID = "";
				info.timeStamp = CFuncCommon::ISO8601ToTime(retObj["timestamp"].asString());
				info.filledQTY = retObj["filled_qty"].asString();
				info.orderID = retObj["order_id"].asString();
				info.price = stod(retObj["price"].asString());
				info.status = retObj["status"].asString();
				std::string tradeType = retObj["type"].asString();
				if(tradeType == "1")
					info.tradeType = eFuturesTradeType_OpenBull;
				else if(tradeType == "2")
					info.tradeType = eFuturesTradeType_OpenBear;
				else if(tradeType == "3")
					info.tradeType = eFuturesTradeType_CloseBull;
				else if(tradeType == "4")
					info.tradeType = eFuturesTradeType_CloseBear;
				g_pDlg->UpdateTradeInfo(info);
				CActionLog("trade", "http更新订单信息 client_order=%s, order=%s, filledQTY=%s, price=%s, status=%s, tradeType=%s", strCustomData.c_str(), info.orderID.c_str(), info.filledQTY.c_str(), retObj["price"].asString().c_str(), info.status.c_str(), tradeType.c_str());
			}
			else
				LOCAL_ERROR("http type=%d ret=%s", apiType, strRet.c_str());
		}
		break;
	case eHttpAPIType_FuturesCancelOrder:
		{
			if(retObj.isObject() && retObj["result"].isBool() && retObj["result"].asBool() == true)
			{
				CActionLog("trade", "撤消订单成功 order=%s", retObj["order_id"].asString().c_str());
			}
			else
				LOCAL_ERROR("http type=%d ret=%s", apiType, strRet.c_str());
		}
		break;
	default:
		break;
	}
}

void local_websocket_callbak_open(const char* szExchangeName)
{
	LOCAL_INFO("连接成功");
	g_pDlg->m_tListenPong = 0;
	if(g_pDlg->m_bRun)
	{
		g_pDlg->m_bRun = false;
		g_pDlg->OnBnClickedButtonStart();
	}
}

void local_websocket_callbak_close(const char* szExchangeName)
{
	LOCAL_ERROR("断开连接");
	g_pDlg->m_tListenPong = 0;
}

void local_websocket_callbak_fail(const char* szExchangeName)
{
	LOCAL_ERROR("连接失败");
	g_pDlg->m_tListenPong = 0;
}

time_t lastKlineTime = 0;
std::string lastKlineRetStr = "";
Json::Value lastKlineJson;
void local_websocket_callbak_message(eWebsocketAPIType apiType, const char* szExchangeName, Json::Value& retObj, const std::string& strRet)
{
	switch(apiType)
	{
	case eWebsocketAPIType_FuturesKline:
		{
			char* szEnd = NULL;
			time_t curTime = CFuncCommon::ISO8601ToTime(retObj["data"][0]["candle"][0].asString());
			if(g_pDlg->m_bFirstKLine)
			{
				g_pDlg->m_bFirstKLine = false;
				g_pDlg->ComplementedKLine(curTime, g_pDlg->m_nBollCycle);
			}
			CActionLog("all_kline", "%s", strRet.c_str());
			if(curTime >= lastKlineTime)
			{
				if(curTime > lastKlineTime && lastKlineTime != 0)
				{
					CActionLog("market", "%s", lastKlineRetStr.c_str());
					SKlineData data;
					data.time = CFuncCommon::ISO8601ToTime(lastKlineJson["data"][0]["candle"][0].asString());
					data.openPrice = stod(lastKlineJson["data"][0]["candle"][1].asString());
					data.highPrice = stod(lastKlineJson["data"][0]["candle"][2].asString());
					data.lowPrice =stod(lastKlineJson["data"][0]["candle"][3].asString());
					data.closePrice = stod(lastKlineJson["data"][0]["candle"][4].asString());
					data.volume = stoi(lastKlineJson["data"][0]["candle"][5].asString());
					data.volumeByCurrency =stod(lastKlineJson["data"][0]["candle"][6].asString());
					g_pDlg->AddKlineData(data);
				}
				lastKlineTime = curTime;
				lastKlineRetStr = strRet;
				lastKlineJson = retObj;
			}
		}
		break;
	case eWebsocketAPIType_FuturesTicker:
		{
			char* szEnd = NULL;
			CActionLog("market", "%s", strRet.c_str());
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
		break;
	case eWebsocketAPIType_Pong:
		{
			g_pDlg->Pong();
		}
		break;
	case eWebsocketAPIType_Login:
		{
			g_pDlg->OnLoginSuccess();
		}
		break;
	case eWebsocketAPIType_FuturesOrderInfo:
		{
			if(retObj.isObject() && retObj["data"].isArray())
			{
				SFuturesTradeInfo info;
				info.strClientOrderID = "";
				info.timeStamp = CFuncCommon::ISO8601ToTime(retObj["data"][0]["timestamp"].asString());
				info.filledQTY = retObj["data"][0]["filled_qty"].asString();
				info.orderID = retObj["data"][0]["order_id"].asString();
				info.price = stod(retObj["data"][0]["price"].asString());
				info.status = retObj["data"][0]["status"].asString();
				info.size = retObj["data"][0]["size"].asString();
				std::string tradeType = retObj["data"][0]["type"].asString();
				if(tradeType == "1")
					info.tradeType = eFuturesTradeType_OpenBull;
				else if(tradeType == "2")
					info.tradeType = eFuturesTradeType_OpenBear;
				else if(tradeType == "3")
					info.tradeType = eFuturesTradeType_CloseBull;
				else if(tradeType == "4")
					info.tradeType = eFuturesTradeType_CloseBear;
				g_pDlg->UpdateTradeInfo(info);
				CActionLog("trade", "ws更新订单信息 order=%s, filledQTY=%s, price=%s, status=%s, tradeType=%s", info.orderID.c_str(), info.filledQTY.c_str(), retObj["data"][0]["price"].asString().c_str(), info.status.c_str(), tradeType.c_str());
			}
		}
		break;
	case eWebsocketAPIType_FuturesAccountInfo:
		{
			if(retObj.isObject() && retObj["data"].isArray())
			{

			}
			else
				LOCAL_ERROR("ws type=%d ret=%s", apiType, strRet.c_str());
		}
		break;
	default:
		break;
	}
}

COKExFuturesDlg::COKExFuturesDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(COKExFuturesDlg::IDD, pParent)
{
	g_pDlg = this;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_nBollCycle = 20;
	m_nPriceDecimal = 2;
	m_nZhangKouCheckCycle = 20;
	m_nShouKouCheckCycle = 20;
	m_nZhangKouTrendCheckCycle = 5;
	m_bRun = false;
	m_eBollState = eBollTrend_Normal;
	m_eLastBollState = eBollTrend_Normal;
	m_nZhangKouDoubleConfirmCycle = 2;
	m_nShoukouDoubleConfirmCycle = 3;
	m_tListenPong = 0;
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
	m_combLeverage.InsertString(0, "10");
	m_combLeverage.InsertString(1, "20");

	m_listCtrlOrderOpen.InsertColumn(0, "价格", LVCFMT_CENTER, 70);
	m_listCtrlOrderOpen.InsertColumn(1, "类型", LVCFMT_CENTER, 50);
	m_listCtrlOrderOpen.InsertColumn(2, "成交量", LVCFMT_CENTER, 70);
	m_listCtrlOrderOpen.InsertColumn(3, "状态", LVCFMT_CENTER, 73);
	m_listCtrlOrderOpen.InsertColumn(4, "最低价", LVCFMT_CENTER, 70);
	m_listCtrlOrderOpen.InsertColumn(5, "最高价", LVCFMT_CENTER, 70);
	m_listCtrlOrderOpen.InsertColumn(6, "参考利润", LVCFMT_CENTER, 85);
	m_listCtrlOrderOpen.InsertColumn(7, "下单时间", LVCFMT_CENTER, 90);
	m_listCtrlOrderOpen.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

	m_listCtrlOrderClose.InsertColumn(0, "价格", LVCFMT_CENTER, 70);
	m_listCtrlOrderClose.InsertColumn(1, "成交量", LVCFMT_CENTER, 70);
	m_listCtrlOrderClose.InsertColumn(2, "状态", LVCFMT_CENTER, 73);
	m_listCtrlOrderClose.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

	if(!m_config.open("./config.ini"))
		return FALSE;
	m_apiKey = m_config.get("futures", "apiKey", "");
	m_secretKey = m_config.get("futures", "secretKey", "");
	m_passphrase = m_config.get("futures", "passphrase", "");
	
	__InitConfigCtrl();

	pExchange = new COkexExchange(m_apiKey, m_secretKey, m_passphrase, true);
	pExchange->SetHttpCallBackMessage(local_http_callbak_message);
	pExchange->SetWebSocketCallBackOpen(local_websocket_callbak_open);
	pExchange->SetWebSocketCallBackClose(local_websocket_callbak_close);
	pExchange->SetWebSocketCallBackFail(local_websocket_callbak_fail);
	pExchange->SetWebSocketCallBackMessage(local_websocket_callbak_message);
	pExchange->Run();

	SetTimer(eTimerType_APIUpdate, 1, NULL);
	SetTimer(eTimerType_Ping, 30000, NULL);

	clib::string log_path = "log/";
	bool bRet = clib::file_util::mkfiledir(log_path.c_str(), true);

	CLocalLogger& _localLogger = CLocalLogger::GetInstance();
	_localLogger.SetBatchMode(true);
	_localLogger.SetLogPath(log_path.c_str());
	_localLogger.Start();
	_localLogger.SetCallBackFunc(LocalLogCallBackFunc);
	CLocalActionLog::GetInstancePt()->set_log_path(log_path.c_str());
	CLocalActionLog::GetInstancePt()->start();

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
			if(m_tListenPong && time(NULL) - m_tListenPong > 15)
			{
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
		}
		break;
	case eTimerType_Ping:
		{
			if(OKEX_WEB_SOCKET->Ping())
				m_tListenPong = time(NULL);
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
	bool bFound = false;
	std::string instrumentID = m_strCoinType + "-USD-" + m_strFuturesCycle;
	for(int i = 0; i<3; ++i)
	{
		SHttpResponse resInfo;
		OKEX_HTTP->API_FuturesInstruments(false, &resInfo);
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
	if(OKEX_WEB_SOCKET)
	{
		OKEX_WEB_SOCKET->API_FuturesKlineData(true, m_strKlineCycle, m_strCoinType, m_strFuturesCycle);
		OKEX_WEB_SOCKET->API_FuturesTickerData(true, m_strCoinType, m_strFuturesCycle);
		OKEX_WEB_SOCKET->API_LoginFutures(m_apiKey, m_secretKey, time(NULL));
		//OKEX_HTTP->API_FuturesAccountInfoByCurrency(m_strCoinType);
		//OKEX_HTTP->API_FuturesServerTime();
	}
	m_bRun = true;
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
	//if(strcmp(data.szTime, "2019-01-02 11:57:00") == 0)
	//	int a = 3;
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
			lineStream >> szContent >> szContent >> szContent;
			Json::Value retObj;
			Json::Reader reader;
			reader.parse(szContent, retObj);
			if(retObj.isObject() && retObj["table"].isString())
			{
				std::string strChannel = retObj["table"].asString();

				std::string strTickChannel = "futures/ticker";

				std::string strKlineChannel = "futures/";
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
	if(REAL_BOLL_DATA_SIZE >= m_nZhangKouCheckCycle)//判断张口
	{
		int minBar = 0;
		double minValue = 100000.0;
		for(int i = BOLL_DATA_SIZE-1; i>=BOLL_DATA_SIZE-m_nZhangKouCheckCycle; --i)
		{
			double offset = BOLL_DATA[i].up - BOLL_DATA[i].dn;
			if(offset < minValue)
			{
				minValue = offset;
				minBar = i;
			}
		}
		double offset = BOLL_DATA[BOLL_DATA_SIZE-1].up - BOLL_DATA[BOLL_DATA_SIZE-1].dn;
		if(offset / minValue > 2.5)
		{
			__SetBollState(eBollTrend_ZhangKou, 0, minValue);
			return;
		}
		else if(offset / minValue > 1.5)
		{
			int check = m_nZhangKouTrendCheckCycle/2 + 1;
			if(KLINE_DATA_SIZE >= check)
			{
				int up = 0;
				int down = 0;
				for(int i = KLINE_DATA_SIZE-1; i>=KLINE_DATA_SIZE-check; --i)
				{
					if(KLINE_DATA[i].lowPrice >= BOLL_DATA[i].up)
						up++;
					else if(KLINE_DATA[i].lowPrice > BOLL_DATA[i].dn && KLINE_DATA[i].lowPrice < BOLL_DATA[i].up && KLINE_DATA[i].highPrice > BOLL_DATA[i].up)
						up++;
					else if(KLINE_DATA[i].highPrice <= BOLL_DATA[i].dn)
						down++;
					else if(KLINE_DATA[i].highPrice < BOLL_DATA[i].up && KLINE_DATA[i].highPrice > BOLL_DATA[i].dn && KLINE_DATA[i].lowPrice < BOLL_DATA[i].dn)
						down++;
				}
				if(up == check && KLINE_DATA[KLINE_DATA_SIZE-1].closePrice > BOLL_DATA[BOLL_DATA_SIZE-1].up)
				{
					double min_up = 100.0;
					for(int i = BOLL_DATA_SIZE-1; i>=BOLL_DATA_SIZE-m_nZhangKouCheckCycle; --i)
					{
						if(BOLL_DATA[i].up < min_up)
							min_up = BOLL_DATA[i].up;
					}
					if((BOLL_DATA[BOLL_DATA_SIZE-1].up / min_up) >= 1.005)
					{
						__SetBollState(eBollTrend_ZhangKou, 1, minValue);
						return;
					}

				}
				else if(down == check && KLINE_DATA[KLINE_DATA_SIZE-1].closePrice < BOLL_DATA[BOLL_DATA_SIZE-1].dn)
				{
					double max_down = 0;
					for(int i = BOLL_DATA_SIZE-1; i>=BOLL_DATA_SIZE-m_nZhangKouCheckCycle; --i)
					{
						if(BOLL_DATA[i].dn > max_down)
							max_down = BOLL_DATA[i].dn;
					}
					if((BOLL_DATA[BOLL_DATA_SIZE-1].dn / max_down) <= 0.095)
					{
						__SetBollState(eBollTrend_ZhangKou, 1, minValue);
						return;
					}
					__SetBollState(eBollTrend_ZhangKou, 1, minValue);
					return;
				}
			}
		}
	}
	if(REAL_BOLL_DATA_SIZE >= m_nShouKouCheckCycle)//判断收口
	{
		int maxBar = 0;
		double maxValue = 0.0;
		for(int i = BOLL_DATA_SIZE-1; i>=BOLL_DATA_SIZE-m_nShouKouCheckCycle; --i)
		{
			double offset = BOLL_DATA[i].up - BOLL_DATA[i].dn;
			if(offset > maxValue)
			{
				maxValue = offset;
				maxBar = i-1;
			}
		}
		double offset = BOLL_DATA[BOLL_DATA_SIZE-1].up - BOLL_DATA[BOLL_DATA_SIZE-1].dn;
		if(maxValue / offset > 3)
		{
			double avgPrice = (KLINE_DATA[KLINE_DATA_SIZE-1].highPrice + KLINE_DATA[KLINE_DATA_SIZE-1].lowPrice) / 2;
			if(offset/avgPrice < 0.02)
			{
				__SetBollState(eBollTrend_ShouKou);
				return;
			}
		}
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

		}
		else
		{
			if(BOLL_DATA[BOLL_DATA_SIZE-1].dn > BOLL_DATA[BOLL_DATA_SIZE-2].dn)
			{
				CActionLog("boll", "张口不成立");
				__SetBollState(m_eLastBollState);
			}
		}
	}
	else
	{
		//寻找收口,从确定张口的柱子开始
		int maxBar = 0;
		double maxValue = 0.0;
		for(int i = m_nZhangKouConfirmBar; i<BOLL_DATA_SIZE; ++i)
		{
			if(m_bZhangKouUp)
			{
				if(BOLL_DATA[BOLL_DATA_SIZE-1].up < BOLL_DATA[i].up && (BOLL_DATA_SIZE-1-i) > 1)
				{
					__SetBollState(eBollTrend_ShouKou);
					return;
				}
			}
			else
			{

				if(BOLL_DATA[BOLL_DATA_SIZE-1].dn > BOLL_DATA[i].dn && (BOLL_DATA_SIZE-1-i) > 1)
				{
					__SetBollState(eBollTrend_ShouKou);
					return;
				}
			}
			double offset = BOLL_DATA[i].up - BOLL_DATA[i].dn;
			if(offset > maxValue)
			{
				maxValue = offset;
				maxBar = i-1;
			}
		}
		double offset = BOLL_DATA[BOLL_DATA_SIZE-1].up - BOLL_DATA[BOLL_DATA_SIZE-1].dn;
		if((offset/m_nZhangKouMinValue) < 1.5)
		{
			__SetBollState(eBollTrend_ShouKou);
			return;
		}
		if((maxValue/offset) > 2.5)
		{
			double avgPrice = (KLINE_DATA[KLINE_DATA_SIZE-1].highPrice + KLINE_DATA[KLINE_DATA_SIZE-1].lowPrice) / 2;
			if(offset/avgPrice < 0.02)
			{
				__SetBollState(eBollTrend_ShouKou);
				return;
			}
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
	//寻找张口,从确定收口的柱子开始
	int minBar = 0;
	double minValue = 100000.0;
	for(int i = m_nShouKouConfirmBar; i<BOLL_DATA_SIZE; ++i)
	{
		double offset = BOLL_DATA[i].up - BOLL_DATA[i].dn;
		if(offset < minValue)
		{
			minValue = offset;
			minBar = i;
		}
	}
	double offset = BOLL_DATA[BOLL_DATA_SIZE-1].up - BOLL_DATA[BOLL_DATA_SIZE-1].dn;
	if(offset / minValue > 2.5)
	{
		__SetBollState(eBollTrend_ZhangKou, 0, minValue);
		return;
	}
	else if(offset / minValue > 1.5)
	{
		int check = m_nZhangKouTrendCheckCycle/2 + 1;
		if(KLINE_DATA_SIZE >= check)
		{
			int up = 0;
			int down = 0;
			for(int i = KLINE_DATA_SIZE-1; i>=KLINE_DATA_SIZE-check; --i)
			{
				if(KLINE_DATA[i].lowPrice >= BOLL_DATA[i].up)
					up++;
				else if(KLINE_DATA[i].lowPrice > BOLL_DATA[i].dn && KLINE_DATA[i].lowPrice < BOLL_DATA[i].up && KLINE_DATA[i].highPrice > BOLL_DATA[i].up)
					up++;
				else if(KLINE_DATA[i].highPrice <= BOLL_DATA[i].dn)
					down++;
				else if(KLINE_DATA[i].highPrice < BOLL_DATA[i].up && KLINE_DATA[i].highPrice > BOLL_DATA[i].dn && KLINE_DATA[i].lowPrice < BOLL_DATA[i].dn)
					down++;
			}
			if(up == check && KLINE_DATA[KLINE_DATA_SIZE-1].closePrice > BOLL_DATA[BOLL_DATA_SIZE-1].up)
			{
				__SetBollState(eBollTrend_ZhangKou, 1, minValue);
				return;
			}
			else if(down == check && KLINE_DATA[KLINE_DATA_SIZE-1].closePrice < BOLL_DATA[BOLL_DATA_SIZE-1].dn)
			{
				__SetBollState(eBollTrend_ZhangKou, 1, minValue);
				return;
			}
		}
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
		int checkBarNum = 0;
		if(m_eLastBollState == eBollTrend_ShouKou)
		{
			checkBarNum = m_nShouKouConfirmBar;
			if(BOLL_DATA_SIZE-1-m_nShouKouConfirmBar > m_nBollCycle)
				checkBarNum = m_nBollCycle;
			else
				checkBarNum = BOLL_DATA_SIZE-1-m_nShouKouConfirmBar;
		}
		else
			checkBarNum = m_nBollCycle;
		if(checkBarNum > 0)
		{
			int minBar = 0;
			double minValue = 100000.0;
			int cnt = 0;
			for(int i = BOLL_DATA_SIZE-1; i>=0; --i)
			{
				double offset = BOLL_DATA[i].up - BOLL_DATA[i].dn;
				if(offset < minValue)
				{
					minValue = offset;
					minBar = i;
				}
				cnt++;
				if(cnt == checkBarNum)
					break;
			}
			double offset = BOLL_DATA[BOLL_DATA_SIZE-1].up - BOLL_DATA[BOLL_DATA_SIZE-1].dn;
			if(offset / minValue > 2.5)
			{
				__SetBollState(eBollTrend_ZhangKou, 0, minValue);
				return;
			}
			else if(offset / minValue > 1.5)
			{
				int check = m_nZhangKouTrendCheckCycle/2 + 1;
				if(KLINE_DATA_SIZE >= check)
				{
					int up = 0;
					int down = 0;
					for(int i = KLINE_DATA_SIZE-1; i>=KLINE_DATA_SIZE-check; --i)
					{
						if(KLINE_DATA[i].lowPrice >= BOLL_DATA[i].up)
							up++;
						else if(KLINE_DATA[i].lowPrice > BOLL_DATA[i].mb && KLINE_DATA[i].lowPrice < BOLL_DATA[i].up && KLINE_DATA[i].highPrice > BOLL_DATA[i].up)
							up++;
						else if(KLINE_DATA[i].highPrice <= BOLL_DATA[i].dn)
							down++;
						else if(KLINE_DATA[i].highPrice < BOLL_DATA[i].mb && KLINE_DATA[i].highPrice > BOLL_DATA[i].dn && KLINE_DATA[i].lowPrice < BOLL_DATA[i].dn)
							down++;
					}
					if(up == check)// && KLINE_DATA[KLINE_DATA_SIZE-1].closePrice > BOLL_DATA[BOLL_DATA_SIZE-1].up)
					{
						__SetBollState(eBollTrend_ZhangKou, 1, minValue);
						return;
					}
					else if(down == check)// && KLINE_DATA[KLINE_DATA_SIZE-1].closePrice < BOLL_DATA[BOLL_DATA_SIZE-1].dn)
					{
						__SetBollState(eBollTrend_ZhangKou, 1, minValue);
						return;
					}
				}
			}
		}
	}
}

void COKExFuturesDlg::__SetBollState(eBollTrend state, int nParam, double dParam)
{
	m_eLastBollState = m_eBollState;
	m_eBollState = state;
	switch(m_eBollState)
	{
	case eBollTrend_ZhangKou:
		{
			m_nZhangKouConfirmBar = KLINE_DATA_SIZE-1;
			m_nZhangKouTradeCheckBar = m_nZhangKouConfirmBar;
			m_nZhangKouMinValue = dParam;
			CString szInfo;
			szInfo.Format("张口产生<<<< 确认时间[%s] %s", CFuncCommon::FormatTimeStr(KLINE_DATA[KLINE_DATA_SIZE-1].time).c_str(), (nParam==0 ? "开口角判断" : "柱体穿插判断"));
			if(KLINE_DATA_SIZE >= m_nZhangKouTrendCheckCycle)
			{
				int up = 0;
				int down = 0;
				for(int i = KLINE_DATA_SIZE-1; i>=KLINE_DATA_SIZE-m_nZhangKouTrendCheckCycle; --i)
				{
					if(KLINE_DATA[i].lowPrice >= BOLL_DATA[i].up)
						up++;
					else if(KLINE_DATA[i].lowPrice > BOLL_DATA[i].dn && KLINE_DATA[i].lowPrice < BOLL_DATA[i].up && KLINE_DATA[i].highPrice > BOLL_DATA[i].up)
						up++;
					else if(KLINE_DATA[i].highPrice <= BOLL_DATA[i].dn)
						down++;
					else if(KLINE_DATA[i].highPrice < BOLL_DATA[i].up && KLINE_DATA[i].highPrice > BOLL_DATA[i].dn && KLINE_DATA[i].lowPrice < BOLL_DATA[i].dn)
						down++;
				}
				CString _szInfo = "";
				if(up > down)
				{
					m_bZhangKouUp = true;
					_szInfo.Format(" 趋势[涨 %d:%d]", up, down);
				}
				else
				{
					m_bZhangKouUp = false;
					_szInfo.Format(" 趋势[跌 %d:%d]", up, down);
				}
				if(up == 0 && down == 0)
				{

					int a = 3;
				}
				szInfo.Append(_szInfo);
			}
			CActionLog("boll", szInfo.GetBuffer());
		}
		break;
	case eBollTrend_ShouKou:
		{
			std::string strConfirmTime = CFuncCommon::FormatTimeStr(KLINE_DATA[KLINE_DATA_SIZE-1].time);
			CActionLog("boll", "收口产生>>>> 确认时间[%s]", strConfirmTime.c_str());
			m_nShouKouConfirmBar = KLINE_DATA_SIZE-1;
		}
		break;
	case eBollTrend_ShouKouChannel:
		{
			std::string strConfirmTime = CFuncCommon::FormatTimeStr(KLINE_DATA[KLINE_DATA_SIZE-1].time);
			CActionLog("boll", "收口通道===== 确认时间[%s] %s", strConfirmTime.c_str(), (nParam==0 ? "趋势判断" : "超时判断"));
			m_nShouKouChannelConfirmBar = KLINE_DATA_SIZE-1;
		}
		break;
	default:
		break;
	}

}

void COKExFuturesDlg::OnBnClickedButtonTest()
{
	if(!__SaveConfigCtrl())
		return;
	/*std::string strClinetOrderID = CFuncCommon::ToString(CFuncCommon::GenUUID());
	std::string price = "4000.00";
	OKEX_HTTP->API_FuturesTrade(eFuturesTradeType_OpenBear, m_strCoinType, m_strFuturesCycle, price, m_strFuturesTradeSize, m_strLeverage, strClinetOrderID);
	SFuturesTradePairInfo info;
	info.open.strClientOrderID = strClinetOrderID;
	info.open.waitClientOrderIDTime = time(NULL);
	info.open.tradeType = eFuturesTradeType_OpenBull;
	m_listTradePairInfo.push_back(info);*/
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
	std::list<SFuturesTradePairInfo>::iterator itB = m_listTradePairInfo.begin();
	std::list<SFuturesTradePairInfo>::iterator itE = m_listTradePairInfo.end();
	while(itB != itE)
	{
		if(itB->open.orderID != "" && !CFuncCommon::CheckEqual(itB->open.minPrice, 0.0) && !CFuncCommon::CheckEqual(itB->open.maxPrice, 0.0))
		{
			if(m_curTickData.last < itB->open.minPrice)
				itB->open.minPrice = m_curTickData.last;
			if(m_curTickData.last > itB->open.maxPrice)
				itB->open.maxPrice = m_curTickData.last;
		}
		++itB;
	}
	_UpdateTradeShow();
	std::string price = CFuncCommon::Double2String(m_curTickData.last + DOUBLE_PRECISION, m_nPriceDecimal);
	price += "[";
	price += CFuncCommon::FormatTimeStr(m_curTickData.time).c_str();
	price += "]";
	m_staticPrice.SetWindowText(price.c_str());
}

void COKExFuturesDlg::__CheckTrade_ZhangKou()
{
	if(m_bStopWhenFinish)
		return;
	//确认张口后第一根柱子
	if(KLINE_DATA_SIZE-1-m_nZhangKouTradeCheckBar <= 1)
	{
		if(m_bZhangKouUp)
		{
			if(m_curTickData.last < m_curTickBoll.up)
			{
				//用买一价格挂多单
				m_nZhangKouTradeCheckBar = 0;
				if(__CheckCanTrade(eFuturesTradeType_OpenBull))
				{
					if(m_bTest)
					{
						std::string strClinetOrderID = CFuncCommon::ToString(CFuncCommon::GenUUID());
						std::string price = CFuncCommon::Double2String(m_curTickData.buy+DOUBLE_PRECISION, m_nPriceDecimal);
						SFuturesTradePairInfo info;
						info.open.strClientOrderID = strClinetOrderID;
						info.open.timeStamp = time(NULL);
						info.open.filledQTY = m_strFuturesTradeSize;
						info.open.orderID = CFuncCommon::ToString(CFuncCommon::GenUUID());
						info.open.price = m_curTickData.buy;
						info.open.status = 2;
						info.open.tradeType = eFuturesTradeType_OpenBull;
						m_listTradePairInfo.push_back(info);
						CActionLog("trade", "[%s]开多单%s张, price=%s, client_oid=%s", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), m_strFuturesTradeSize.c_str(), price.c_str(), strClinetOrderID.c_str());
						CActionLog("trade", "[%s]http更新订单信息 client_order=%s, order=%s, filledQTY=%s, price=%s, status=%s, tradeType=1", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), strClinetOrderID.c_str(), info.open.orderID.c_str(), info.open.filledQTY.c_str(), price.c_str(), "2");
					}
					else
					{
						std::string strClinetOrderID = CFuncCommon::ToString(CFuncCommon::GenUUID());
						std::string price = CFuncCommon::Double2String(m_curTickData.buy+DOUBLE_PRECISION, m_nPriceDecimal);
						OKEX_HTTP->API_FuturesTrade(eFuturesTradeType_OpenBull, m_strCoinType, m_strFuturesCycle, price, m_strFuturesTradeSize, m_strLeverage, strClinetOrderID);
						SFuturesTradePairInfo info;
						info.open.strClientOrderID = strClinetOrderID;
						info.open.waitClientOrderIDTime = time(NULL);
						info.open.tradeType = eFuturesTradeType_OpenBull;
						m_listTradePairInfo.push_back(info);
						CActionLog("trade", "开多单%s张, price=%s, client_oid=%s", m_strFuturesTradeSize.c_str(), price.c_str(), strClinetOrderID.c_str());
					}
				}
			}
		}
		else
		{
			if(m_curTickData.last > m_curTickBoll.dn)
			{
				//用卖一价格挂空单
				m_nZhangKouTradeCheckBar = 0;
				if(__CheckCanTrade(eFuturesTradeType_OpenBear))
				{
					if(m_bTest)
					{
						std::string strClinetOrderID = CFuncCommon::ToString(CFuncCommon::GenUUID());
						std::string price = CFuncCommon::Double2String(m_curTickData.sell+DOUBLE_PRECISION, m_nPriceDecimal);
						SFuturesTradePairInfo info;
						info.open.strClientOrderID = strClinetOrderID;
						info.open.timeStamp = time(NULL);
						info.open.filledQTY = m_strFuturesTradeSize.c_str();
						info.open.orderID = CFuncCommon::ToString(CFuncCommon::GenUUID());
						info.open.price = m_curTickData.sell;
						info.open.status = 2;
						info.open.tradeType = eFuturesTradeType_OpenBear;
						m_listTradePairInfo.push_back(info);
						CActionLog("trade", "[%s]开空单%s张 price=%s, client_oid=%s", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), m_strFuturesTradeSize.c_str(), price.c_str(), strClinetOrderID.c_str());
						CActionLog("trade", "[%s]http更新订单信息 client_order=%s, order=%s, filledQTY=%s, price=%s, status=%s, tradeType=2", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), strClinetOrderID.c_str(), info.open.orderID.c_str(), info.open.filledQTY.c_str(), price.c_str(), "2");
					}
					else
					{
						std::string strClinetOrderID = CFuncCommon::ToString(CFuncCommon::GenUUID());
						std::string price = CFuncCommon::Double2String(m_curTickData.sell+DOUBLE_PRECISION, m_nPriceDecimal);
						OKEX_HTTP->API_FuturesTrade(eFuturesTradeType_OpenBear, m_strCoinType, m_strFuturesCycle, price, m_strFuturesTradeSize, m_strLeverage, strClinetOrderID);
						SFuturesTradePairInfo info;
						info.open.strClientOrderID = strClinetOrderID;
						info.open.waitClientOrderIDTime = time(NULL);
						info.open.tradeType = eFuturesTradeType_OpenBear;
						m_listTradePairInfo.push_back(info);
						CActionLog("trade", "开空单%s张 price=%s, client_oid=%s", m_strFuturesTradeSize.c_str(), price.c_str(), strClinetOrderID.c_str());
					}
				}
			}
		}
	}
}

void COKExFuturesDlg::__CheckTrade_ShouKou()
{

}

void COKExFuturesDlg::__CheckTrade_ShouKouChannel()
{

}

void COKExFuturesDlg::__CheckTradeOrder()
{
	std::list<SFuturesTradePairInfo>::iterator itB = m_listTradePairInfo.begin();
	std::list<SFuturesTradePairInfo>::iterator itE = m_listTradePairInfo.end();
	while(itB != itE)
	{
		//如果交易完成就删除
		bool bOpenFinish = ((itB->open.status == "-1") || (itB->open.status == "2"));
		bool bCloseFinish = ((itB->close.status == "-1") || (itB->close.status == "2"));
		if(bOpenFinish && bCloseFinish)
		{
			std::string openPrice = CFuncCommon::Double2String(itB->open.price+DOUBLE_PRECISION, m_nPriceDecimal);
			std::string closePrice = CFuncCommon::Double2String(itB->close.price+DOUBLE_PRECISION, m_nPriceDecimal);
			CActionLog("finish_trade", "删除已完成%s交易对 open_price=%s, open_num=%s, open_order=%s, close_price=%s, close_num=%s, close_order=%s", ((itB->open.tradeType == eFuturesTradeType_OpenBull) ? "多单" : "空单"), openPrice.c_str(), itB->open.filledQTY.c_str(), itB->open.orderID.c_str(), closePrice.c_str(), itB->close.filledQTY.c_str(), itB->close.orderID.c_str());
			itB = m_listTradePairInfo.erase(itB);
			_UpdateTradeShow();
			continue;
		}
		if(bOpenFinish && itB->open.filledQTY == "0" && itB->close.strClientOrderID == "")
		{
			CActionLog("trade", "删除未完成交易对 order=%s", itB->open.orderID.c_str());
			itB = m_listTradePairInfo.erase(itB);
			_UpdateTradeShow();
			continue;
		}
		//如果已进行平仓交易,等待平仓完成
		//否则判断开仓的盈亏
		if(itB->close.orderID != "" || itB->close.strClientOrderID != "")
		{
		
		}
		else if(itB->open.orderID != "") 
		{
			//多仓
			if(itB->open.tradeType == eFuturesTradeType_OpenBull)
			{
				if(m_curTickData.bValid)
				{
					//超过止损线 平仓
					if(m_curTickData.last < itB->open.price && ((itB->open.price-m_curTickData.last)/itB->open.price >= m_stopLoss))
					{
						std::string price = CFuncCommon::Double2String(m_curTickData.sell+DOUBLE_PRECISION, m_nPriceDecimal);
						if(m_bTest)
						{
							itB->close.strClientOrderID = CFuncCommon::ToString(CFuncCommon::GenUUID());
							itB->close.timeStamp = time(NULL);
							itB->close.filledQTY = m_strFuturesTradeSize;
							itB->close.orderID = CFuncCommon::ToString(CFuncCommon::GenUUID());
							itB->close.price = m_curTickData.sell;
							itB->close.status = 2;
							itB->close.tradeType = eFuturesTradeType_CloseBull;
							CActionLog("trade", "[%s]止损平多 order=%s, price=%s, filledQTY=%s", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), itB->open.orderID.c_str(), price.c_str(), m_strFuturesTradeSize.c_str());
						}
						else
						{
							std::string strClinetOrderID = CFuncCommon::ToString(CFuncCommon::GenUUID());
							OKEX_HTTP->API_FuturesTrade(eFuturesTradeType_CloseBull, m_strCoinType, m_strFuturesCycle, price, m_strFuturesTradeSize, m_strLeverage, strClinetOrderID);
							itB->close.strClientOrderID = strClinetOrderID;
							itB->close.waitClientOrderIDTime = time(NULL);
							itB->close.tradeType = eFuturesTradeType_CloseBull;
							CActionLog("trade", "止损平多 order=%s, price=%s, filledQTY=%s", itB->open.orderID.c_str(), price.c_str(), m_strFuturesTradeSize.c_str());
						}
						
					}
					//盈利达到2倍移动平均线后开始设置止盈线, 回撤破止盈线就平仓
					if(m_curTickData.last > itB->open.price)
					{
						if(itB->open.stopProfit)
						{
							//平仓
							if(m_curTickData.last <= (itB->open.price*(1+itB->open.stopProfit*m_moveStopProfit)))
							{
								//如果open未交易完,先撤单
								if(itB->open.status == "1")
								{
									if(m_bTest)
									{
										itB->open.status = "-1";
										CActionLog("trade", "[%s]撤消订单成功 order=%s", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), itB->open.orderID.c_str());
									}
									else
										OKEX_HTTP->API_FuturesCancelOrder(m_strCoinType, m_strFuturesCycle, itB->open.orderID);
								}
								if(itB->open.filledQTY != "0")
								{
									std::string price = CFuncCommon::Double2String(m_curTickData.sell + DOUBLE_PRECISION, m_nPriceDecimal);
									if(m_bTest)
									{
										itB->close.strClientOrderID = CFuncCommon::ToString(CFuncCommon::GenUUID());
										itB->close.timeStamp = time(NULL);
										itB->close.filledQTY = itB->open.filledQTY;
										itB->close.orderID = CFuncCommon::ToString(CFuncCommon::GenUUID());
										itB->close.price = m_curTickData.sell;
										itB->close.status = 2;
										itB->close.tradeType = eFuturesTradeType_CloseBull;
										CActionLog("trade", "[%s]止盈平多 openClientOrder=%s, order=%s, price=%s, filledQTY=%s", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), itB->open.strClientOrderID.c_str(), itB->open.orderID.c_str(), price.c_str(), itB->open.filledQTY.c_str());
									}
									else
									{
										std::string strClinetOrderID = CFuncCommon::ToString(CFuncCommon::GenUUID());
										OKEX_HTTP->API_FuturesTrade(eFuturesTradeType_CloseBull, m_strCoinType, m_strFuturesCycle, price, itB->open.filledQTY, m_strLeverage, strClinetOrderID);
										itB->close.strClientOrderID = strClinetOrderID;
										itB->close.waitClientOrderIDTime = time(NULL);
										itB->close.tradeType = eFuturesTradeType_CloseBull;
										CActionLog("trade", "止盈平多 openClientOrder=%s, order=%s, price=%s, filledQTY=%s", itB->open.strClientOrderID.c_str(), itB->open.orderID.c_str(), price.c_str(), itB->open.filledQTY.c_str());
									}
								}
							}
							else
							{
								double up = (m_curTickData.last - itB->open.price)/itB->open.price;
								int nowStep = int(up/m_moveStopProfit);
								if(nowStep - itB->open.stopProfit >= 2)
									itB->open.stopProfit = nowStep-1;
							}
						}
						else
						{
							double up = (m_curTickData.last - itB->open.price)/itB->open.price;
							if(up/m_moveStopProfit >= 2)
								itB->open.stopProfit = 1;
						}
					}
				}
			}
			else if(itB->open.tradeType == eFuturesTradeType_OpenBear)
			{
				if(m_curTickData.bValid)
				{
					//超过止损线 平仓
					if(m_curTickData.last > itB->open.price && ((m_curTickData.last-itB->open.price)/itB->open.price >= m_stopLoss))
					{
						std::string price = CFuncCommon::Double2String(m_curTickData.buy + DOUBLE_PRECISION, m_nPriceDecimal);
						if(m_bTest)
						{
							itB->close.strClientOrderID = CFuncCommon::ToString(CFuncCommon::GenUUID());
							itB->close.timeStamp = time(NULL);
							itB->close.filledQTY = m_strFuturesTradeSize;
							itB->close.orderID = CFuncCommon::ToString(CFuncCommon::GenUUID());
							itB->close.price = m_curTickData.buy;
							itB->close.status = 2;
							itB->close.tradeType = eFuturesTradeType_CloseBear;
							CActionLog("trade", "[%s]止损平空 openClientOrder=%s, order=%s, price=%s, filledQTY=%s", itB->open.strClientOrderID.c_str(), CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), itB->open.orderID.c_str(), price.c_str(), m_strFuturesTradeSize.c_str());
						}
						else
						{
							std::string strClinetOrderID = CFuncCommon::ToString(CFuncCommon::GenUUID());
							OKEX_HTTP->API_FuturesTrade(eFuturesTradeType_CloseBear, m_strCoinType, m_strFuturesCycle, price, m_strFuturesTradeSize, m_strLeverage, strClinetOrderID);
							itB->close.strClientOrderID = strClinetOrderID;
							itB->close.waitClientOrderIDTime = time(NULL);
							itB->close.tradeType = eFuturesTradeType_CloseBear;
							CActionLog("trade", "止损平空 openClientOrder=%s, order=%s, price=%s, filledQTY=%s", itB->open.strClientOrderID.c_str(), itB->open.orderID.c_str(), price.c_str(), m_strFuturesTradeSize.c_str());
						}
					}
					//盈利达到2倍移动平均线后开始移动止赢
					if(m_curTickData.last < itB->open.price)
					{
						if(itB->open.stopProfit)
						{
							//平仓
							if(m_curTickData.last >= (itB->open.price*(1 - itB->open.stopProfit*m_moveStopProfit)))
							{
								//如果open未交易完,先撤单
								if(itB->open.status == "1")
								{
									if(m_bTest)
									{
										itB->open.status = "-1";
										CActionLog("trade", "[%s]撤消订单成功 order=%s", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), itB->open.orderID.c_str());
									}
									else
										OKEX_HTTP->API_FuturesCancelOrder(m_strCoinType, m_strFuturesCycle, itB->open.orderID);
								}
								if(itB->open.filledQTY != "0")
								{
									std::string price = CFuncCommon::Double2String(m_curTickData.buy + DOUBLE_PRECISION, m_nPriceDecimal);
									if(m_bTest)
									{
										itB->close.strClientOrderID = CFuncCommon::ToString(CFuncCommon::GenUUID());
										itB->close.timeStamp = time(NULL);
										itB->close.filledQTY = itB->open.filledQTY;
										itB->close.orderID = CFuncCommon::ToString(CFuncCommon::GenUUID());
										itB->close.price = m_curTickData.buy;
										itB->close.status = 2;
										itB->close.tradeType = eFuturesTradeType_CloseBear;
										CActionLog("trade", "[%s]止盈平空 order=%s, price=%s, filledQTY=%s", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), itB->open.orderID.c_str(), price.c_str(), itB->open.filledQTY.c_str());
									}
									else
									{
										std::string strClinetOrderID = CFuncCommon::ToString(CFuncCommon::GenUUID());
										OKEX_HTTP->API_FuturesTrade(eFuturesTradeType_CloseBear, m_strCoinType, m_strFuturesCycle, price, itB->open.filledQTY, m_strLeverage, strClinetOrderID);
										itB->close.strClientOrderID = strClinetOrderID;
										itB->close.waitClientOrderIDTime = time(NULL);
										itB->close.tradeType = eFuturesTradeType_CloseBear;
										CActionLog("trade", "止盈平空 order=%s, price=%s, filledQTY=%s", itB->open.orderID.c_str(), price.c_str(), itB->open.filledQTY.c_str());
									}
								}
							}
							else
							{
								double up = (itB->open.price - m_curTickData.last) / itB->open.price;
								int nowStep = int(up / m_moveStopProfit);
								if(nowStep - itB->open.stopProfit >= 2)
									itB->open.stopProfit = nowStep - 1;
							}
						}
						else
						{
							double up = (itB->open.price - m_curTickData.last) / itB->open.price;
							if(up / m_moveStopProfit >= 2)
								itB->open.stopProfit = 1;
						}
					}
				}
			}
		}
		++itB;
	}
}

void COKExFuturesDlg::OnLoginSuccess()
{
	OKEX_WEB_SOCKET->API_FuturesOrderInfo(true, m_strCoinType, m_strFuturesCycle);
	OKEX_WEB_SOCKET->API_FuturesAccountInfoByCurrency(true, m_strCoinType);
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
			int bullCount = 0;
			int bearCount = 0;
			while(itBegin != itEnd)
			{
				if(itBegin->open.orderID != "" && itBegin->open.tradeType == eFuturesTradeType_OpenBull)
					bullCount++;
				else if(itBegin->open.orderID != "" && itBegin->open.tradeType == eFuturesTradeType_OpenBear)
					bearCount++;
				if(bullCount + bearCount >= m_nMaxDirTradeCnt)
					break;
				itBegin++;
			}
			if(bullCount >= m_nMaxDirTradeCnt)
				return false;
		}
		break;
	case eFuturesTradeType_OpenBear:
		{
			if((int)m_listTradePairInfo.size() >= m_nMaxTradeCnt)
				return false;
			std::list<SFuturesTradePairInfo>::reverse_iterator itBegin = m_listTradePairInfo.rbegin();
			std::list<SFuturesTradePairInfo>::reverse_iterator itEnd = m_listTradePairInfo.rend();
			int bullCount = 0;
			int bearCount = 0;
			while(itBegin != itEnd)
			{
				if(itBegin->open.orderID != "" && itBegin->open.tradeType == eFuturesTradeType_OpenBull)
					bullCount++;
				else if(itBegin->open.orderID != "" && itBegin->open.tradeType == eFuturesTradeType_OpenBear)
					bearCount++;
				if(bullCount + bearCount >= m_nMaxDirTradeCnt)
					break;
				itBegin++;
			}
			if(bearCount >= m_nMaxDirTradeCnt)
				return false;
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
			return;
		}
		else if(itB->close.strClientOrderID == clientOrderID && itB->close.waitClientOrderIDTime)
		{
			itB->close.Reset();
			return;
		}
		++itB;
	}
}

void COKExFuturesDlg::OnTradeSuccess(std::string& clientOrderID, std::string& serverOrderID)
{
	std::list<SFuturesTradePairInfo>::iterator itB = m_listTradePairInfo.begin();
	std::list<SFuturesTradePairInfo>::iterator itE = m_listTradePairInfo.end();
	while(itB != itE)
	{
		if(itB->open.strClientOrderID == clientOrderID)
		{
			itB->open.orderID = serverOrderID;
			itB->open.waitClientOrderIDTime = 0;
			OKEX_HTTP->API_FuturesOrderInfo(m_strCoinType, m_strFuturesCycle, serverOrderID);
			break;
		}
		if(itB->close.strClientOrderID == clientOrderID)
		{
			itB->close.orderID = serverOrderID;
			itB->close.waitClientOrderIDTime = 0;
			OKEX_HTTP->API_FuturesOrderInfo(m_strCoinType, m_strFuturesCycle, serverOrderID);
			break;
		}
		++itB;
	}
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
			itB->open.status = info.status;
			itB->open.tradeType = info.tradeType;
			itB->open.size = info.size;
			break;
		}
		else if(info.tradeType == itB->close.tradeType && itB->close.orderID == info.orderID)
		{
			itB->close.timeStamp = info.timeStamp;
			itB->close.filledQTY = info.filledQTY;
			itB->close.price = info.price;
			itB->close.status = info.status;
			itB->close.tradeType = info.tradeType;
			itB->close.size = info.size;
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
			szFormat.Format("%s", CFuncCommon::Double2String(itB->open.price+DOUBLE_PRECISION, m_nPriceDecimal).c_str());
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
				double baozhengjin = (sizePrice*count/itB->open.price)/m_nLeverage;
				if(itB->open.tradeType == eFuturesTradeType_OpenBull)
				{
					double calcuPrice = m_curTickData.last;
					if(itB->close.orderID != "" && itB->close.status == "2")
						calcuPrice = itB->close.price;
					if(calcuPrice >= itB->open.price)
					{
						double profit = (calcuPrice-itB->open.price)/itB->open.price*m_nLeverage*baozhengjin;
						totalProfit += profit;
						szFormat.Format("%s", CFuncCommon::Double2String(profit+DOUBLE_PRECISION, 5).c_str());
						m_listCtrlOrderOpen.SetItemText(i, 6, szFormat.GetBuffer());
					}
					else
					{
						double profit = (itB->open.price-calcuPrice)/itB->open.price*m_nLeverage*baozhengjin;
						totalProfit -= profit;
						szFormat.Format("-%s", CFuncCommon::Double2String(profit+DOUBLE_PRECISION, 5).c_str());
						m_listCtrlOrderOpen.SetItemText(i, 6, szFormat.GetBuffer());
					}

				}
				else if(itB->open.tradeType == eFuturesTradeType_OpenBear)
				{
					double calcuPrice = m_curTickData.last;
					if(itB->close.orderID != "" && itB->close.status == "2")
						calcuPrice = itB->close.price;
					if(calcuPrice <= itB->open.price)
					{
						double profit = (itB->open.price-calcuPrice)/itB->open.price*m_nLeverage*baozhengjin;
						totalProfit += profit;
						szFormat.Format("%s", CFuncCommon::Double2String(profit+DOUBLE_PRECISION, 5).c_str());
						m_listCtrlOrderOpen.SetItemText(i, 6, szFormat.GetBuffer());
					}
					else
					{
						double profit = (calcuPrice-itB->open.price)/itB->open.price*m_nLeverage*baozhengjin;
						totalProfit -= profit;
						szFormat.Format("-%s", CFuncCommon::Double2String(profit+DOUBLE_PRECISION, 5).c_str());
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
			m_listCtrlOrderClose.InsertItem(i, "");
			szFormat.Format("%s", CFuncCommon::Double2String(itB->close.price+DOUBLE_PRECISION, m_nPriceDecimal).c_str());
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
	strTemp = m_config.get("futures", "futuresCycle", "");
	m_editFuturesCycle.SetWindowText(strTemp.c_str());

	strTemp = m_config.get("futures", "futuresTradeSize", "");
	m_editFuturesTradeSize.SetWindowText(strTemp.c_str());

	strTemp = m_config.get("futures", "leverage", "");
	if(strTemp == "10")
		m_combLeverage.SetCurSel(0);
	else if(strTemp == "20")
		m_combLeverage.SetCurSel(1);

	strTemp = m_config.get("futures", "stopLoss", "");
	m_editStopLoss.SetWindowText(strTemp.c_str());

	strTemp = m_config.get("futures", "moveStopProfit", "");
	m_editMoveStopProfit.SetWindowText(strTemp.c_str());
	
	strTemp = m_config.get("futures", "maxTradeCnt", "");
	m_editMaxTradeCnt.SetWindowText(strTemp.c_str());
	
	strTemp = m_config.get("futures", "maxDirTradeCnt", "");
	m_editMaxDirTradeCnt.SetWindowText(strTemp.c_str());
}

bool COKExFuturesDlg::__SaveConfigCtrl()
{
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
	m_strCoinType = strCoinType.GetBuffer();
	m_strFuturesCycle = strFuturesCycle.GetBuffer();
	m_strFuturesTradeSize = strFuturesTradeSize.GetBuffer();
	m_strLeverage = strLeverage.GetBuffer();
	m_nLeverage = stoi(m_strLeverage);
	m_stopLoss = stod(strStopLoss.GetBuffer());
	m_moveStopProfit = stod(strStopProfit.GetBuffer());
	m_nMaxTradeCnt = stoi(strMaxTradeCnt.GetBuffer());
	m_nMaxDirTradeCnt = stoi(strMaxDirTradeCnt.GetBuffer());
	m_config.set_value("futures", "coinType", m_strCoinType.c_str());
	m_config.set_value("futures", "futuresCycle", m_strFuturesCycle.c_str());
	m_config.set_value("futures", "futuresTradeSize", m_strFuturesTradeSize.c_str());
	m_config.set_value("futures", "leverage", m_strLeverage.c_str());
	m_config.set_value("futures", "stopLoss", strStopLoss.GetBuffer());
	m_config.set_value("futures", "moveStopProfit", strStopProfit.GetBuffer());
	m_config.set_value("futures", "maxTradeCnt", strMaxTradeCnt.GetBuffer());
	m_config.set_value("futures", "maxDirTradeCnt", strMaxDirTradeCnt.GetBuffer());
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
	OKEX_HTTP->API_GetFuturesSomeKline(false, m_strCoinType, m_strFuturesCycle, strKlineCycle, strFrom, strTo, &resInfo);
	Json::Value& retObj = resInfo.retObj;
	if(retObj.isArray())
	{
		for(int i = retObj.size()-1; i>=0; --i)
		{
			SKlineData data;
			data.time = retObj[i][0].asInt64()/1000;
			data.lowPrice = retObj[i][1].asDouble();
			data.highPrice = retObj[i][2].asDouble();
			data.openPrice = retObj[i][3].asDouble();
			data.closePrice = retObj[i][4].asDouble();
			data.volume = retObj[i][5].asInt();
			data.volumeByCurrency = retObj[i][6].asDouble();
			CString strlocalLog;
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
