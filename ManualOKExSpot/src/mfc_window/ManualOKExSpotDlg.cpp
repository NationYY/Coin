
// ManualOKExSpotDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ManualOKExSpot.h"
#include "ManualOKExSpotDlg.h"

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

#define OKEX_CHANGE ((COkexExchange*)pExchange)
CExchange* pExchange = NULL;
CManualOKExSpotDlg* g_pDlg = NULL;
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


// CManualOKExSpotDlg 对话框



CManualOKExSpotDlg::CManualOKExSpotDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CManualOKExSpotDlg::IDD, pParent)
{
	g_pDlg = this;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_bWaitDepthBegin = true;
	m_tListenPong = 0;
	m_bRun = false;
	m_apiKey = "";
	m_secretKey = "";
	m_passphrase = "";
	m_nPriceDecimal = 0;
	m_nSizeDecimal = 0;
	m_tWaitNewSubDepth = 0;
	m_strFirstCoinType = "";
	m_strSecondCoinType = "";
	m_strInstrumentType = "";
}

void CManualOKExSpotDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_OPEN, m_listCtrlOrderOpen);
	DDX_Control(pDX, IDC_LIST_CLOSE, m_listCtrlOrderClose);
	DDX_Control(pDX, IDC_COMBO1, m_combCoinType);
	DDX_Control(pDX, IDC_EDIT2, m_editSpotTradeSize);
	DDX_Control(pDX, IDC_LIST1, m_ctrlListLog);
	DDX_Control(pDX, IDC_LIST_CLOSE2, m_listCtrlDepth);
	DDX_Control(pDX, IDC_EDIT5, m_editTradePrice);
	DDX_Control(pDX, IDC_EDIT6, m_editClosePrice);
	DDX_Control(pDX, IDC_EDIT7, m_editCloseSize);
	DDX_Control(pDX, IDC_LIST_OPEN2, m_listCtrlAccountInfo);
}

BEGIN_MESSAGE_MAP(CManualOKExSpotDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDOK, &CManualOKExSpotDlg::OnBnClickedStart)
	ON_BN_CLICKED(IDC_BUTTON1, &CManualOKExSpotDlg::OnBnClickedButtonBuy)
	ON_BN_CLICKED(IDC_BUTTON2, &CManualOKExSpotDlg::OnBnClickedButtonSell)
	ON_BN_CLICKED(IDC_BUTTON3, &CManualOKExSpotDlg::OnBnClickedButtonClose)
	ON_BN_CLICKED(IDC_BUTTON4, &CManualOKExSpotDlg::OnBnClickedButtonCancel)
	ON_BN_CLICKED(IDC_BUTTON7, &CManualOKExSpotDlg::OnBnClickedButtonClearFreeLine)
	ON_BN_CLICKED(IDC_BUTTON8, &CManualOKExSpotDlg::OnBnClickedButtonBearFirst)
	ON_BN_CLICKED(IDC_BUTTON9, &CManualOKExSpotDlg::OnBnClickedButtonBullFirst)
END_MESSAGE_MAP()


// CManualOKExSpotDlg 消息处理程序

BOOL CManualOKExSpotDlg::OnInitDialog()
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
	if(!m_config.open("./config.ini"))
		return FALSE;
	int index = 0;
	for(int i=0; i<100; ++i)
	{
		CString szTemp;
		szTemp.Format("instrument%d", i+1);
		std::string instrument = m_config.get("spot", szTemp.GetBuffer(), "");
		if(instrument != "")
			m_combCoinType.InsertString(index++, instrument.c_str());
	}

	m_listCtrlOrderOpen.InsertColumn(0, "序号", LVCFMT_CENTER, 40);
	m_listCtrlOrderOpen.InsertColumn(1, "价格", LVCFMT_CENTER, 75);
	m_listCtrlOrderOpen.InsertColumn(2, "类型", LVCFMT_CENTER, 45);
	m_listCtrlOrderOpen.InsertColumn(3, "成交量", LVCFMT_CENTER, 70);
	m_listCtrlOrderOpen.InsertColumn(4, "状态", LVCFMT_CENTER, 65);
	m_listCtrlOrderOpen.InsertColumn(5, "参考利润", LVCFMT_CENTER, 100);
	m_listCtrlOrderOpen.InsertColumn(6, "下单时间", LVCFMT_CENTER, 115);
	m_listCtrlOrderOpen.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);


	m_listCtrlOrderClose.InsertColumn(0, "序号", LVCFMT_CENTER, 40);
	m_listCtrlOrderClose.InsertColumn(1, "价格", LVCFMT_CENTER, 75);
	m_listCtrlOrderClose.InsertColumn(2, "成交量", LVCFMT_CENTER, 70);
	m_listCtrlOrderClose.InsertColumn(3, "状态", LVCFMT_CENTER, 65);
	m_listCtrlOrderClose.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);

	m_listCtrlDepth.InsertColumn(0, "", LVCFMT_CENTER, 30);
	m_listCtrlDepth.InsertColumn(1, "价", LVCFMT_CENTER, 70);
	m_listCtrlDepth.InsertColumn(2, "量", LVCFMT_CENTER, 80);
	m_listCtrlDepth.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);


	m_listCtrlAccountInfo.InsertColumn(0, "币种", LVCFMT_CENTER, 100);
	m_listCtrlAccountInfo.InsertColumn(1, "余额", LVCFMT_CENTER, 100);
	m_listCtrlAccountInfo.InsertColumn(2, "冻结数量", LVCFMT_CENTER, 100);
	m_listCtrlAccountInfo.InsertColumn(3, "可交易数量", LVCFMT_CENTER, 100);
	m_listCtrlAccountInfo.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	// TODO:  在此添加额外的初始化代码
	m_apiKey = m_config.get("spot", "apiKey", "");
	m_secretKey = m_config.get("spot", "secretKey", "");
	m_passphrase = m_config.get("spot", "passphrase", "");
	__InitBaseConfigCtrl();
	__InitTradeConfigCtrl();

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

void CManualOKExSpotDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CManualOKExSpotDlg::OnPaint()
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
HCURSOR CManualOKExSpotDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CManualOKExSpotDlg::__InitBaseConfigCtrl()
{
	std::string strTemp = m_config.get("spot", "instrumentType", "");
	if(strTemp != "")
	{
		int nIndex = m_combCoinType.FindString(0, strTemp.c_str());
		if(nIndex != -1)
			m_combCoinType.SetCurSel(nIndex);
	}
}

void CManualOKExSpotDlg::__InitTradeConfigCtrl()
{
	std::string strTemp = m_config.get("spot", "spotTradeSize", "");
	m_editSpotTradeSize.SetWindowText(strTemp.c_str());
	m_strSpotTradeSize = strTemp.c_str();
}

bool CManualOKExSpotDlg::__SaveBaseConfigCtrl()
{
	CString strCoinType;
	m_combCoinType.GetWindowText(strCoinType);
	if(strCoinType == "")
	{
		MessageBox("未选择交易对");
		return false;
	}
	m_strInstrumentType = strCoinType.GetBuffer();
	int pos = m_strInstrumentType.find_first_of("-");
	if(pos != std::string::npos)
	{
		m_strFirstCoinType = m_strInstrumentType.substr(0, pos);
		m_strSecondCoinType = m_strInstrumentType.substr(pos+1);
	}
	m_config.set_value("spot", "instrumentType", m_strInstrumentType.c_str());
	m_config.save("./config.ini");
	return true;
}

bool CManualOKExSpotDlg::__SaveTradeConfigCtrl()
{
	CString strSpotTradeSize;
	m_editSpotTradeSize.GetWindowText(strSpotTradeSize);
	if(strSpotTradeSize == "")
	{
		MessageBox("未填写下单数量");
		return false;
	}
	m_strSpotTradeSize = strSpotTradeSize.GetBuffer();
	m_config.set_value("spot", "spotTradeSize", m_strSpotTradeSize.c_str());
	m_config.save("./config.ini");
	return true;
}

void CManualOKExSpotDlg::OnTimer(UINT_PTR nIDEvent)
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
			if(m_tWaitNewSubDepth && tNow - m_tWaitNewSubDepth >= 5)
			{
				OKEX_WEB_SOCKET->API_EntrustDepth(true, m_strInstrumentType);
				m_tWaitNewSubDepth = 0;
			}
			std::map<time_t, std::list<std::string> >::iterator itB = m_mapFindTradeinfo.begin();
			std::map<time_t, std::list<std::string> >::iterator itE = m_mapFindTradeinfo.end();
			while(itB != itE)
			{
				if(tNow < itB->first)
					break;
				std::list<std::string>::iterator _itB = itB->second.begin();
				std::list<std::string>::iterator _itE = itB->second.end();
				while(_itB != _itE)
				{
					OKEX_HTTP->API_SpotOrderInfo(true, m_strInstrumentType, *_itB);
					++_itB;
				}
				itB = m_mapFindTradeinfo.erase(itB);
			}
			_CheckAllOrder();
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
			{
				OKEX_HTTP->API_SpotAccountInfoByCurrency(true, m_strFirstCoinType);
				OKEX_HTTP->API_SpotAccountInfoByCurrency(true, m_strSecondCoinType);
			}	 
		}
		break;
	}

	CDialog::OnTimer(nIDEvent);
}


void CManualOKExSpotDlg::OnBnClickedStart()
{
	if(!__SaveBaseConfigCtrl())
		return;
	if(m_bRun)
		return;
	bool bFound = false;
	for(int i = 0; i < 3; ++i)
	{
		SHttpResponse resInfo;
		OKEX_HTTP->API_SpotInstruments(false, &resInfo);
		bFound = false;
		if(resInfo.retObj.isArray())
		{
			for(int j = 0; j < (int)resInfo.retObj.size(); ++j)
			{
				if(resInfo.retObj[j]["instrument_id"].asString() == m_strInstrumentType)
				{
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
					}
					{
						std::string strMinSize = resInfo.retObj[j]["min_size"].asString();
						int pos = strMinSize.find_first_of(".");
						if(pos != std::string::npos)
						{
							m_nSizeDecimal = strMinSize.size() - pos - 1;
							bFound = true;
						}
						else
						{
							m_nSizeDecimal = 0;
							bFound = true;
						}
						LOCAL_INFO("SizeDecimal=%d", m_nSizeDecimal);
					}
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
		OKEX_WEB_SOCKET->API_SpotTickerData(true, m_strInstrumentType);
		OKEX_WEB_SOCKET->API_LoginFutures(m_apiKey, m_secretKey, time(NULL));
		OKEX_WEB_SOCKET->API_EntrustDepth(true, m_strInstrumentType);
	}
	CString strTitle;
	strTitle.Format("%s", m_strInstrumentType.c_str());
	CWnd *m_pMainWnd;
	m_pMainWnd = AfxGetMainWnd();
	m_pMainWnd->SetWindowText(strTitle);
	m_bRun = true;
}


void CManualOKExSpotDlg::OnRevTickerInfo(STickerData &data)
{
	m_curTickData = data;
	m_curTickData.bValid = true;
	time_t tNow = time(NULL);
	for(int i=0; i<(int)m_vecTradePairInfo.size(); ++i)
	{
		if(m_vecTradePairInfo[i].open.orderID != "" && (m_vecTradePairInfo[i].open.status == "open" || m_vecTradePairInfo[i].open.status == "part_filled"))
		{
			if(tNow - m_vecTradePairInfo[i].open.tLastCheck > 5)
			{
				if(m_vecTradePairInfo[i].open.tradeType == "buy")
				{
					if(m_curTickData.last < m_vecTradePairInfo[i].open.price)
					{
						m_mapFindTradeinfo[tNow+2].push_back(m_vecTradePairInfo[i].open.orderID);
						m_vecTradePairInfo[i].open.tLastCheck = tNow;
					}
				}
				else if(m_vecTradePairInfo[i].open.tradeType == "sell")
				{
					if(m_curTickData.last > m_vecTradePairInfo[i].open.price)
					{
						m_mapFindTradeinfo[tNow + 2].push_back(m_vecTradePairInfo[i].open.orderID);
						m_vecTradePairInfo[i].open.tLastCheck = tNow;
					}
				}
			}
		}
		if(m_vecTradePairInfo[i].close.orderID != "" && (m_vecTradePairInfo[i].close.status == "open" || m_vecTradePairInfo[i].close.status == "part_filled"))
		{
			if(tNow - m_vecTradePairInfo[i].close.tLastCheck > 5)
			{
				if(m_vecTradePairInfo[i].close.tradeType == "buy")
				{
					if(m_curTickData.last < m_vecTradePairInfo[i].close.price)
					{
						m_mapFindTradeinfo[tNow + 2].push_back(m_vecTradePairInfo[i].close.orderID);
						m_vecTradePairInfo[i].close.tLastCheck = tNow;
					}
				}
				else if(m_vecTradePairInfo[i].close.tradeType == "sell")
				{
					if(m_curTickData.last > m_vecTradePairInfo[i].close.price)
					{
						m_mapFindTradeinfo[tNow + 2].push_back(m_vecTradePairInfo[i].close.orderID);
						m_vecTradePairInfo[i].close.tLastCheck = tNow;
					}
				}
			}
		}
	}
	_UpdateTradeShow();
	_UpdateAccountShow();
}

void CManualOKExSpotDlg::Pong()
{
	m_tListenPong = 0;
}

void CManualOKExSpotDlg::OnLoginSuccess()
{
	OKEX_WEB_SOCKET->API_SpotOrderInfo(true, m_strInstrumentType);
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

		char szClosePlanPrice[128] = { 0 };
		char szClosePlanSize[128] = { 0 };

		char szCloseClientID[128] = { 0 };
		char szCloseOrderID[128] = { 0 };
		char szOpenQTY[128] = { 0 };
		lineStream >> szOpenOrderID >> szOpenClientID >> szOpenQTY >> szClosePlanPrice >> szClosePlanSize >> szCloseOrderID >> szCloseClientID;
		SSpotTradePairInfo info;
		if(strcmp(szOpenOrderID, "0") != 0)
		{
			info.open.strClientOrderID = szOpenClientID;
			info.open.orderID = szOpenOrderID;
			SHttpResponse resInfo;
			OKEX_HTTP->API_SpotOrderInfo(false, m_strInstrumentType, info.open.orderID, &resInfo);
			Json::Value& retObj = resInfo.retObj;
			if(retObj.isObject() && retObj["order_id"].isString())
			{
				info.open.timeStamp = CFuncCommon::ISO8601ToTime(retObj["timestamp"].asString());
				info.open.filledSize = CFuncCommon::ToString(stoi(retObj["filled_size"].asString()));
				info.open.price = stod(retObj["price"].asString());
				info.open.status = retObj["status"].asString();
				info.open.size = CFuncCommon::ToString(stoi(retObj["size"].asString()));
				info.open.tradeType = retObj["side"].asString();
				if(strcmp(szOpenQTY, "0") != 0)
				{
					info.open.filledSize = szOpenQTY;
					info.open.bModifyQTY = true;
				}
				CActionLog("trade", "http更新订单信息 client_order=%s, order=%s, filledSize=%s, price=%s, priceAvg=%s, status=%s, tradeType=%s", info.open.strClientOrderID.c_str(), info.open.orderID.c_str(), info.open.filledSize.c_str(), retObj["price"].asString().c_str(), retObj["price_avg"].asString().c_str(), info.open.status.c_str(), info.open.tradeType.c_str());
			}
			if(strcmp(szClosePlanPrice, "0") != 0 && strcmp(szClosePlanSize, "0") != 0)
			{
				info.closePlanPrice = szClosePlanPrice;
				info.closePlanSize = szClosePlanSize;
			}
		}
		if(strcmp(szCloseOrderID, "0") != 0)
		{
			info.close.strClientOrderID = szCloseClientID;
			info.close.orderID = szCloseOrderID;
			SHttpResponse resInfo;
			OKEX_HTTP->API_SpotOrderInfo(false, m_strInstrumentType, info.close.orderID, &resInfo);
			Json::Value& retObj = resInfo.retObj;
			if(retObj.isObject() && retObj["order_id"].isString())
			{
				info.close.timeStamp = CFuncCommon::ISO8601ToTime(retObj["timestamp"].asString());
				info.close.filledSize = CFuncCommon::ToString(stoi(retObj["filled_size"].asString()));
				info.close.price = stod(retObj["price"].asString());
				info.close.status = retObj["status"].asString();
				info.close.size = CFuncCommon::ToString(stoi(retObj["size"].asString()));
				info.close.tradeType = retObj["type"].asString();
				CActionLog("trade", "http更新订单信息 client_order=%s, order=%s, filledSize=%s, price=%s, priceAvg=%s, status=%s, tradeType=%s", info.close.strClientOrderID.c_str(), info.close.orderID.c_str(), info.close.filledSize.c_str(), retObj["price"].asString().c_str(), retObj["price_avg"].asString().c_str(), info.close.status.c_str(), info.close.tradeType.c_str());
			}
		}
		if(strcmp(szOpenOrderID, "0") != 0)
			m_vecTradePairInfo.push_back(info);
	}
	stream.close();
	_UpdateTradeShow();
}

void CManualOKExSpotDlg::UpdateTradeInfo(SSpotTradeInfo& info)
{
	std::vector<SSpotTradePairInfo>::iterator itB = m_vecTradePairInfo.begin();
	std::vector<SSpotTradePairInfo>::iterator itE = m_vecTradePairInfo.end();
	while(itB != itE)
	{
		if(info.tradeType == itB->open.tradeType && itB->open.orderID == info.orderID)
		{
			if(itB->open.status != "filled" && info.status == "filled")
				itB->open.tLastALLFillTime = time(NULL);
			itB->open.timeStamp = info.timeStamp;
			itB->open.filledSize = info.filledSize;
			itB->open.price = info.price;
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
			itB->close.filledSize = info.filledSize;
			itB->close.price = info.price;
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

void CManualOKExSpotDlg::UpdateAccountInfo(SSpotAccountInfo& info)
{
	std::list<SSpotAccountInfo>::iterator itB = m_listAccountInfo.begin();
	std::list<SSpotAccountInfo>::iterator itE = m_listAccountInfo.end();
	while(itB != itE)
	{
		if(itB->currency == info.currency)
		{
			*itB = info;
			break;
		}
		++itB;
	}
	if(itB == itE)
		m_listAccountInfo.push_back(info);
	_UpdateAccountShow();
}

void CManualOKExSpotDlg::ClearDepth()
{
	m_mapDepthSell.clear();
	m_mapDepthBuy.clear();
}

void CManualOKExSpotDlg::UpdateDepthInfo(bool bBuy, SSpotDepth& info)
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

void CManualOKExSpotDlg::OnTradeSuccess(std::string& clientOrderID, std::string& serverOrderID)
{
	std::vector<SSpotTradePairInfo>::iterator itB = m_vecTradePairInfo.begin();
	std::vector<SSpotTradePairInfo>::iterator itE = m_vecTradePairInfo.end();
	bool bUpdate = false;
	while(itB != itE)
	{
		if(itB->open.strClientOrderID == clientOrderID)
		{
			itB->open.orderID = serverOrderID;
			itB->open.waitClientOrderIDTime = 0;
			CActionLog("trade", "http下单成功 client_order=%s, order=%s", itB->open.strClientOrderID.c_str(), itB->open.orderID.c_str());
			m_mapFindTradeinfo[time(NULL)+2].push_back(serverOrderID);
			bUpdate = true;
			break;
		}
		if(itB->close.strClientOrderID == clientOrderID)
		{
			itB->close.orderID = serverOrderID;
			itB->close.waitClientOrderIDTime = 0;
			CActionLog("trade", "http下单成功 client_order=%s, order=%s", itB->close.strClientOrderID.c_str(), itB->close.orderID.c_str());
			m_mapFindTradeinfo[time(NULL)+2].push_back(serverOrderID);
			bUpdate = true;
			break;
		}
		++itB;
	}
	if(bUpdate)
		_SaveData();
}

void CManualOKExSpotDlg::OnCancelSuccess(std::string& orderID)
{
	m_mapFindTradeinfo[time(NULL)+2].push_back(orderID);
}

void CManualOKExSpotDlg::OnTradeFail(std::string& clientOrderID)
{
	std::vector<SSpotTradePairInfo>::iterator itB = m_vecTradePairInfo.begin();
	std::vector<SSpotTradePairInfo>::iterator itE = m_vecTradePairInfo.end();
	while(itB != itE)
	{
		if(itB->open.strClientOrderID == clientOrderID && itB->open.waitClientOrderIDTime)
		{
			itB->Reset();
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

void CManualOKExSpotDlg::_UpdateAccountShow()
{
	m_listCtrlAccountInfo.DeleteAllItems();
	std::list<SSpotAccountInfo>::iterator itB = m_listAccountInfo.begin();
	std::list<SSpotAccountInfo>::iterator itE = m_listAccountInfo.end();
	int index = 0;
	while(itB != itE)
	{
		m_listCtrlAccountInfo.InsertItem(index, "");
		m_listCtrlAccountInfo.SetItemText(index, 0, itB->currency.c_str());
		m_listCtrlAccountInfo.SetItemText(index, 1, itB->balance.c_str());
		m_listCtrlAccountInfo.SetItemText(index, 2, itB->hold.c_str());
		if(m_curTickData.bValid)
		{
			double available = stod(itB->available);
			if(itB->currency == m_strFirstCoinType)
			{
				double cnt = available*m_curTickData.last;
				m_listCtrlAccountInfo.SetItemText(index, 3, CFuncCommon::Double2String(cnt+DOUBLE_PRECISION, 5).c_str());
			}
			else if(itB->currency == m_strSecondCoinType)
			{
				double cnt = available/m_curTickData.last;
				m_listCtrlAccountInfo.SetItemText(index, 3, CFuncCommon::Double2String(cnt + DOUBLE_PRECISION, m_nSizeDecimal).c_str());
			}
		}
		
		index++;
		++itB;
	}
}

void CManualOKExSpotDlg::_UpdateTradeShow()
{
	CString szFormat;
	for(int i = 0; i<(int)m_vecTradePairInfo.size(); ++i)
	{
		SSpotTradePairInfo& info = m_vecTradePairInfo[i];
		if(m_listCtrlOrderOpen.GetItemCount() <= i)
		{
			m_listCtrlOrderOpen.InsertItem(i, "");
			m_listCtrlOrderClose.InsertItem(i, "");
		}
		if(info.open.orderID != "")
		{
			szFormat.Format("%d", i+1);
			m_listCtrlOrderOpen.SetItemText(i, 0, szFormat);
			szFormat = CFuncCommon::Double2String(info.open.price+DOUBLE_PRECISION, m_nPriceDecimal).c_str();
			m_listCtrlOrderOpen.SetItemText(i, 1, szFormat);
			m_listCtrlOrderOpen.SetItemText(i, 2, info.open.tradeType.c_str());
			szFormat.Format("%s/%s", info.open.filledSize.c_str(), info.open.size.c_str());
			m_listCtrlOrderOpen.SetItemText(i, 3, szFormat);
			m_listCtrlOrderOpen.SetItemText(i, 4, info.open.status.c_str());
			if(info.open.filledSize != "0")
			{
				int count = stoi(info.open.filledSize);
				if(info.open.tradeType == "buy")
				{
					double calcuPrice = stod(m_mapDepthBuy.rbegin()->second.price);
					double profitPersent = (calcuPrice/info.open.price);
					if(calcuPrice >= info.open.price)
					{
						szFormat.Format("(%s%%)", CFuncCommon::Double2String((profitPersent-1)*100+DOUBLE_PRECISION, 2).c_str());
						m_listCtrlOrderOpen.SetItemText(i, 5, szFormat.GetBuffer());
					}
					else
					{
						szFormat.Format("(-%s%%)", CFuncCommon::Double2String((1-profitPersent)*100+DOUBLE_PRECISION, 2).c_str());
						m_listCtrlOrderOpen.SetItemText(i, 5, szFormat.GetBuffer());
					}

				}
				else if(info.open.tradeType == "sell")
				{
					double calcuPrice = stod(m_mapDepthSell.begin()->second.price);
					double profitPersent = (calcuPrice/info.open.price);
					if(calcuPrice <= info.open.price)
					{
						szFormat.Format("(%s%%)", CFuncCommon::Double2String((1-profitPersent)*100+DOUBLE_PRECISION, 2).c_str());
						m_listCtrlOrderOpen.SetItemText(i, 5, szFormat.GetBuffer());
					}
					else
					{
						szFormat.Format("(-%s%%)", CFuncCommon::Double2String((profitPersent-1)*100+DOUBLE_PRECISION, 2).c_str());
						m_listCtrlOrderOpen.SetItemText(i, 5, szFormat.GetBuffer());
					}
				}
			}
			else
				m_listCtrlOrderOpen.SetItemText(i, 5, "");
			tm _tm;
			localtime_s(&_tm, ((const time_t*)&(info.open.timeStamp)));
			szFormat.Format("%02d-%02d %02d:%02d:%02d", _tm.tm_mon+1, _tm.tm_mday, _tm.tm_hour, _tm.tm_min, _tm.tm_sec);
			m_listCtrlOrderOpen.SetItemText(i, 6, szFormat.GetBuffer());

			if(info.close.orderID != "")
			{
				szFormat.Format("%d", i+1);
				m_listCtrlOrderClose.SetItemText(i, 0, szFormat);
				szFormat = CFuncCommon::Double2String(info.close.price + DOUBLE_PRECISION, m_nPriceDecimal).c_str();
				m_listCtrlOrderClose.SetItemText(i, 1, szFormat);
				szFormat.Format("%s/%s", info.close.filledSize.c_str(), info.close.size.c_str());
				m_listCtrlOrderClose.SetItemText(i, 2, szFormat);
				m_listCtrlOrderClose.SetItemText(i, 3, info.close.status.c_str());
			}
			else if(info.closePlanPrice != "" && info.closePlanSize != "")
			{
				szFormat.Format("%d", i + 1);
				m_listCtrlOrderClose.SetItemText(i, 0, szFormat);
				m_listCtrlOrderClose.SetItemText(i, 1, info.closePlanPrice.c_str());
				m_listCtrlOrderClose.SetItemText(i, 2, info.closePlanSize.c_str());
				m_listCtrlOrderClose.SetItemText(i, 3, "plan");
			}
			else
			{
				m_listCtrlOrderClose.SetItemText(i, 0, "");
				m_listCtrlOrderClose.SetItemText(i, 1, "");
				m_listCtrlOrderClose.SetItemText(i, 2, "");
				m_listCtrlOrderClose.SetItemText(i, 3, "");
			}
		}
		else
		{
			m_listCtrlOrderOpen.SetItemText(i, 0, "");
			m_listCtrlOrderOpen.SetItemText(i, 1, "");
			m_listCtrlOrderOpen.SetItemText(i, 2, "");
			m_listCtrlOrderOpen.SetItemText(i, 3, "");
			m_listCtrlOrderOpen.SetItemText(i, 4, "");
			m_listCtrlOrderOpen.SetItemText(i, 5, "");
			m_listCtrlOrderOpen.SetItemText(i, 6, "");

			m_listCtrlOrderClose.SetItemText(i, 0, "");
			m_listCtrlOrderClose.SetItemText(i, 1, "");
			m_listCtrlOrderClose.SetItemText(i, 2, "");
			m_listCtrlOrderClose.SetItemText(i, 3, "");
		}

	}
	if(m_listCtrlOrderOpen.GetItemCount() > (int)m_vecTradePairInfo.size())
	{
		for(int i = m_vecTradePairInfo.size(); i<m_listCtrlOrderOpen.GetItemCount(); ++i)
		{
			m_listCtrlOrderOpen.SetItemText(i, 0, "");
			m_listCtrlOrderOpen.SetItemText(i, 1, "");
			m_listCtrlOrderOpen.SetItemText(i, 2, "");
			m_listCtrlOrderOpen.SetItemText(i, 3, "");
			m_listCtrlOrderOpen.SetItemText(i, 4, "");
			m_listCtrlOrderOpen.SetItemText(i, 5, "");
			m_listCtrlOrderOpen.SetItemText(i, 6, "");

			m_listCtrlOrderClose.SetItemText(i, 0, "");
			m_listCtrlOrderClose.SetItemText(i, 1, "");
			m_listCtrlOrderClose.SetItemText(i, 2, "");
			m_listCtrlOrderClose.SetItemText(i, 3, "");
		}
	}
}

void CManualOKExSpotDlg::_UpdateDepthShow()
{
	const int showLines = 8;
	m_listCtrlDepth.DeleteAllItems();
	int sellLine = min(m_mapDepthSell.size(), showLines);
	for(int i = 0; i < sellLine; ++i)
	{
		m_listCtrlDepth.InsertItem(i, "");
	}
	std::map<std::string, SSpotDepth>::iterator itB = m_mapDepthSell.begin();
	std::map<std::string, SSpotDepth>::iterator itE = m_mapDepthSell.end();
	CString szFormat;
	int count = 0;
	while(itB != itE)
	{
		szFormat.Format("%d", count + 1);
		m_listCtrlDepth.SetItemText(sellLine - 1 - count, 0, szFormat);
		szFormat.Format("%s", itB->first.c_str());
		m_listCtrlDepth.SetItemText(sellLine - 1 - count, 1, szFormat);
		szFormat.Format("%s", itB->second.showSize.c_str());
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
		std::map<std::string, SSpotDepth>::iterator itB = m_mapDepthBuy.begin();
		std::map<std::string, SSpotDepth>::iterator itE = m_mapDepthBuy.end();
		CString _szFormat;
		int count = 0;
		itE--;
		while(itB != itE)
		{
			_szFormat.Format("%d", count + 1);
			m_listCtrlDepth.SetItemText(sellLine + 1 + count, 0, _szFormat);
			_szFormat.Format("%s", itE->first.c_str());
			m_listCtrlDepth.SetItemText(sellLine + 1 + count, 1, _szFormat);
			_szFormat.Format("%s", itE->second.showSize.c_str());
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
			_szFormat.Format("%s", itB->second.showSize.c_str());
			m_listCtrlDepth.SetItemText(sellLine + 1 + count, 2, _szFormat);
		}
	}
}

void CManualOKExSpotDlg::SetHScroll()
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

void CManualOKExSpotDlg::OnBnClickedButtonBuy()
{
	std::string temp = "buy";
	_OpenOrder(temp);
}


void CManualOKExSpotDlg::OnBnClickedButtonSell()
{
	std::string temp = "sell";
	_OpenOrder(temp);
}

int CManualOKExSpotDlg::_GetFreeOrderIndex()
{
	for(int i = 0; i<(int)m_vecTradePairInfo.size(); ++i)
	{
		if(m_vecTradePairInfo[i].open.orderID == "" && m_vecTradePairInfo[i].open.strClientOrderID == "")
			return i;
	}
	return -1;
}

void CManualOKExSpotDlg::_OpenOrder(std::string& type)
{
	if(!m_bRun)
	{
		MessageBox("未订阅行情");
		return;
	}
	if(!__SaveTradeConfigCtrl())
		return;
	std::string price;
	CString tradePrice;
	m_editTradePrice.GetWindowText(tradePrice);
	if(tradePrice == "")
		price = "-1";
	else
		price = tradePrice.GetBuffer();
	std::string strClientOrderID = CFuncCommon::GenUUID();
	OKEX_HTTP->API_SpotTrade(true, m_strInstrumentType, type, price, m_strSpotTradeSize, strClientOrderID);
	SSpotTradePairInfo* pInfo;
	int nIndex = _GetFreeOrderIndex();
	if(nIndex == -1)
	{
		SSpotTradePairInfo temp;
		m_vecTradePairInfo.push_back(temp);
		pInfo = &m_vecTradePairInfo[m_vecTradePairInfo.size()-1];
	}
	else
		pInfo = &m_vecTradePairInfo[nIndex];
	pInfo->open.strClientOrderID = strClientOrderID;
	pInfo->open.waitClientOrderIDTime = time(NULL);
	pInfo->open.tradeType = type;
	CActionLog("trade", "下%s单%s个, price=%s, client_oid=%s", type.c_str(), m_strSpotTradeSize.c_str(), price.c_str(), strClientOrderID.c_str());
}

void CManualOKExSpotDlg::_SaveData()
{
	std::string strFilePath = "./save.txt";
	std::ofstream stream(strFilePath);
	if(!stream.is_open())
		return;
	std::vector<SSpotTradePairInfo>::iterator itB = m_vecTradePairInfo.begin();
	std::vector<SSpotTradePairInfo>::iterator itE = m_vecTradePairInfo.end();
	while(itB != itE)
	{
		if(itB->open.orderID != "")
		{
			stream << itB->open.orderID << "	" << itB->open.strClientOrderID << "	";
			if(itB->open.bModifyQTY)
				stream << itB->open.filledSize;
			else
				stream << "0";
			if(itB->closePlanPrice != "" && itB->closePlanSize != "")
				stream << "	" << itB->closePlanPrice << "	" << itB->closePlanSize;
			else
				stream << "	0	0";
			if(itB->close.orderID != "")
				stream  << "	" << itB->close.orderID << "	" << itB->close.strClientOrderID;
			else
				stream << "	0	0";
			stream << std::endl;
		}
		++itB;
	}
	stream.close();
}

void CManualOKExSpotDlg::OnBnClickedButtonClose()
{
	// TODO:  在此添加控件通知处理程序代码
	bool bUpdate = false;
	for(int i = 0; i<(int)m_vecTradePairInfo.size(); ++i)
	{
		SSpotTradePairInfo& info = m_vecTradePairInfo[i];
		if(info.open.orderID != "")
		{
			if(m_listCtrlOrderOpen.GetCheck(i))
			{
				if(info.close.orderID == "")
				{
					std::string price;
					CString closePrice;
					m_editClosePrice.GetWindowText(closePrice);
					if(closePrice == "")
						price = "-1";
					else
						price = closePrice.GetBuffer();

					std::string size;
					CString closeSize;
					m_editCloseSize.GetWindowText(closeSize);
					if(closeSize == "")
						size = info.open.filledSize;
					else
						size = closeSize.GetBuffer();
					if(info.open.status == "open")
					{
						if(price == "-1")
							return;
						info.closePlanPrice = price;
						if(closeSize == "")
							info.closePlanSize = info.open.size;
						else
							info.closePlanSize = size;
						bUpdate = true;
					}
					else if(info.open.status == "part_filled")
					{
						OKEX_HTTP->API_SpotCancelOrder(true, m_strInstrumentType, info.open.orderID);
						
						std::string strClientOrderID = CFuncCommon::GenUUID();
						std::string tradeType;
						if(info.open.tradeType == "buy")
							tradeType = "sell";
						else
							tradeType = "buy";
						OKEX_HTTP->API_SpotTrade(true, m_strInstrumentType, tradeType, price, size, strClientOrderID);
						info.close.strClientOrderID = strClientOrderID;
						info.close.waitClientOrderIDTime = time(NULL);
						info.close.tradeType = tradeType;
						CActionLog("trade", "开平仓单");
					}
					else if(info.open.status == "filled" || info.open.status == "cancelled")
					{
						std::string strClientOrderID = CFuncCommon::GenUUID();
						std::string tradeType;
						if(info.open.tradeType == "buy")
							tradeType = "sell";
						else
							tradeType = "buy";
						OKEX_HTTP->API_SpotTrade(true, m_strInstrumentType, tradeType, price, size, strClientOrderID);
						info.close.strClientOrderID = strClientOrderID;
						info.close.waitClientOrderIDTime = time(NULL);
						info.close.tradeType = tradeType;
						CActionLog("trade", "开平仓单");
					}
				}
			}
		}
	}
	int cnt = m_listCtrlOrderOpen.GetItemCount();
	for(int i = 0; i<cnt; ++i)
	{
		m_listCtrlOrderOpen.SetCheck(i, FALSE);
	}
	cnt = m_listCtrlOrderClose.GetItemCount();
	for(int i = 0; i<cnt; ++i)
	{
		m_listCtrlOrderClose.SetCheck(i, FALSE);
	}
	if(bUpdate)
		_SaveData();
}


void CManualOKExSpotDlg::OnBnClickedButtonCancel()
{
	bool bUpdate = false;
	for(int i = 0; i<(int)m_vecTradePairInfo.size(); ++i)
	{
		SSpotTradePairInfo& info = m_vecTradePairInfo[i];
		if(info.open.orderID != "")
		{
			if(m_listCtrlOrderOpen.GetCheck(i))
			{
				if(info.open.status == "open" || info.open.status == "part_filled")
					OKEX_HTTP->API_SpotCancelOrder(true, m_strInstrumentType, info.open.orderID);
			}
			if(m_listCtrlOrderClose.GetCheck(i))
			{
				if(info.close.orderID != "")
				{
					if(info.close.status == "open" || info.close.status == "part_filled")
						OKEX_HTTP->API_SpotCancelOrder(true, m_strInstrumentType, info.close.orderID);
				}
				else if(info.closePlanPrice != "" && info.closePlanSize != "")
				{
					info.closePlanPrice = "";
					info.closePlanSize = "";
					bUpdate = true;
				}
			}
		}
	}
	int cnt = m_listCtrlOrderOpen.GetItemCount();
	for(int i = 0; i<cnt; ++i)
	{
		m_listCtrlOrderOpen.SetCheck(i, FALSE);
	}
	cnt = m_listCtrlOrderClose.GetItemCount();
	for(int i = 0; i<cnt; ++i)
	{
		m_listCtrlOrderClose.SetCheck(i, FALSE);
	}
	if(bUpdate)
		_SaveData();
}


void CManualOKExSpotDlg::_CheckAllOrder()
{
	time_t tNow = time(NULL);
	bool bUpdate = false;
	std::vector<SSpotTradePairInfo>::iterator itB = m_vecTradePairInfo.begin();
	std::vector<SSpotTradePairInfo>::iterator itE = m_vecTradePairInfo.end();
	while(itB != itE)
	{
		if(itB->open.orderID != "")
		{
			if(itB->open.status == "cancelled")
			{
				if(itB->close.orderID == "")
				{
					if(itB->open.filledSize == "0")
					{
						itB->Reset();
						bUpdate = true;
					}
				}
				else
				{
					if(itB->close.status == "cancelled")
					{
						if(itB->close.filledSize == "0")
						{
							itB->close.Reset();
							bUpdate = true;
						}
						else
						{
							int openQTY = stoi(itB->open.filledSize);
							int closeQTY = stoi(itB->close.filledSize);
							int last = openQTY - closeQTY;
							if(last)
							{
								itB->open.filledSize = CFuncCommon::ToString(last);
								itB->open.bModifyQTY = true;
								bUpdate = true;
								itB->close.Reset();
							}
						}
					}
					else if(itB->close.status == "filled")
					{
						int openQTY = stoi(itB->open.filledSize);
						int closeQTY = stoi(itB->close.filledSize);
						int last = openQTY - closeQTY;
						if(last)
						{
							itB->open.filledSize = CFuncCommon::ToString(last);
							itB->open.bModifyQTY = true;
							bUpdate = true;
							itB->close.Reset();
						}
						else
						{
							itB->Reset();
							bUpdate = true;
						}
					}
				}

			}
			else if(itB->open.status == "filled")
			{
				if(itB->close.orderID != "")
				{
					if(itB->close.status == "cancelled")
					{
						if(itB->close.filledSize == "0")
						{
							itB->close.Reset();
							bUpdate = true;
						}
						else
						{
							int openQTY = stoi(itB->open.filledSize);
							int closeQTY = stoi(itB->close.filledSize);
							int last = openQTY - closeQTY;
							if(last == 0)
							{
								itB->Reset();
								bUpdate = true;
							}
							else
							{
								itB->open.filledSize = CFuncCommon::ToString(last);
								itB->open.bModifyQTY = true;
								bUpdate = true;
								itB->close.Reset();
							}
						}
					}
					else if(itB->close.status == "filled")
					{
						int openQTY = stoi(itB->open.filledSize);
						int closeQTY = stoi(itB->close.filledSize);
						int last = openQTY - closeQTY;
						if(last == 0)
						{
							itB->Reset();
							bUpdate = true;
						}
						else
						{
							itB->open.filledSize = CFuncCommon::ToString(last);
							itB->open.bModifyQTY = true;
							bUpdate = true;
							itB->close.Reset();
						}
					}
				}
				else
				{
					
					if(itB->closePlanPrice != "" && itB->closePlanSize != "" && tNow-itB->open.tLastALLFillTime > 3)
					{
						std::string strClientOrderID = CFuncCommon::GenUUID();
						std::string tradeType;
						if(itB->open.tradeType == "buy")
							tradeType = "sell";
						else
							tradeType = "buy";
						OKEX_HTTP->API_SpotTrade(true, m_strInstrumentType, tradeType, itB->closePlanPrice, itB->closePlanSize, strClientOrderID);
						itB->close.strClientOrderID = strClientOrderID;
						itB->close.waitClientOrderIDTime = time(NULL);
						itB->close.tradeType = tradeType;
						CActionLog("trade", "开平仓单");
						itB->closePlanPrice = "";
						itB->closePlanSize = "";
					}
				}
			}
		}
		itB++;
	}
	if(bUpdate)
		_SaveData();
}


bool CManualOKExSpotDlg::CheckDepthInfo(int checkNum, std::string& checkSrc)
{
	std::map<std::string, SSpotDepth>::reverse_iterator itBB = m_mapDepthBuy.rbegin();
	std::map<std::string, SSpotDepth>::reverse_iterator itBE = m_mapDepthBuy.rend();
	std::map<std::string, SSpotDepth>::iterator itSB = m_mapDepthSell.begin();
	std::map<std::string, SSpotDepth>::iterator itSE = m_mapDepthSell.end();
	int nIndex = 0;
	checkSrc = "";
	while(nIndex < 25)
	{
		bool have = false;
		if(itBB != itBE)
		{
			if(nIndex != 0)
				checkSrc.append(":");
			checkSrc.append(itBB->second.price).append(":").append(itBB->second.size);
			have = true;
			++itBB;
		}
		if(itSB != itSE)
		{
			if(nIndex != 0 || have)
				checkSrc.append(":");
			checkSrc.append(itSB->second.price).append(":").append(itSB->second.size);
			++itSB;
		}
		++nIndex;
	}
	int crc = CFuncCommon::crc32((const unsigned char*)checkSrc.c_str(), checkSrc.size());
	if(checkNum != crc)
	{
		LOCAL_ERROR("crc校验失败 checknum=%d local=%d", checkNum, crc);
		OKEX_WEB_SOCKET->API_EntrustDepth(false, m_strInstrumentType);
		m_tWaitNewSubDepth = time(NULL);
		m_bWaitDepthBegin = true;
		return false;
	}
	return true;
}


void CManualOKExSpotDlg::OnBnClickedButtonClearFreeLine()
{
	std::vector<SSpotTradePairInfo>::iterator itB = m_vecTradePairInfo.begin();
	while(itB != m_vecTradePairInfo.end())
	{
		if(itB->open.strClientOrderID == "" && itB->open.orderID == "" && itB->close.strClientOrderID == "" && itB->close.orderID == "")
		{
			itB = m_vecTradePairInfo.erase(itB);
			continue;
		}
		++itB;
	}
	_UpdateTradeShow();
}

bool CompareLowPrice(const SSpotTradePairInfo &value1, const SSpotTradePairInfo &value2)
{
	return value1.open.price < value2.open.price;
}

bool CompareHightPrice(const SSpotTradePairInfo &value1, const SSpotTradePairInfo &value2)
{
	return value1.open.price > value2.open.price;
}

void CManualOKExSpotDlg::OnBnClickedButtonBearFirst()
{
	std::vector<SSpotTradePairInfo> bearFinish;
	std::vector<SSpotTradePairInfo> bearNotFinish;
	std::vector<SSpotTradePairInfo> bullFinish;
	std::vector<SSpotTradePairInfo> bullNotFinish;

	for(int i = 0; i<(int)m_vecTradePairInfo.size(); ++i)
	{
		if(m_vecTradePairInfo[i].open.tradeType == "buy")
		{
			if(m_vecTradePairInfo[i].open.status == "filled" || m_vecTradePairInfo[i].open.status == "part_filled")
				bullFinish.push_back(m_vecTradePairInfo[i]);
			else
				bullNotFinish.push_back(m_vecTradePairInfo[i]);
				
		}
		else if(m_vecTradePairInfo[i].open.tradeType == "sell")
		{
			if(m_vecTradePairInfo[i].open.status == "filled" || m_vecTradePairInfo[i].open.status == "part_filled")
				bearFinish.push_back(m_vecTradePairInfo[i]);
			else
				bearNotFinish.push_back(m_vecTradePairInfo[i]);
		}
	}
	std::sort(bearFinish.begin(), bearFinish.end(), CompareHightPrice);
	std::sort(bearNotFinish.begin(), bearNotFinish.end(), CompareLowPrice);
	std::sort(bullFinish.begin(), bullFinish.end(), CompareLowPrice);
	std::sort(bullNotFinish.begin(), bullNotFinish.end(), CompareHightPrice);
	m_vecTradePairInfo.clear();
	std::vector<SSpotTradePairInfo>::iterator itB = bearFinish.begin();
	std::vector<SSpotTradePairInfo>::iterator itE = bearFinish.end();
	while(itB != itE)
	{
		m_vecTradePairInfo.push_back(*itB);
		++itB;
	}
	itB = bullFinish.begin();
	itE = bullFinish.end();
	while(itB != itE)
	{
		m_vecTradePairInfo.push_back(*itB);
		++itB;
	}
	itB = bearNotFinish.begin();
	itE = bearNotFinish.end();
	while(itB != itE)
	{
		m_vecTradePairInfo.push_back(*itB);
		++itB;
	}
	itB = bullNotFinish.begin();
	itE = bullNotFinish.end();
	while(itB != itE)
	{
		m_vecTradePairInfo.push_back(*itB);
		++itB;
	}
	_UpdateTradeShow();
}


void CManualOKExSpotDlg::OnBnClickedButtonBullFirst()
{
	std::list<SSpotTradePairInfo> bearFinish;
	std::list<SSpotTradePairInfo> bearNotFinish;
	std::list<SSpotTradePairInfo> bullFinish;
	std::list<SSpotTradePairInfo> bullNotFinish;
	for(int i = 0; i<(int)m_vecTradePairInfo.size(); ++i)
	{
		if(m_vecTradePairInfo[i].open.tradeType == "buy")
		{
			if(m_vecTradePairInfo[i].open.status == "filled")
				bullFinish.push_back(m_vecTradePairInfo[i]);
			else
				bullNotFinish.push_back(m_vecTradePairInfo[i]);

		}
		else if(m_vecTradePairInfo[i].open.tradeType == "sell")
		{
			if(m_vecTradePairInfo[i].open.status == "filled")
				bearFinish.push_back(m_vecTradePairInfo[i]);
			else
				bearNotFinish.push_back(m_vecTradePairInfo[i]);
		}
	}
	m_vecTradePairInfo.clear();
	std::list<SSpotTradePairInfo>::iterator itB = bullFinish.begin();
	std::list<SSpotTradePairInfo>::iterator itE = bullFinish.end();
	while(itB != itE)
	{
		m_vecTradePairInfo.push_back(*itB);
		++itB;
	}
	itB = bearFinish.begin();
	itE = bearFinish.end();
	while(itB != itE)
	{
		m_vecTradePairInfo.push_back(*itB);
		++itB;
	}
	itB = bullNotFinish.begin();
	itE = bullNotFinish.end();
	while(itB != itE)
	{
		m_vecTradePairInfo.push_back(*itB);
		++itB;
	}
	itB = bearNotFinish.begin();
	itE = bearNotFinish.end();
	while(itB != itE)
	{
		m_vecTradePairInfo.push_back(*itB);
		++itB;
	}
	_UpdateTradeShow();
}
