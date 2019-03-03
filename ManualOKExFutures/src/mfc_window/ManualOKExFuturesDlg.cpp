
// ManualOKExFuturesDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ManualOKExFutures.h"
#include "ManualOKExFuturesDlg.h"

#include "afxdialogex.h"
#include <clib/lib/file/file_util.h>
#include "log/local_log.h"
#include "log/local_action_log.h"
#include "exchange/okex/okex_exchange.h"
#include "common/func_common.h"
#include "api_callback.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#define DOUBLE_PRECISION 0.00000001
#define OKEX_CHANGE ((COkexExchange*)pExchange)
CExchange* pExchange = NULL;
CManualOKExFuturesDlg* g_pDlg = NULL;
int LocalLogCallBackFunc(LOG_TYPE type, const char* szLog)
{
	if(g_pDlg->m_ctrlListLog.GetCount() > 1500)
		g_pDlg->m_ctrlListLog.ResetContent();
	g_pDlg->m_ctrlListLog.InsertString(0, szLog);
	g_pDlg->SetHScroll();
	return 0;
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
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CManualOKExFuturesDlg 对话框



CManualOKExFuturesDlg::CManualOKExFuturesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CManualOKExFuturesDlg::IDD, pParent)
{
	g_pDlg = this;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CManualOKExFuturesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_OPEN, m_listCtrlOrderOpen);
	DDX_Control(pDX, IDC_LIST_CLOSE, m_listCtrlOrderClose);
	DDX_Control(pDX, IDC_COMBO1, m_combCoinType);
	DDX_Control(pDX, IDC_COMBO2, m_combLeverage);
	DDX_Control(pDX, IDC_EDIT1, m_editFuturesCycle);
	DDX_Control(pDX, IDC_EDIT2, m_editFuturesTradeSize);
	DDX_Control(pDX, IDC_COMBO3, m_combFuturesType);
	DDX_Control(pDX, IDC_EDIT3, m_editCapital);
	DDX_Control(pDX, IDC_EDIT4, m_editCapitalToday);
	DDX_Control(pDX, IDC_LIST1, m_ctrlListLog);
	DDX_Control(pDX, IDC_LIST_CLOSE2, m_listCtrlDepth);
	DDX_Control(pDX, IDC_EDIT5, m_editOpenPrice);
}

BEGIN_MESSAGE_MAP(CManualOKExFuturesDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDOK, &CManualOKExFuturesDlg::OnBnClickedStart)
	ON_BN_CLICKED(IDC_BUTTON1, &CManualOKExFuturesDlg::OnBnClickedButtonOpenBull)
	ON_BN_CLICKED(IDC_BUTTON2, &CManualOKExFuturesDlg::OnBnClickedButtonOpenBear)
END_MESSAGE_MAP()


// CManualOKExFuturesDlg 消息处理程序

BOOL CManualOKExFuturesDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
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

	m_combFuturesType.InsertString(0, "交割合约");
	m_combFuturesType.InsertString(1, "永续合约");

	m_listCtrlOrderOpen.InsertColumn(0, "价格", LVCFMT_CENTER, 90);
	m_listCtrlOrderOpen.InsertColumn(1, "类型", LVCFMT_CENTER, 45);
	m_listCtrlOrderOpen.InsertColumn(2, "成交量", LVCFMT_CENTER, 70);
	m_listCtrlOrderOpen.InsertColumn(3, "状态", LVCFMT_CENTER, 45);
	m_listCtrlOrderOpen.InsertColumn(4, "参考利润", LVCFMT_CENTER, 120);
	m_listCtrlOrderOpen.InsertColumn(5, "下单时间", LVCFMT_CENTER, 115);
	m_listCtrlOrderOpen.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);

	m_listCtrlOrderClose.InsertColumn(0, "价格", LVCFMT_CENTER, 85);
	m_listCtrlOrderClose.InsertColumn(1, "成交量", LVCFMT_CENTER, 70);
	m_listCtrlOrderClose.InsertColumn(2, "状态", LVCFMT_CENTER, 45);
	m_listCtrlOrderClose.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

	m_listCtrlDepth.InsertColumn(0, "", LVCFMT_CENTER, 30);
	m_listCtrlDepth.InsertColumn(1, "价", LVCFMT_CENTER, 70);
	m_listCtrlDepth.InsertColumn(2, "量", LVCFMT_CENTER, 80);
	m_listCtrlDepth.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	// TODO:  在此添加额外的初始化代码
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
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CManualOKExFuturesDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CManualOKExFuturesDlg::OnPaint()
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
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CManualOKExFuturesDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CManualOKExFuturesDlg::__InitConfigCtrl()
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
	strTemp = m_config.get("futures", "beginMoney", "");
	m_editCapital.SetWindowText(strTemp.c_str());
	strTemp = m_config.get("futures", "todayBeginMoney", "");
	m_editCapitalToday.SetWindowText(strTemp.c_str());
}

bool CManualOKExFuturesDlg::__SaveConfigCtrl()
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

	CString szCost = "";
	m_editCapital.GetWindowText(szCost);

	CString szTodayCost = "";
	m_editCapitalToday.GetWindowText(szTodayCost);

	m_strCoinType = strCoinType.GetBuffer();
	m_strFuturesCycle = strFuturesCycle.GetBuffer();
	m_strFuturesTradeSize = strFuturesTradeSize.GetBuffer();
	m_strLeverage = strLeverage.GetBuffer();
	m_nLeverage = stoi(m_strLeverage);
	if(strFuturesType == "永续合约")
		m_bSwapFutures = true;
	else
		m_bSwapFutures = false;
	if(szCost != "")
		m_beginMoney = stod(szCost.GetBuffer());
	if(szTodayCost != "")
		m_todayBeginMoney = stod(szTodayCost.GetBuffer());
	m_config.set_value("futures", "coinType", m_strCoinType.c_str());
	m_config.set_value("futures", "futuresCycle", m_strFuturesCycle.c_str());
	m_config.set_value("futures", "futuresTradeSize", m_strFuturesTradeSize.c_str());
	m_config.set_value("futures", "leverage", m_strLeverage.c_str());
	m_config.set_value("futures", "futuresType", strFuturesType.GetBuffer());
	m_config.set_value("futures", "beginMoney", szCost.GetBuffer());
	m_config.set_value("futures", "todayBeginMoney", szTodayCost.GetBuffer());
	m_config.save("./config.ini");

	return true;
}

void CManualOKExFuturesDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	switch(nIDEvent)
	{
	case eTimerType_APIUpdate:
		{
			CLocalLogger::GetInstancePt()->SwapFront2Middle();
			if(OKEX_CHANGE)
				OKEX_CHANGE->Update();
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
		}
		break;
	case eTimerType_Ping:
		{
			if(OKEX_WEB_SOCKET->Ping())
				m_tListenPong = time(NULL);
		}
		break;
	case eTimerType_Account:
		{
			if(m_bRun)
				 OKEX_HTTP->API_FuturesAccountInfoByCurrency(m_bSwapFutures, m_strCoinType);
		}
		break;
	}

	CDialog::OnTimer(nIDEvent);
}


void CManualOKExFuturesDlg::OnBnClickedStart()
{
	if(!__SaveConfigCtrl())
		return;
	if(m_bRun)
		return;
	bool bFound = false;
	std::string instrumentID;
	if(m_bSwapFutures)
		instrumentID = m_strCoinType + "-USD-SWAP";
	else
		instrumentID = m_strCoinType + "-USD-" + m_strFuturesCycle;
	for(int i = 0; i < 3; ++i)
	{
		SHttpResponse resInfo;
		OKEX_HTTP->API_FuturesInstruments(false, m_bSwapFutures, &resInfo);
		bFound = false;
		if(resInfo.retObj.isArray())
		{
			for(int j = 0; j < (int)resInfo.retObj.size(); ++j)
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
		OKEX_WEB_SOCKET->API_FuturesTickerData(true, m_bSwapFutures, m_strCoinType, m_strFuturesCycle);
		OKEX_WEB_SOCKET->API_LoginFutures(m_apiKey, m_secretKey, time(NULL));
		OKEX_WEB_SOCKET->API_FuturesEntrustDepth(true, m_bSwapFutures, m_strCoinType, m_strFuturesCycle);
	}
	CString strTitle;
	strTitle.Format("%s-%s", (m_bSwapFutures ? "永续合约" : "交割合约"), m_strCoinType.c_str());
	CWnd *m_pMainWnd;
	m_pMainWnd = AfxGetMainWnd();
	m_pMainWnd->SetWindowText(strTitle);
	m_bRun = true;
}


void CManualOKExFuturesDlg::OnRevTickerInfo(STickerData &data)
{
	m_curTickData = data;
	m_curTickData.bValid = true;
	
	_UpdateTradeShow();
}

void CManualOKExFuturesDlg::Pong()
{
	m_tListenPong = 0;
}

void CManualOKExFuturesDlg::OnLoginSuccess()
{
	OKEX_WEB_SOCKET->API_FuturesOrderInfo(true, m_bSwapFutures, m_strCoinType, m_strFuturesCycle);
	m_vecTradePairInfo.clear();
	std::string strFilePath = "./save.txt";
	std::ifstream stream(strFilePath);
	if(!stream.is_open())
		return;
	char lineBuffer[4096] = { 0 };
	if(stream.fail())
		return;
	while(!stream.eof())
	{
		stream.getline(lineBuffer, sizeof(lineBuffer));
		if(*lineBuffer == 0 || (lineBuffer[0] == '/' && lineBuffer[1] == '/') || (lineBuffer[0] == '-' && lineBuffer[1] == '-'))
			continue;
		std::stringstream lineStream(lineBuffer, std::ios_base::in);
		char szOpenClientID[128] = { 0 };
		char szOpenOrderID[128] = { 0 };
		char szCloseClientID[128] = { 0 };
		char szCloseOrderID[128] = { 0 };
		lineStream >> szOpenOrderID >> szOpenClientID >> szCloseOrderID >> szCloseClientID;
		SFuturesTradePairInfo info;
		if(strcmp(szOpenOrderID, "0") != 0)
		{
			info.open.strClientOrderID = szOpenClientID;
			info.open.orderID = szOpenOrderID;
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
			}
		}
		if(strcmp(szOpenOrderID, "0") != 0)
			m_vecTradePairInfo.push_back(info);
	}
	stream.close();
	_UpdateTradeShow();
}

void CManualOKExFuturesDlg::UpdateTradeInfo(SFuturesTradeInfo& info)
{
	std::vector<SFuturesTradePairInfo>::iterator itB = m_vecTradePairInfo.begin();
	std::vector<SFuturesTradePairInfo>::iterator itE = m_vecTradePairInfo.end();
	while(itB != itE)
	{
		if(info.tradeType == itB->open.tradeType && itB->open.orderID == info.orderID)
		{
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
		++itB;
	}
	_UpdateTradeShow();
}

void CManualOKExFuturesDlg::UpdateAccountInfo(SFuturesAccountInfo& info)
{
	m_accountInfo = info;
	m_accountInfo.bValid = true;
	_UpdateAccountShow();
}

void CManualOKExFuturesDlg::ClearDepth()
{
	m_mapDepthSell.clear();
	m_mapDepthBuy.clear();
}

void CManualOKExFuturesDlg::UpdateDepthInfo(bool bBuy, SFuturesDepth& info)
{
	if(bBuy)
	{
		if(info.size == "0")
			m_mapDepthBuy.erase(info.price);
		else
			m_mapDepthBuy[info.price] = info;
	}
	else
	{
		if(info.size == "0")
			m_mapDepthSell.erase(info.price);
		else
			m_mapDepthSell[info.price] = info;
	}
	_UpdateDepthShow();
}

void CManualOKExFuturesDlg::_UpdateAccountShow()
{

}

void CManualOKExFuturesDlg::_UpdateTradeShow()
{

}

void CManualOKExFuturesDlg::_UpdateDepthShow()
{
	const int showLines = 5;
	m_listCtrlDepth.DeleteAllItems();
	int sellLine = min(m_mapDepthSell.size(), showLines);
	for(int i = 0; i < sellLine; ++i)
	{
		m_listCtrlDepth.InsertItem(i, "");
	}
	std::map<std::string, SFuturesDepth>::iterator itB = m_mapDepthSell.begin();
	std::map<std::string, SFuturesDepth>::iterator itE = m_mapDepthSell.end();
	CString szFormat;
	int count = 0;
	while(itB != itE)
	{
		szFormat.Format("%d", count + 1);
		m_listCtrlDepth.SetItemText(sellLine - 1 - count, 0, szFormat);
		szFormat.Format("%s", itB->first.c_str());
		m_listCtrlDepth.SetItemText(sellLine - 1 - count, 1, szFormat);
		szFormat.Format("%s", itB->second.size.c_str());
		m_listCtrlDepth.SetItemText(sellLine - 1 - count, 2, szFormat);
		if(++count >= sellLine)
			break;
		++itB;
	}
	m_listCtrlDepth.InsertItem(sellLine, "");
	if(m_curTickData.bValid)
	{
		m_listCtrlDepth.InsertItem(sellLine, "");
		m_listCtrlDepth.SetItemText(sellLine, 0, "-----");
		m_listCtrlDepth.SetItemText(sellLine, 1, CFuncCommon::Double2String(m_curTickData.last, m_nPriceDecimal).c_str());
		szFormat.Format("%s---", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str());
		m_listCtrlDepth.SetItemText(sellLine, 2, szFormat);
	}
	else
	{
		m_listCtrlDepth.SetItemText(sellLine, 0, "-----");
		m_listCtrlDepth.SetItemText(sellLine, 1, "-------------");
		m_listCtrlDepth.SetItemText(sellLine, 2, "-------------");
	}
	
	int buyLine = min(m_mapDepthBuy.size(), showLines);
	for(int i = 0; i < sellLine; ++i)
	{
		m_listCtrlDepth.InsertItem(sellLine + 1 + i, "");
	}
	if(buyLine > 0)
	{
		std::map<std::string, SFuturesDepth>::iterator itB = m_mapDepthBuy.begin();
		std::map<std::string, SFuturesDepth>::iterator itE = m_mapDepthBuy.end();
		CString _szFormat;
		int count = 0;
		itE--;
		while(itB != itE)
		{
			_szFormat.Format("%d", count + 1);
			m_listCtrlDepth.SetItemText(sellLine + 1 + count, 0, _szFormat);
			_szFormat.Format("%s", itE->first.c_str());
			m_listCtrlDepth.SetItemText(sellLine + 1 + count, 1, _szFormat);
			_szFormat.Format("%s", itE->second.size.c_str());
			m_listCtrlDepth.SetItemText(sellLine + 1 + count, 2, _szFormat);
			if(++count >= buyLine)
				break;
			itE--;
		}
		if(count < buyLine)
		{
			_szFormat.Format("%d", count + 1);
			m_listCtrlDepth.SetItemText(sellLine + 1 + count, 0, _szFormat);
			_szFormat.Format("%s", itB->first.c_str());
			m_listCtrlDepth.SetItemText(sellLine + 1 + count, 1, _szFormat);
			_szFormat.Format("%s", itB->second.size.c_str());
			m_listCtrlDepth.SetItemText(sellLine + 1 + count, 2, _szFormat);
		}
	}
}

void CManualOKExFuturesDlg::SetHScroll()
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

		temp = (long)SendDlgItemMessage(IDC_LIST1, LB_GETHORIZONTALEXTENT, 0, 0);
		if(s.cx > temp)
			SendDlgItemMessage(IDC_LIST1, LB_SETHORIZONTALEXTENT, (WPARAM)s.cx, 0);
	}
	ReleaseDC(dc);
}

void CManualOKExFuturesDlg::OnBnClickedButtonOpenBull()
{
	// TODO:  在此添加控件通知处理程序代码
}


void CManualOKExFuturesDlg::OnBnClickedButtonOpenBear()
{
	// TODO:  在此添加控件通知处理程序代码
}
