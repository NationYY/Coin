
// CoinDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Coin.h"
#include "CoinDlg.h"
#include "afxdialogex.h"
#include <string>
#include "exchange/bw/bw_exchange.h"
#include "exchange/coinex/coinex_exchange.h"
#include <MMSystem.h>
#include "common/func_common.h"
#include <clib/lib/file/file_util.h>
#include "log/local_log.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
// 用于应用程序“关于”菜单项的 CAboutDlg 对话框
CCoinDlg* g_pCoinDlg = NULL;
const int CHECK_STATE_RATE = 5;
int g_trade_pair_index = 1;
CExchange* pExchange = NULL;
CExchange* pReferenceExchange = NULL;
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


// CCoinDlg 对话框



CCoinDlg::CCoinDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CCoinDlg::IDD, pParent)
	, m_tradeFrequency(0)
	, m_webSokectFailTimes(0)
	, m_bIsRun(false)
	, m_strTradeVolume(_T(""))
	, m_tradePriceDecimal(0)
	, m_marketType(eMarketType_Max)
	, m_tLastGetEntrustDepth(0)
	, m_successCreateTradeCnt(0)
	, m_succesCancelTradeCnt(0)
	, m_allFinishTradeCnt(0)
	, m_failCreateTradeCnt(0)
	, m_priceCheckValue(0)
	, m_tLastGetReferenceExecutedOrder(0)
	, m_bUseRreferenceCheck(true)
	, m_tLastReconnectHightSpeed(0)
	, m_maxBuyOrderCnt(0)
	, m_maxSellOrderCnt(0)
	, m_tLastCheckBalanceTime(0)
	, m_fSpecPrice(0)
	, m_nextTradeFrequency(0)
	, m_dwLastUpdateTime(0)
	, m_minPrice(0)
	, m_maxPrice(0)
{
	g_pCoinDlg = this;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCoinDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_listBalance);
	DDX_Control(pDX, IDC_LIST2, m_listCtrlEntrustDepth);
	DDX_Control(pDX, IDC_COMBO1, m_cbMarketType);
	DDX_Text(pDX, IDC_EDIT2, m_tradeFrequency);
	DDX_Control(pDX, IDC_STATIC_UPDATE_TIME, m_staticUpdateTime);
	DDX_Control(pDX, IDC_RADIO1, m_btnHightSpeed);
	DDX_Control(pDX, IDC_RADIO2, m_btnNormalSpeed);
	DDX_Text(pDX, IDC_EDIT1, m_strTradeVolume);
	DDX_Control(pDX, IDC_LIST3, m_listFinishOrder);
	DDX_Text(pDX, IDC_EDIT4, m_tradePriceDecimal);
	DDX_Control(pDX, IDC_LIST4, m_listLog);
	DDX_Control(pDX, IDC_STATIC9, m_staticSuccessCreateTradeCnt);
	DDX_Control(pDX, IDC_STATIC11, m_staticSuccesCancelTradeCnt);
	DDX_Control(pDX, IDC_STATIC13, m_staticAllFinishTradeCnt);
	DDX_Control(pDX, IDC_STATIC15, m_staticFailCreateTradeCnt);
	DDX_Text(pDX, IDC_EDIT3, m_priceCheckValue);
	DDX_Text(pDX, IDC_EDIT5, m_maxBuyOrderCnt);
	DDX_Text(pDX, IDC_EDIT6, m_maxSellOrderCnt);
	DDX_Control(pDX, IDC_EDIT3, m_editPriceCheck);
	DDX_Control(pDX, IDC_STATIC10, m_staticNowBuyTradeCnt);
	DDX_Control(pDX, IDC_STATIC12, m_staticNowSellTradeCnt);
	DDX_Text(pDX, IDC_EDIT7, m_fSpecPrice);
	DDX_Text(pDX, IDC_EDIT8, m_minPrice);
	DDX_Text(pDX, IDC_EDIT9, m_maxPrice);
}

BEGIN_MESSAGE_MAP(CCoinDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CCoinDlg::OnBnClickedButtonBegin)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_RADIO1, &CCoinDlg::OnBnClickedRadioHightSpeed)
	ON_BN_CLICKED(IDC_RADIO2, &CCoinDlg::OnBnClickedRadioNormalSpeed)
	ON_BN_CLICKED(IDC_BUTTON2, &CCoinDlg::OnBnClickedButtonStop)
END_MESSAGE_MAP()


// CCoinDlg 消息处理程序
void reference_websocket_callbak_open(const char* szExchangeName)
{
	g_pCoinDlg->AddLog("第三方参考行情连接成功!");
	if(g_pCoinDlg->m_bIsRun && g_pCoinDlg->m_bUseRreferenceCheck)
	{
		pReferenceExchange->GetWebSocket()->API_LatestExecutedOrder(g_pCoinDlg->m_marketType);
	}
}
void reference_websocket_callbak_close(const char* szExchangeName)
{
	g_pCoinDlg->AddLog("第三方参考行情断开连接!");
}
void reference_websocket_callbak_fail(const char* szExchangeName)
{
	g_pCoinDlg->AddLog("第三方参考行情连接失败!");
}

void local_websocket_callbak_open(const char* szExchangeName)
{
	g_pCoinDlg->m_btnHightSpeed.SetWindowTextA("高速行情(可用)");

	if(g_pCoinDlg->m_bIsRun)
	{
		if(g_pCoinDlg->m_btnHightSpeed.GetCheck())
		{
			if(pExchange->GetWebSocket())
				pExchange->GetWebSocket()->API_EntrustDepth(g_pCoinDlg->m_marketType, 5, true);
		}
	}
	//g_pCoinDlg->AddLog("行情连接成功!");
}
void local_websocket_callbak_close(const char* szExchangeName)
{
	g_pCoinDlg->m_btnHightSpeed.SetWindowTextA("高速行情(连接中断, 重连中)");
	//g_pCoinDlg->AddLog("行情断开连接!");
}
void local_websocket_callbak_fail(const char* szExchangeName)
{
	g_pCoinDlg->m_webSokectFailTimes++;
	CString temp;
	temp.Format("高速行情(连接失败%d, 再次尝试中)", g_pCoinDlg->m_webSokectFailTimes);
	g_pCoinDlg->m_btnHightSpeed.SetWindowTextA(temp.GetBuffer());
	//g_pCoinDlg->AddLog("行情连接失败!");
}

void local_websocket_callbak_message(eWebsocketAPIType apiType, const char* szExchangeName, Json::Value& retObj, const std::string& strRet)
{
	switch(apiType)
	{
	case eWebsocketAPIType_EntrustDepth:
		g_pCoinDlg->m_tLastGetEntrustDepth = time(NULL);
		g_pCoinDlg->UpdateEntrustDepth();
		break;
	}
	OutputDebugString("[main exchange]");
	OutputDebugString(strRet.c_str());
	OutputDebugString("\n");
}

void reference_websocket_callbak_message(eWebsocketAPIType apiType, const char* szExchangeName, Json::Value& retObj, const std::string& strRet)
{
	switch(apiType)
	{
	case eWebsocketAPIType_LatestExecutedOrder:
		g_pCoinDlg->m_tLastGetReferenceExecutedOrder = time(NULL);
		g_pCoinDlg->UpdateExecutedOrderPrice();
		break;
	}
	//OutputDebugString("[reference exchange]");
	//OutputDebugString(strRet.c_str());
	//OutputDebugString("\n");
}

void local_http_callbak_message(eHttpAPIType apiType, Json::Value& retObj, const std::string& strRet, int customData, std::string strCustomData)
{
	switch(apiType)
	{
	case eHttpAPIType_Balance:
		g_pCoinDlg->UpdateBalance();
		g_pCoinDlg->SetTimer(eTimerType_Balance, 5000, NULL);
		return;
		break;
	case eHttpAPIType_Ticker:
		break;
	case eHttpAPIType_EntrustDepth:
		g_pCoinDlg->m_tLastGetEntrustDepth = time(NULL);
		g_pCoinDlg->UpdateEntrustDepth();
		return;
		break;
	case eHttpAPIType_Trade:
		{
			std::string code = "-1";
			std::string errorMessage = "";
			if(retObj["resMsg"]["code"].isString())
				code = retObj["resMsg"]["code"].asString();
			if(retObj["resMsg"]["message"].isString())
				errorMessage = retObj["resMsg"]["message"].asString();
			if(code != "1")
				g_pCoinDlg->AddLog("下单失败,code=%s,message=%s", code.c_str(), errorMessage.c_str());
		    std::map<int, CCoinDlg::STradePair>::iterator it = g_pCoinDlg->m_mapTradePairs.find(customData);
			if(it != g_pCoinDlg->m_mapTradePairs.end())
			{
				if(code == "1")
				{
					g_pCoinDlg->m_successCreateTradeCnt++;
					g_pCoinDlg->UpdateTradeReport();
					std::string id = retObj["datas"]["entrustId"].asString();

					if(it->second.id1 == "NULL")
						it->second.id1 = id;
					else if(it->second.id1 == "")
					{
						it->second.id2 = id;
						g_pCoinDlg->AddLog("组合下单失败,提交撤单2[%s]!", it->second.id2.c_str());
						pExchange->GetHttp()->API_CancelTrade(g_pCoinDlg->m_marketType, it->second.id2, it->second.id2);
					}
					else
					{
						g_pCoinDlg->m_mapTradePairs.erase(it);
						if(pExchange && pExchange->GetHttp())
							pExchange->GetHttp()->API_Balance();
						g_pCoinDlg->m_tLastCheckBalanceTime = time(NULL);
						g_pCoinDlg->KillTimer(eTimerType_Balance);
					}
				}
				else
				{
					g_pCoinDlg->m_failCreateTradeCnt++;
					if(it->second.id1 == "NULL")
						it->second.id1 = "";
					else if(it->second.id1 == "")
						g_pCoinDlg->m_mapTradePairs.erase(it);
					else if(it->second.id1 == "NO COIN")
						g_pCoinDlg->m_mapTradePairs.erase(it);
					else
					{
						it->second.id2 = "";
						g_pCoinDlg->m_successCreateTradeCnt--;
						g_pCoinDlg->AddLog("组合下单失败,提交撤单3[%s]!", it->second.id1.c_str());
						pExchange->GetHttp()->API_CancelTrade(g_pCoinDlg->m_marketType, it->second.id1, it->second.id1);
					}
					g_pCoinDlg->UpdateTradeReport();
				}
			}
		}
		break;
	case eHttpAPIType_TradeOrderState:
		g_pCoinDlg->m_allFinishTradeCnt += pExchange->GetDataCenter()->m_listAllFinishOrder.size();
		g_pCoinDlg->UpdateTradeReport();
		g_pCoinDlg->UpdateFinishOrder();
		break;
	case eHttpAPIType_CancelTrade:
		{
			if(retObj["resMsg"]["code"].isString() && retObj["resMsg"]["code"].asString() == "1")
			{
				g_pCoinDlg->m_succesCancelTradeCnt++;
				g_pCoinDlg->UpdateTradeReport();
				g_pCoinDlg->AddLog("撤单成功[%s]!", strCustomData.c_str());
				std::map<int, CCoinDlg::STradePair>::iterator itB = g_pCoinDlg->m_mapTradePairs.begin();
				std::map<int, CCoinDlg::STradePair>::iterator itE = g_pCoinDlg->m_mapTradePairs.end();
				while(itB != itE)
				{
					if(itB->second.id1 == "" && itB->second.id2 == strCustomData)
					{
						g_pCoinDlg->m_mapTradePairs.erase(itB);
						break;
					}
					else if(itB->second.id2 == "" && itB->second.id1 == strCustomData)
					{
						g_pCoinDlg->m_mapTradePairs.erase(itB);
						break;
					}
					++itB;
				}
			}
			else
			{
				if(retObj["resMsg"]["message"].isString())
					g_pCoinDlg->AddLog("撤单失败[%s]! id[%s]", retObj["resMsg"]["message"].asString().c_str(), strCustomData.c_str());
				else
					g_pCoinDlg->AddLog("撤单失败[%s]!", strCustomData.c_str());
			}
		}
		break;
	default:
		break;
	}
	OutputDebugString(strRet.c_str());
	OutputDebugString("\n");
};


/*
void CALLBACK UpdateFunc(UINT wTimerID, UINT msg, DWORD dwUser, DWORD dwl, DWORD dw2)
{
	CCoinDlg* pDlg = (CCoinDlg*)dwUser;
}*/

BOOL CCoinDlg::OnInitDialog()
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
	const char* id = m_config.get("bw", "AccessKey", "");
	const char* key = m_config.get("bw", "SecretKey", "");
	m_accessKey = id;
	m_secretKey = key;
	// TODO:  在此添加额外的初始化代码
	pExchange = new CBWExchange(id, key);
	pExchange->SetHttpCallBackMessage(local_http_callbak_message);
	pExchange->SetWebSocketCallBackOpen(local_websocket_callbak_open);
	pExchange->SetWebSocketCallBackClose(local_websocket_callbak_close);
	pExchange->SetWebSocketCallBackFail(local_websocket_callbak_fail);
	pExchange->SetWebSocketCallBackMessage(local_websocket_callbak_message);
	pExchange->Run();

	pReferenceExchange = new CCoinexExchange("", "");
	pReferenceExchange->SetWebSocketCallBackOpen(reference_websocket_callbak_open);
	pReferenceExchange->SetWebSocketCallBackClose(reference_websocket_callbak_close);
	pReferenceExchange->SetWebSocketCallBackFail(reference_websocket_callbak_fail);
	pReferenceExchange->SetWebSocketCallBackMessage(reference_websocket_callbak_message);
	pReferenceExchange->Run(true, 0, 0);
	/*TIMECAPS   tc;
	UINT wTimerRes;
	if(timeGetDevCaps(&tc, sizeof(TIMECAPS)) != TIMERR_NOERROR)//向机器申请一个多媒体定时器       
		return FALSE;
	wTimerRes = min(max(tc.wPeriodMin, 1), tc.wPeriodMax);
	timeBeginPeriod(wTimerRes);
	MMRESULT g_wTimerID = timeSetEvent(6, wTimerRes, (LPTIMECALLBACK)UpdateFunc, (DWORD)this, TIME_PERIODIC);
	if(g_wTimerID == 0)
		return FALSE;*/
	
	m_btnHightSpeed.SetCheck(1);
	m_btnNormalSpeed.ShowWindow(0);
	m_listBalance.InsertColumn(0, "币种", LVCFMT_CENTER, 40);
	m_listBalance.InsertColumn(1, "余额", LVCFMT_CENTER, 100);
	m_listBalance.InsertColumn(2, "可用", LVCFMT_CENTER, 100);
	m_listBalance.InsertColumn(3, "冻结", LVCFMT_CENTER, 100);
	m_listBalance.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	
	m_listCtrlEntrustDepth.InsertColumn(0, "", LVCFMT_CENTER, 30);
	m_listCtrlEntrustDepth.InsertColumn(1, "价", LVCFMT_CENTER, 100);
	m_listCtrlEntrustDepth.InsertColumn(2, "量", LVCFMT_CENTER, 100);
	m_listCtrlEntrustDepth.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

	
	m_listFinishOrder.InsertColumn(0, "时间", LVCFMT_CENTER, 100);
	m_listFinishOrder.InsertColumn(1, "价", LVCFMT_CENTER, 100);
	m_listFinishOrder.InsertColumn(2, "量", LVCFMT_CENTER, 100);
	m_listFinishOrder.InsertColumn(3, "类型", LVCFMT_CENTER, 50);
	m_listFinishOrder.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	if(pExchange)
	{
		const std::list<eMarketType>& listMarket = pExchange->GetSupportMarket();
		std::list<eMarketType>::const_iterator itB = listMarket.begin();
		std::list<eMarketType>::const_iterator itE = listMarket.end();
		int index = 0;
		while(itB != itE)
		{
			switch(*itB)
			{
			case eMarketType_ETH_BTC:
				m_cbMarketType.InsertString(index, "ETH_BTC");
				break;
			case eMarketType_ETH_USDT:
				m_cbMarketType.InsertString(index, "ETH_USDT");
				break;
			case eMarketType_BTC_USDT:
				m_cbMarketType.InsertString(index, "BTC_USDT");
				break;
			case eMarketType_BWB_USDT:
				m_cbMarketType.InsertString(index, "BWB_USDT");
				break;
			case eMarketType_BWB_BTC:
				m_cbMarketType.InsertString(index, "BWB_BTC");
				break;
			case eMarketType_BWB_ETH:
				m_cbMarketType.InsertString(index, "BWB_ETH");
				break;
			case eMarketType_BWB_QC:
				m_cbMarketType.InsertString(index, "BWB_QC");
				break;
			case eMarketType_USDT_QC:
				m_cbMarketType.InsertString(index, "USDT_QC");
				break;
				
			default:
				break;
			}
			m_cbMarketType.SetItemData(index, *itB);
			++itB;
		}
	}

	SetTimer(eTimerType_APIUpdate, 1, NULL);
	SetTimer(eTimerType_Balance, 5000, NULL);
	SetTimer(eTimerType_Ping, 5000, NULL);
	

	clib::string log_path = "log/";
	bool bRet = clib::file_util::mkfiledir(log_path.c_str(), true);

	LocalLogger& localLogger = LocalLogger::GetInstance();
	localLogger.SetBatchMode(true);
	localLogger.SetLogPath(log_path.c_str());
	localLogger.Start();


	int sel = m_config.get_int("bw", "market_select", -1);
	if(sel != -1)
		m_cbMarketType.SetCurSel(sel);
	m_strTradeVolume = m_config.get("bw", "trade_volume", "");
	m_tradeFrequency = m_config.get_int("bw", "trade_frequency", 0);
	m_tradePriceDecimal = m_config.get_int("bw", "trade_price_decimal", 0);
	m_maxBuyOrderCnt = m_config.get_int("bw", "max_buy_order", 0);
	m_maxSellOrderCnt = m_config.get_int("bw", "max_sell_order", 0);
	UpdateData(FALSE);
	m_editPriceCheck.SetWindowTextA(m_config.get("bw", "price_check_value", ""));
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CCoinDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CCoinDlg::OnPaint()
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
HCURSOR CCoinDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CCoinDlg::OnBnClickedButtonBegin()
{
	if(m_bIsRun)
		return;
	UpdateData(TRUE);
	if(m_accessKey == "" || m_secretKey == "")
	{
		AfxMessageBox("accessKey或secretKey没填写");
		return;
	}
	int sel = m_cbMarketType.GetCurSel();
	if(CB_ERR == sel)
	{
		AfxMessageBox("请选择交易币对");
		return;
	}
	DWORD type = m_cbMarketType.GetItemData(sel);
	if(type < 0 || type >= eMarketType_Max)
	{
		AfxMessageBox("交易币对错误");
		return;
	}
	CString temp;
	m_cbMarketType.GetWindowTextA(temp);
	//char szBuffer[128];
	//_snprintf(szBuffer, 128, "%d", pExchange->GetMarketID((eMarketType)type));
	//m_strMarketType = szBuffer;
	m_strMarketType = temp.GetBuffer();
	m_marketType = (eMarketType)type;
	if(m_strTradeVolume == "")
	{
		AfxMessageBox("请填写单笔交易量");
		return;
	}
	if(m_tradeFrequency == 0)
	{
		AfxMessageBox("请填写交易频率");
		return;
	}
	if(m_tradeFrequency < 1000)
	{
		int ret = AfxMessageBox("交易频率低于1000,请确认是否填写正确", MB_YESNO);
		if(ret == 7)
			return;
	}
	if(m_tradePriceDecimal == 0)
	{
		AfxMessageBox("请填写价格小数位精度");
		return;
	}
	if(m_btnHightSpeed.GetCheck() && !pExchange->GetWebSocket()->IsConnect())
	{
		AfxMessageBox("请等待与交易所服务器建立连接!");
		return;
	}
	if(CFuncCommon::CheckEqual(m_priceCheckValue, 0))
	{
		AfxMessageBox("请填写参考价格暂停交易的比例值");
		return;
	}
	m_priceCheckValue /= 100;
	if(m_btnHightSpeed.GetCheck())
	{
		if(pExchange->GetWebSocket())
			pExchange->GetWebSocket()->API_EntrustDepth((eMarketType)type, 5, true);
	}
	else
	{
		SetTimer(eTimerType_EntrustDepth, 500, NULL);
	}
	m_bUseRreferenceCheck = pReferenceExchange->IsSupportMarket(m_marketType);
	if(m_bUseRreferenceCheck)
		pReferenceExchange->GetWebSocket()->API_LatestExecutedOrder((eMarketType)type);
	//SetTimer(eTimerType_Trade, m_tradeFrequency, NULL);
	SetTimer(eTimerType_TradeOrderState, 1000, NULL);


	m_nextTradeFrequency = 0;

	m_config.set_value("bw", "market_select", CFuncCommon::ToString(sel));
	m_config.set_value("bw", "trade_volume", m_strTradeVolume.GetBuffer());
	m_config.set_value("bw", "trade_frequency", CFuncCommon::ToString(m_tradeFrequency));
	m_config.set_value("bw", "trade_price_decimal", CFuncCommon::ToString(m_tradePriceDecimal));
	m_editPriceCheck.GetWindowTextA(temp);
	m_config.set_value("bw", "price_check_value", temp.GetBuffer());
	m_config.set_value("bw", "max_buy_order", CFuncCommon::ToString(m_maxBuyOrderCnt));
	m_config.set_value("bw", "max_sell_order", CFuncCommon::ToString(m_maxSellOrderCnt));
	m_config.save("./config.ini");
	m_bIsRun = true;
	// TODO:  在此添加控件通知处理程序代码
}

void CCoinDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	switch(nIDEvent)
	{
	case eTimerType_APIUpdate:
		{
			time_t tNow = time(NULL);
			if(m_btnHightSpeed.GetCheck() && m_tLastGetEntrustDepth && tNow - m_tLastGetEntrustDepth > 20000 && tNow - m_tLastReconnectHightSpeed > 10)
			{
				g_pCoinDlg->AddLog("第三方参考行情长时间未同步,尝试重连!");
				m_tLastReconnectHightSpeed = tNow;
				pExchange->GetWebSocket()->Close();
				pExchange->GetWebSocket()->Run();
			}
			LocalLogger::GetInstancePt()->SwapFront2Middle();
			if(pExchange)
				pExchange->Update();
			if(pReferenceExchange)
				pReferenceExchange->Update();
			std::map<std::string, CDataCenter::SOrderInfo>::iterator itB = pExchange->GetDataCenter()->m_mapTradeOrderID.begin();
			std::map<std::string, CDataCenter::SOrderInfo>::iterator itE = pExchange->GetDataCenter()->m_mapTradeOrderID.end();
			int cancelCheckTime = m_tradeFrequency/1000 * 5;
			cancelCheckTime = max(cancelCheckTime, 90);
			while(itB != itE)
			{
				if(itB->second.cancelTimes == 10)
					itB = pExchange->GetDataCenter()->m_mapTradeOrderID.erase(itB);
				else
				{
					if(itB->second.tradeType == eTradeType_buy)
					{
						if(tNow - itB->second.addTime >= 40 && tNow - itB->second.lastCancelTime > 30)
						{
							itB->second.cancelTimes++;
							itB->second.lastCancelTime = tNow;
							g_pCoinDlg->AddLog("超时未成交,提交撤单1[%s]!", itB->first.c_str());
							pExchange->GetHttp()->API_CancelTrade(m_marketType, itB->first, itB->first);
						}
					}
					else if(itB->second.tradeType == eTradeType_sell)
					{
						if(tNow - itB->second.addTime >= cancelCheckTime && tNow - itB->second.lastCancelTime > 30)
						{
							itB->second.cancelTimes++;
							itB->second.lastCancelTime = tNow;
							g_pCoinDlg->AddLog("超时未成交,提交撤单1[%s]!", itB->first.c_str());
							pExchange->GetHttp()->API_CancelTrade(m_marketType, itB->first, itB->first);
						}
					}
					
					++itB;
				}

			}

			CDataCenter* pDataCenter = pExchange->GetDataCenter();
			std::map<std::string, CDataCenter::SOrderInfo>::iterator _itB = pDataCenter->m_mapTradeOrderID.begin();
			std::map<std::string, CDataCenter::SOrderInfo>::iterator _itE = pDataCenter->m_mapTradeOrderID.end();
			int buyCnt = 0;
			int sellCnt = 0;
			while(_itB != _itE)
			{
				if(_itB->second.tradeType == eTradeType_buy)
					buyCnt++;
				else if(_itB->second.tradeType == eTradeType_sell)
					sellCnt++;
				++_itB;
			}
			CString temp;
			temp.Format("%d", buyCnt);
			m_staticNowBuyTradeCnt.SetWindowText(temp.GetBuffer());
			temp.Format("%d", sellCnt);
			m_staticNowSellTradeCnt.SetWindowText(temp.GetBuffer());

			if(m_bUseRreferenceCheck && m_tLastGetReferenceExecutedOrder && tNow - m_tLastGetReferenceExecutedOrder > 30000)
			{
				OnBnClickedButtonStop();
				pExchange->GetWebSocket()->Close();
				pExchange->Run();
				while(!pExchange->GetWebSocket()->IsConnect())
				{

				}
				OnBnClickedButtonBegin();
			}
			DWORD dwNow = GetTickCount();
			if(dwNow < m_dwLastUpdateTime || dwNow - m_dwLastUpdateTime >= m_nextTradeFrequency && m_bIsRun)
			{
				TradeLogic();
				m_dwLastUpdateTime = dwNow;
				int min = m_tradeFrequency*0.3;
				int _rand = (rand()%(m_tradeFrequency - min)) + min;
				m_nextTradeFrequency = _rand;
			}
		}
		break;
	case eTimerType_Balance:
		{
			time_t tNow = time(NULL);
			if(tNow - m_tLastCheckBalanceTime > 4)
			{
				if(pExchange && pExchange->GetHttp())
					pExchange->GetHttp()->API_Balance();
				m_tLastCheckBalanceTime = tNow;
				KillTimer(eTimerType_Balance);
			}
			
		}
		break;
	case eTimerType_EntrustDepth:
		{
			if(pExchange && pExchange->GetHttp())
				pExchange->GetHttp()->API_EntrustDepth(m_marketType);
		}
		break;
	case eTimerType_Trade:
		{
			TradeLogic();
		}
		break;
	case eTimerType_TradeOrderState:
		{
			time_t tNow = time(NULL);
			CDataCenter* pDataCenter = pExchange->GetDataCenter();
			std::map<std::string, CDataCenter::SOrderInfo>::iterator itB = pDataCenter->m_mapTradeOrderID.begin();
			std::map<std::string, CDataCenter::SOrderInfo>::iterator itE = pDataCenter->m_mapTradeOrderID.end();
			int sendCnt = 0;
			bool bFound = false;
			while(itB != itE)
			{
				if(itB->second.checkIndex != pDataCenter->m_orderCheckIndex)
				{
					//if(itB->second.serverCreatDate != 0 || (tNow - itB->second.addTime > 2))
					{
						bFound = true;
						itB->second.checkIndex = pDataCenter->m_orderCheckIndex;
						if(pExchange->GetHttp())
							pExchange->GetHttp()->API_GetTradeOrderState(m_marketType, itB->first);
						if(++sendCnt > CHECK_STATE_RATE)
							break;;
					}
				}
				++itB;
			}
			if(!bFound)
				pDataCenter->m_orderCheckIndex++;
		}
		break;
	case eTimerType_Ping:
		{
			pExchange->GetWebSocket()->Ping();
			pReferenceExchange->GetWebSocket()->Ping();
		}
		break;
	default:
		break;
	}
	
	CDialogEx::OnTimer(nIDEvent);
}

void CCoinDlg::UpdateBalance()
{
	m_listBalance.DeleteAllItems();
	CDataCenter* pDataCenter = pExchange->GetDataCenter();
	std::map<std::string, SBalanceInfo>::iterator itB = pDataCenter->m_mapBalanceInfo.begin();
	std::map<std::string, SBalanceInfo>::iterator itE = pDataCenter->m_mapBalanceInfo.end();
	int index = 0;
	CString szFormat;
	while(itB != itE)
	{
		m_listBalance.InsertItem(index, "");
		szFormat.Format("%s", itB->first.c_str());
		m_listBalance.SetItemText(index, 0, szFormat);
		szFormat.Format("%lf", itB->second.total);
		m_listBalance.SetItemText(index, 1, szFormat);
		szFormat.Format("%lf", itB->second.balance);
		m_listBalance.SetItemText(index, 2, szFormat);
		szFormat.Format("%lf", itB->second.freeze);
		m_listBalance.SetItemText(index, 3, szFormat);
		index++;
		++itB;
	}
}

void CCoinDlg::UpdateEntrustDepth()
{
	const int showLines = 5;
	m_listCtrlEntrustDepth.DeleteAllItems();
	CDataCenter* pDataCenter = pExchange->GetDataCenter();
	std::map<std::string, std::string>& mapBuyEntrustDepth = pDataCenter->m_mapEntrustDepth[m_strMarketType].mapBuyEntrustDepth;
	std::map<std::string, std::string>& mapSellEntrustDepth = pDataCenter->m_mapEntrustDepth[m_strMarketType].mapSellEntrustDepth;

	int sellLine = min(mapSellEntrustDepth.size(), showLines);
	for(int i = 0; i < sellLine; ++i)
	{
		m_listCtrlEntrustDepth.InsertItem(i, "");
	}
	std::map<std::string, std::string>::iterator itB = mapSellEntrustDepth.begin();
	std::map<std::string, std::string>::iterator itE = mapSellEntrustDepth.end();
	CString szFormat;
	int count = 0;
	while(itB != itE)
	{
		szFormat.Format("%d", count + 1);
		m_listCtrlEntrustDepth.SetItemText(sellLine - 1 - count, 0, szFormat);
		szFormat.Format("%s", itB->first.c_str());
		m_listCtrlEntrustDepth.SetItemText(sellLine - 1 - count, 1, szFormat);
		szFormat.Format("%s", itB->second.c_str());
		m_listCtrlEntrustDepth.SetItemText(sellLine - 1 - count, 2, szFormat);
		if(++count >= sellLine)
			break;
		++itB;
	}
	m_listCtrlEntrustDepth.InsertItem(sellLine, "");
	m_listCtrlEntrustDepth.SetItemText(sellLine, 0, "---");
	m_listCtrlEntrustDepth.SetItemText(sellLine, 1, "-------------");
	m_listCtrlEntrustDepth.SetItemText(sellLine, 2, "-------------");
	int buyLine = min(mapBuyEntrustDepth.size(), showLines);
	for(int i = 0; i < sellLine; ++i)
	{
		m_listCtrlEntrustDepth.InsertItem(sellLine+1+i, "");
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
			m_listCtrlEntrustDepth.SetItemText(sellLine+1+count, 0, _szFormat);
			_szFormat.Format("%s", itE->first.c_str());
			m_listCtrlEntrustDepth.SetItemText(sellLine+1+count, 1, _szFormat);
			_szFormat.Format("%s", itE->second.c_str());
			m_listCtrlEntrustDepth.SetItemText(sellLine+1+count, 2, _szFormat);
			if(++count >= buyLine)
				break;
			itE--;
		}
		if(count < buyLine)
		{
			_szFormat.Format("%d", count + 1);
			m_listCtrlEntrustDepth.SetItemText(sellLine + 1 + count, 0, _szFormat);
			_szFormat.Format("%s", itB->first.c_str());
			m_listCtrlEntrustDepth.SetItemText(sellLine + 1 + count, 1, _szFormat);
			_szFormat.Format("%s", itB->second.c_str());
			m_listCtrlEntrustDepth.SetItemText(sellLine + 1 + count, 2, _szFormat);
		}
	}
	tm* pTM = localtime(&pDataCenter->m_updateEntrustDepthTime);
	std::string price = CFuncCommon::Double2String(pReferenceExchange->GetDataCenter()->m_mapLatestExecutedOrderPrice[""], m_tradePriceDecimal);
	szFormat.Format("更新时间: %d:%d:%d 最新价格:%s", pTM->tm_hour, pTM->tm_min, pTM->tm_sec, price.c_str());
	m_staticUpdateTime.SetWindowTextA(szFormat.GetBuffer());
}

void CCoinDlg::UpdateExecutedOrderPrice()
{
	CString szFormat;
	CDataCenter* pDataCenter = pExchange->GetDataCenter();
	tm* pTM = localtime(&pDataCenter->m_updateEntrustDepthTime);
	std::string price = CFuncCommon::Double2String(pReferenceExchange->GetDataCenter()->m_mapLatestExecutedOrderPrice[""], m_tradePriceDecimal);
	szFormat.Format("更新时间: %d:%d:%d 最新价格:%s", pTM->tm_hour, pTM->tm_min, pTM->tm_sec, price.c_str());
	m_staticUpdateTime.SetWindowTextA(szFormat.GetBuffer());
}

void CCoinDlg::OnBnClickedRadioHightSpeed()
{
	m_btnNormalSpeed.SetCheck(0);
}


void CCoinDlg::OnBnClickedRadioNormalSpeed()
{
	m_btnHightSpeed.SetCheck(0);
}


void CCoinDlg::OnBnClickedButtonStop()
{
	if(m_bIsRun)
	{
		if(m_btnHightSpeed.GetCheck())
		{
			if(pExchange->GetWebSocket())
				pExchange->GetWebSocket()->API_EntrustDepth(m_marketType, 5, false);
		}
		else
		{
			g_pCoinDlg->KillTimer(eTimerType_EntrustDepth);
		}
		//g_pCoinDlg->KillTimer(eTimerType_Trade);
		//g_pCoinDlg->KillTimer(eTimerType_TradeOrderState);
		m_bIsRun = false;
	}
}

void CCoinDlg::UpdateFinishOrder()
{
	CDataCenter* pDataCenter = pExchange->GetDataCenter();
	if(pDataCenter->m_bJustUpdateFinishOrder)
	{
		if(m_listFinishOrder.GetItemCount() > 500)
			m_listFinishOrder.DeleteAllItems();
		pDataCenter->m_bJustUpdateFinishOrder = false;
		std::list<CDataCenter::SFinishOrderInfo>::iterator itB = pDataCenter->m_listAllFinishOrder.begin();
		std::list<CDataCenter::SFinishOrderInfo>::iterator itE = pDataCenter->m_listAllFinishOrder.end();
		CString szTemp;
		while(itB != itE)
		{
			m_listFinishOrder.InsertItem(0, "");
			time_t tTime = itB->time;
			tm* pTM = localtime(&tTime);
			szTemp.Format("%02d-%02d %02d:%02d:%02d", pTM->tm_mon+1, pTM->tm_mday, pTM->tm_hour, pTM->tm_min, pTM->tm_sec);
			m_listFinishOrder.SetItemText(0, 0, szTemp.GetBuffer());
			szTemp.Format("%lf", itB->price);
			m_listFinishOrder.SetItemText(0, 1, szTemp.GetBuffer());
			szTemp.Format("%lf", itB->amount);
			m_listFinishOrder.SetItemText(0, 2, szTemp.GetBuffer());
			if(itB->type == eTradeType_buy)
				m_listFinishOrder.SetItemText(0, 3, "buy");
			else if(itB->type == eTradeType_sell)
				m_listFinishOrder.SetItemText(0, 3, "sell");
			++itB;
		}
		pDataCenter->m_listAllFinishOrder.clear();
	}
}

void CCoinDlg::AddLog(char* szLog, ...)
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

void CCoinDlg::UpdateTradeReport()
{
	CString temp;
	temp.Format("%d", m_successCreateTradeCnt);
	m_staticSuccessCreateTradeCnt.SetWindowText(temp.GetBuffer());
	temp.Format("%d", m_succesCancelTradeCnt);
	m_staticSuccesCancelTradeCnt.SetWindowText(temp.GetBuffer());
	temp.Format("%d", m_allFinishTradeCnt);
	m_staticAllFinishTradeCnt.SetWindowText(temp.GetBuffer());
	temp.Format("%d", m_failCreateTradeCnt);
	m_staticFailCreateTradeCnt.SetWindowText(temp.GetBuffer());
	
}


void CCoinDlg::TradeLogic()
{
	double offset = m_tradeFrequency/1000.0*2.5;
	time_t tNow = time(NULL);
	if(tNow - m_tLastGetEntrustDepth > offset)
		return;
	if(m_bUseRreferenceCheck && tNow - m_tLastGetReferenceExecutedOrder > 30000)
		return;
	CDataCenter* pDataCenter = pExchange->GetDataCenter();
	std::map<std::string, std::string>& mapBuyEntrustDepth = pDataCenter->m_mapEntrustDepth[m_strMarketType].mapBuyEntrustDepth;
	std::map<std::string, std::string>& mapSellEntrustDepth = pDataCenter->m_mapEntrustDepth[m_strMarketType].mapSellEntrustDepth;

	std::map<std::string, CDataCenter::SOrderInfo>::iterator itB = pDataCenter->m_mapTradeOrderID.begin();
	std::map<std::string, CDataCenter::SOrderInfo>::iterator itE = pDataCenter->m_mapTradeOrderID.end();
	int buyCnt = 0;
	int sellCnt = 0;
	while(itB != itE)
	{
		if(itB->second.tradeType == eTradeType_buy)
			buyCnt++;
		else if(itB->second.tradeType == eTradeType_sell)
			sellCnt++;
		++itB;
	}
	bool bCheck = ((m_maxBuyOrderCnt == 0 || buyCnt < m_maxBuyOrderCnt) && (m_maxSellOrderCnt == 0 || sellCnt < m_maxSellOrderCnt));
	if(pDataCenter && mapBuyEntrustDepth.size() && mapSellEntrustDepth.size() && bCheck)
	{
		double buyPrice, sellPrice;
		sellPrice = atof(mapSellEntrustDepth.begin()->first.c_str());
		sellPrice = CFuncCommon::Round(sellPrice, m_tradePriceDecimal);
		std::map<std::string, std::string>::iterator it = mapBuyEntrustDepth.end();
		it--;
		buyPrice = atof(it->first.c_str());
		buyPrice = CFuncCommon::Round(buyPrice, m_tradePriceDecimal);
		if(m_bUseRreferenceCheck)
		{
			double latestExecutedOrderPrice = pReferenceExchange->GetDataCenter()->m_mapLatestExecutedOrderPrice[""];
			if(buyPrice < sellPrice && latestExecutedOrderPrice > 0)
			{
				bool bPass = true;
				int firstAction = 0;
				if(buyPrice < latestExecutedOrderPrice && sellPrice > latestExecutedOrderPrice)
				{
					double tempBuyPrice = (sellPrice - buyPrice) / 2 + buyPrice;
					if(tempBuyPrice > latestExecutedOrderPrice && (tempBuyPrice / latestExecutedOrderPrice) > 1.001)
						buyPrice = latestExecutedOrderPrice;
					else if(tempBuyPrice < latestExecutedOrderPrice && (latestExecutedOrderPrice / tempBuyPrice) >1.001)
						buyPrice = latestExecutedOrderPrice;
					else
						buyPrice = tempBuyPrice;
				}
				else
				{
					double tradePremiumPrice = 1 / double(pow(10, m_tradePriceDecimal));
					if(buyPrice > latestExecutedOrderPrice)
					{
						if((buyPrice / latestExecutedOrderPrice) > 1 + m_priceCheckValue)
							bPass = false;
						else
						{
							static int arrCheckOffset[] = {2, 3, 4, 5, 1};
							bool bFound = false;
							for(int i = 0; i<5; ++i)
							{
								double newBuyPrice = sellPrice - tradePremiumPrice * arrCheckOffset[i];
								if(newBuyPrice > buyPrice)
								{
									bFound = true;
									buyPrice = newBuyPrice;
									break;
								}
							}
							firstAction = 2;
						}
					}
					else if(sellPrice < latestExecutedOrderPrice)
					{
						if((latestExecutedOrderPrice / sellPrice) > 1 + m_priceCheckValue)
							bPass = false;
						else
						{
							static int arrCheckOffset[] = {2, 3, 4, 5, 1};
							bool bFound = false;
							for(int i = 0; i<5; ++i)
							{
								double newBuyPrice = buyPrice + tradePremiumPrice * arrCheckOffset[i];
								if(newBuyPrice < sellPrice)
								{
									bFound = true;
									buyPrice = newBuyPrice;
									break;
								}
							}
							if(!bFound)
								buyPrice = sellPrice;
							firstAction = 1;
						}
					}
								
					//else if(sellPrice > latestExecutedOrderPrice && (sellPrice/latestExecutedOrderPrice) > 1.005)
					//	bPass = false;
					//else if(sellPrice < latestExecutedOrderPrice && (latestExecutedOrderPrice/sellPrice) > 1.005)
					//	bPass = false;
					/*else
					{
						double tradePremiumPrice = 1 / double(pow(10, m_tradePriceDecimal));
						static int arrCheckOffset[] = {2, 3, 4, 5, 1};
						bool bFound = false;
						for(int i = 0; i<5; ++i)
						{
							double newBuyPrice = sellPrice - tradePremiumPrice * arrCheckOffset[i];
							if(newBuyPrice > buyPrice)
							{
								bFound = true;
								buyPrice = newBuyPrice;
								break;
							}
						}
						if(!bFound)
							buyPrice = sellPrice;

								
						//buyPrice = buyPrice + tradePremiumPrice * 3;
					}*/
				}
				if(bPass)
				{
					CString szPrice = CFuncCommon::Double2String(buyPrice, m_tradePriceDecimal).c_str();
					if(firstAction == 0)
					{
						if(pExchange->GetTradeHttp())
							pExchange->GetTradeHttp()->API_Trade(m_marketType, m_strTradeVolume.GetBuffer(), szPrice.GetBuffer(), true, g_trade_pair_index, "buy");
						if(pExchange->GetTradeHttp())
							pExchange->GetTradeHttp()->API_Trade(m_marketType, m_strTradeVolume.GetBuffer(), szPrice.GetBuffer(), false, g_trade_pair_index, "sell");
					}
					else if(firstAction == 1)
					{
						if(pExchange->GetTradeHttp())
							pExchange->GetTradeHttp()->API_Trade(m_marketType, m_strTradeVolume.GetBuffer(), szPrice.GetBuffer(), true, g_trade_pair_index, "buy");
						Sleep(1);
						if(pExchange->GetTradeHttp())
							pExchange->GetTradeHttp()->API_Trade(m_marketType, m_strTradeVolume.GetBuffer(), szPrice.GetBuffer(), false, g_trade_pair_index, "sell");
					}
					else if(firstAction == 2)
					{
						if(pExchange->GetTradeHttp())
							pExchange->GetTradeHttp()->API_Trade(m_marketType, m_strTradeVolume.GetBuffer(), szPrice.GetBuffer(), false, g_trade_pair_index, "sell");
						Sleep(1);
						if(pExchange->GetTradeHttp())
							pExchange->GetTradeHttp()->API_Trade(m_marketType, m_strTradeVolume.GetBuffer(), szPrice.GetBuffer(), true, g_trade_pair_index, "buy");
					}
					STradePair pair;
					pair.tSendTime = time(NULL);
					pair.tLastCheckTime = pair.tSendTime;
					m_mapTradePairs[g_trade_pair_index] = pair;
					g_trade_pair_index++;
				}
			}
		}
		else
		{
			double finalPrice = 0.0;
			CString szFinalPrice = "0.0";
			double tradePremiumPrice = 1 / double(pow(10, m_tradePriceDecimal));
			if(m_fSpecPrice > 0)
			{
				if(m_fSpecPrice > buyPrice && m_fSpecPrice < sellPrice)
				{
					finalPrice = m_fSpecPrice;
					szFinalPrice = CFuncCommon::Double2String(finalPrice, m_tradePriceDecimal).c_str();
				}
			}
			else
			{
				buyPrice += 0.0000001;
				static int arrCheckOffset[] = {2, 3, 4, 5, 1};
				bool bFound = false;
				for(int i = 0; i<5; ++i)
				{
					double newBuyPrice = sellPrice - tradePremiumPrice * arrCheckOffset[i];
					if(newBuyPrice > buyPrice)
					{
						bFound = true;
						finalPrice = newBuyPrice;
						break;
					}
				}
				if(!bFound)
				{
					std::map<std::string, std::string>::iterator itTemp = mapBuyEntrustDepth.end();
					itTemp--;
					szFinalPrice = itTemp->first.c_str();
					finalPrice = atof(szFinalPrice.GetBuffer());
					finalPrice = CFuncCommon::Round(finalPrice, m_tradePriceDecimal);
				}
				else
					szFinalPrice = CFuncCommon::Double2String(finalPrice, m_tradePriceDecimal).c_str();
			}
			bool bCheck = true;
			if(m_minPrice > 0.0000001 && m_maxPrice > 0.0000001)
			{
				if(finalPrice < m_minPrice || finalPrice > m_maxPrice)
				{
					if(finalPrice > m_maxPrice)
					{
						//尝试压价
						std::map<std::string, std::string>::reverse_iterator itRB = mapBuyEntrustDepth.rbegin();
						std::map<std::string, std::string>::reverse_iterator itRE = mapBuyEntrustDepth.rend();
						double pushPrice = 0.0;
						double num = 0;
						while(itRB != itRE)
						{
							double ___price = atof(itRB->first.c_str());
							___price = CFuncCommon::Round(___price, m_tradePriceDecimal);
							num += atof(itRB->second.c_str());
							if(___price < m_maxPrice)
							{
								pushPrice = ___price;
								break;
							}
							++itRB;
						}
						if(pushPrice > 0 && num < 50)
						{
							num += 1;
							CString szPushVolume, szPushPrice;
							szPushVolume.Format("%f", num);
							g_pCoinDlg->AddLog("自动压价%.2f个!", num);
							szPushPrice = CFuncCommon::Double2String(pushPrice, m_tradePriceDecimal).c_str();
							if(pExchange->GetTradeHttp())
								pExchange->GetTradeHttp()->API_Trade(m_marketType, szPushVolume.GetBuffer(), szPushPrice.GetBuffer(), false, -1, "sell");
						}
					}
					bCheck = false;
				}
			}
			if(finalPrice > 0.0000001 && bCheck)
			{
				CString szTradeVolume = m_strTradeVolume;
				int nTradeVolume = atoi(szTradeVolume.GetBuffer());
				if(m_marketType != eMarketType_BWB_USDT && m_marketType != eMarketType_BWB_ETH && m_marketType != eMarketType_BWB_QC && m_marketType != eMarketType_USDT_QC)
				{
					//KillTimer(eTimerType_Trade);
					AfxMessageBox("必须挖BWB_USDT,BWB_ETH,BWB_QC,USDT_QC之一");
					return;
				}
				const char* sellName = NULL;
				if(m_marketType == eMarketType_BWB_USDT || m_marketType == eMarketType_BWB_ETH || m_marketType == eMarketType_BWB_QC)
					sellName = "bwb";
				else if(m_marketType == eMarketType_USDT_QC)
					sellName = "usdt";
				double sellCnt = pDataCenter->m_mapBalanceInfo[sellName].balance;
				double buyCnt = 0;
				if(m_marketType == eMarketType_BWB_USDT)
					buyCnt = pDataCenter->m_mapBalanceInfo["usdt"].balance / finalPrice*0.9;
				else if(m_marketType == eMarketType_BWB_ETH)
					buyCnt = pDataCenter->m_mapBalanceInfo["eth"].balance / finalPrice*0.9;
				else if(m_marketType == eMarketType_BWB_QC || m_marketType == eMarketType_USDT_QC)
					buyCnt = pDataCenter->m_mapBalanceInfo["qc"].balance / finalPrice*0.9;
				if(nTradeVolume <= (int)sellCnt && nTradeVolume <= (int)buyCnt)
				{
				}
				else if(nTradeVolume <= (int)sellCnt || nTradeVolume <= (int)buyCnt)
				{
					double minCnt = min(sellCnt, buyCnt);
					if(minCnt > nTradeVolume*0.3)
					{
						nTradeVolume = (int)minCnt;
						szTradeVolume.Format("%d", nTradeVolume);
					}
				}
				/*else if(nTradeVolume <= (int)buyCnt)
				{
					double tradePremiumPrice = 1 / double(pow(10, m_tradePriceDecimal));
					double newBuyPrice = buyPrice + tradePremiumPrice;
					buyCnt = pDataCenter->m_mapBalanceInfo["usdt"].balance / newBuyPrice*0.9;
					buyCnt = min(nTradeVolume, buyCnt);
					szFinalPrice = CFuncCommon::Double2String(newBuyPrice, m_tradePriceDecimal).c_str();
					nTradeVolume = (int)buyCnt;
					szTradeVolume.Format("%d", nTradeVolume);
				}*/
				else
				{
					if(sellCnt <= buyCnt)
						nTradeVolume = int(sellCnt*0.9);
					else
						nTradeVolume = (int)buyCnt;
					if(nTradeVolume <= 10)
					{
						if(sellCnt <= buyCnt)
							nTradeVolume = (int)buyCnt;
						else
							nTradeVolume = int(sellCnt*0.9);
					}
					szTradeVolume.Format("%d", nTradeVolume);
				}
				if(nTradeVolume > 0)
				{
					if(pExchange->GetTradeHttp())
						pExchange->GetTradeHttp()->API_Trade(m_marketType, szTradeVolume.GetBuffer(), szFinalPrice.GetBuffer(), false, g_trade_pair_index, "sell");
					Sleep(1);
					if(pExchange->GetTradeHttp())
						pExchange->GetTradeHttp()->API_Trade(m_marketType, szTradeVolume.GetBuffer(), szFinalPrice.GetBuffer(), true, g_trade_pair_index, "buy");
					STradePair pair;
					pair.tSendTime = time(NULL);
					pair.tLastCheckTime = pair.tSendTime;
					m_mapTradePairs[g_trade_pair_index] = pair;
					g_trade_pair_index++;
				}
			}
		}
	}
}