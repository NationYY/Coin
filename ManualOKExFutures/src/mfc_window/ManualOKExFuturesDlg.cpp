
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

	m_bWaitDepthBegin = true;
	m_tListenPong = 0;
	m_bRun = false;
	m_bSwapFutures = false;
	m_apiKey = "";
	m_secretKey = "";
	m_passphrase = "";
	m_nPriceDecimal = 0;
	m_tWaitNewSubDepth = 0;
	m_strCoinType = "";
	m_strFuturesCycle = "";
	m_strFuturesTradeSize = "";
	m_strLeverage = "";
	m_dLeverage = 0.0;
	m_beginMoney = 0.0;
}

void CManualOKExFuturesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_OPEN, m_listCtrlOrderOpen);
	DDX_Control(pDX, IDC_LIST_CLOSE, m_listCtrlOrderClose);
	DDX_Control(pDX, IDC_COMBO1, m_combCoinType);
	DDX_Control(pDX, IDC_EDIT1, m_editFuturesCycle);
	DDX_Control(pDX, IDC_EDIT2, m_editFuturesTradeSize);
	DDX_Control(pDX, IDC_COMBO3, m_combFuturesType);
	DDX_Control(pDX, IDC_EDIT3, m_editCapital);
	DDX_Control(pDX, IDC_LIST1, m_ctrlListLog);
	DDX_Control(pDX, IDC_LIST_CLOSE2, m_listCtrlDepth);
	DDX_Control(pDX, IDC_EDIT5, m_editOpenPrice);
	DDX_Control(pDX, IDC_STATIC_PROFIT, m_staticProfit);
	DDX_Control(pDX, IDC_STATIC_ACCOUNT, m_staticAccountInfo);
	DDX_Control(pDX, IDC_EDIT6, m_editClosePrice);
	DDX_Control(pDX, IDC_EDIT7, m_editCloseSize);
	DDX_Control(pDX, IDC_LIST_OPEN2, m_listCtrlPostionInfo);
	DDX_Control(pDX, IDC_STATIC_ACCOUNT2, m_staticAccountAvailInfo);
	DDX_Control(pDX, IDC_STATIC_ACCOUNT3, m_staticCanOpenSize);
	DDX_Control(pDX, IDC_COMBO4, m_combMoneyType);
	DDX_Control(pDX, IDC_EDIT8, m_editTriggerPrice);
	DDX_Control(pDX, IDC_EDIT9, m_editLeverage);
}

BEGIN_MESSAGE_MAP(CManualOKExFuturesDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDOK, &CManualOKExFuturesDlg::OnBnClickedStart)
	ON_BN_CLICKED(IDC_BUTTON1, &CManualOKExFuturesDlg::OnBnClickedButtonOpenBull)
	ON_BN_CLICKED(IDC_BUTTON2, &CManualOKExFuturesDlg::OnBnClickedButtonOpenBear)
	ON_BN_CLICKED(IDC_BUTTON3, &CManualOKExFuturesDlg::OnBnClickedButtonClose)
	ON_BN_CLICKED(IDC_BUTTON4, &CManualOKExFuturesDlg::OnBnClickedButtonCancel)
	ON_BN_CLICKED(IDC_BUTTON5, &CManualOKExFuturesDlg::OnBnClickedButtonUpdateBeginMoney)
	ON_BN_CLICKED(IDC_BUTTON6, &CManualOKExFuturesDlg::OnBnClickedButtonSaveProfit)
	ON_BN_CLICKED(IDC_BUTTON7, &CManualOKExFuturesDlg::OnBnClickedButtonClearFreeLine)
	ON_BN_CLICKED(IDC_BUTTON8, &CManualOKExFuturesDlg::OnBnClickedButtonBearFirst)
	ON_BN_CLICKED(IDC_BUTTON9, &CManualOKExFuturesDlg::OnBnClickedButtonBullFirst)
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
	m_combCoinType.InsertString(4, "XRP");
	m_combCoinType.InsertString(5, "EOS");
	m_combCoinType.InsertString(6, "BCH");
	m_combCoinType.InsertString(7, "TRX");
	m_combCoinType.InsertString(8, "BSV");
	m_combCoinType.InsertString(9, "DOT");

	m_combMoneyType.InsertString(0, "USD");
	m_combMoneyType.InsertString(1, "USDT");

	m_combFuturesType.InsertString(0, "交割合约");
	m_combFuturesType.InsertString(1, "永续合约");

	m_listCtrlOrderOpen.InsertColumn(0, "序号", LVCFMT_CENTER, 40);
	m_listCtrlOrderOpen.InsertColumn(1, "价格", LVCFMT_CENTER, 80);
	m_listCtrlOrderOpen.InsertColumn(2, "类型", LVCFMT_CENTER, 45);
	m_listCtrlOrderOpen.InsertColumn(3, "成交量", LVCFMT_CENTER, 70);
	m_listCtrlOrderOpen.InsertColumn(4, "状态", LVCFMT_CENTER, 45);
	m_listCtrlOrderOpen.InsertColumn(5, "参考利润", LVCFMT_CENTER, 120);
	m_listCtrlOrderOpen.InsertColumn(6, "下单时间", LVCFMT_CENTER, 115);
	m_listCtrlOrderOpen.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);


	m_listCtrlOrderClose.InsertColumn(0, "序号", LVCFMT_CENTER, 40);
	m_listCtrlOrderClose.InsertColumn(1, "价格", LVCFMT_CENTER, 75);
	m_listCtrlOrderClose.InsertColumn(2, "成交量", LVCFMT_CENTER, 70);
	m_listCtrlOrderClose.InsertColumn(3, "状态", LVCFMT_CENTER, 45);
	m_listCtrlOrderClose.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);

	m_listCtrlDepth.InsertColumn(0, "", LVCFMT_CENTER, 30);
	m_listCtrlDepth.InsertColumn(1, "价", LVCFMT_CENTER, 70);
	m_listCtrlDepth.InsertColumn(2, "量", LVCFMT_CENTER, 80);
	m_listCtrlDepth.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);


	m_listCtrlPostionInfo.InsertColumn(0, "多仓", LVCFMT_CENTER, 55);
	m_listCtrlPostionInfo.InsertColumn(1, "多可平", LVCFMT_CENTER, 54);
	m_listCtrlPostionInfo.InsertColumn(2, "多均价", LVCFMT_CENTER, 65);
	m_listCtrlPostionInfo.InsertColumn(3, "空仓", LVCFMT_CENTER, 55);
	m_listCtrlPostionInfo.InsertColumn(4, "空可平", LVCFMT_CENTER, 54);
	m_listCtrlPostionInfo.InsertColumn(5, "空均价", LVCFMT_CENTER, 65);
	m_listCtrlPostionInfo.InsertColumn(6, "爆仓价", LVCFMT_CENTER, 65);
	m_listCtrlPostionInfo.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	// TODO:  在此添加额外的初始化代码
	if(!m_config.open("./config.ini"))
		return FALSE;
	m_apiKey = m_config.get("futures", "apiKey", "");
	m_secretKey = m_config.get("futures", "secretKey", "");
	m_passphrase = m_config.get("futures", "passphrase", "");
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

void CManualOKExFuturesDlg::__InitBaseConfigCtrl()
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
	else if(strTemp == "XRP")
		m_combCoinType.SetCurSel(4);
	else if(strTemp == "EOS")
		m_combCoinType.SetCurSel(5);
	else if(strTemp == "BCH")
		m_combCoinType.SetCurSel(6);
	else if(strTemp == "TRX")
		m_combCoinType.SetCurSel(7);
	else if(strTemp == "BSV")
		m_combCoinType.SetCurSel(8);
	else if(strTemp == "DOT")
		m_combCoinType.SetCurSel(9);

	strTemp = m_config.get("futures", "moneyType", "");
	if(strTemp == "USD")
		m_combMoneyType.SetCurSel(0);
	else if(strTemp == "USDT")
		m_combMoneyType.SetCurSel(1);
	strTemp = m_config.get("futures", "futuresCycle", "");
	m_editFuturesCycle.SetWindowText(strTemp.c_str());
	strTemp = m_config.get("futures", "leverage", "");
	m_editLeverage.SetWindowText(strTemp.c_str());
	strTemp = m_config.get("futures", "futuresType", "");
	if(strTemp == "交割合约")
		m_combFuturesType.SetCurSel(0);
	else if(strTemp == "永续合约")
		m_combFuturesType.SetCurSel(1);
	strTemp = m_config.get("futures", "beginMoney", "");
	m_editCapital.SetWindowText(strTemp.c_str());
}

void CManualOKExFuturesDlg::__InitTradeConfigCtrl()
{
	std::string strTemp = m_config.get("futures", "futuresTradeSize", "");
	m_editFuturesTradeSize.SetWindowText(strTemp.c_str());
	m_strFuturesTradeSize = strTemp.c_str();
}

bool CManualOKExFuturesDlg::__SaveBaseConfigCtrl()
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
	CString strLeverage;
	m_editLeverage.GetWindowText(strLeverage);
	if(strLeverage == "")
	{
		MessageBox("未填写杠杆");
		return false;
	}
	CString strMoneyType;
	m_combMoneyType.GetWindowText(strMoneyType);
	if(strMoneyType == "")
	{
		MessageBox("未选择货币类型");
		return false;
	}

	CString szCost = "";
	m_editCapital.GetWindowText(szCost);

	m_strCoinType = strCoinType.GetBuffer();
	m_strMoneyType = strMoneyType.GetBuffer();
	m_strFuturesCycle = strFuturesCycle.GetBuffer();
	m_strLeverage = strLeverage.GetBuffer();
	m_dLeverage = stod(m_strLeverage);
	if(strFuturesType == "永续合约")
		m_bSwapFutures = true;
	else
		m_bSwapFutures = false;
	if(szCost != "")
		m_beginMoney = stod(szCost.GetBuffer());

	m_config.set_value("futures", "coinType", m_strCoinType.c_str());
	m_config.set_value("futures", "moneyType", m_strMoneyType.c_str());
	m_config.set_value("futures", "futuresCycle", m_strFuturesCycle.c_str());
	m_config.set_value("futures", "leverage", m_strLeverage.c_str());
	m_config.set_value("futures", "futuresType", strFuturesType.GetBuffer());
	m_config.set_value("futures", "beginMoney", szCost.GetBuffer());
	m_config.save("./config.ini");

	return true;
}

bool CManualOKExFuturesDlg::__SaveTradeConfigCtrl()
{
	CString strFuturesTradeSize;
	m_editFuturesTradeSize.GetWindowText(strFuturesTradeSize);
	m_strFuturesTradeSize = strFuturesTradeSize.GetBuffer();
	m_config.set_value("futures", "futuresTradeSize", m_strFuturesTradeSize.c_str());
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
			if(m_tWaitNewSubDepth && tNow - m_tWaitNewSubDepth >= 5)
			{
				OKEX_WEB_SOCKET->API_FuturesEntrustDepth(true, m_bSwapFutures, m_strCoinType, m_strMoneyType, m_strFuturesCycle);
				m_tWaitNewSubDepth = 0;
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
				 OKEX_HTTP->API_FuturesAccountInfoByCurrency(true, m_bSwapFutures, m_strCoinType, m_strMoneyType);
		}
		break;
	}

	CDialog::OnTimer(nIDEvent);
}


void CManualOKExFuturesDlg::OnBnClickedStart()
{
	if(!__SaveBaseConfigCtrl())
		return;
	if(m_bRun)
		return;
	bool bFound = false;
	std::string instrumentID;
	if(m_bSwapFutures)
		instrumentID = m_strCoinType + "-" + m_strMoneyType + "-SWAP";
	else
		instrumentID = m_strCoinType + "-" + m_strMoneyType + "-" + m_strFuturesCycle;
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
		OKEX_HTTP->API_FuturesSetLeverage(false, m_bSwapFutures, m_strCoinType, m_strMoneyType, m_strLeverage, &resInfo);
		if(m_bSwapFutures)
		{
			std::string strInstrumentID = m_strCoinType + "-" + m_strMoneyType + "-SWAP";
			if(!resInfo.retObj.isObject() || ((resInfo.retObj["instrument_id"].asString() != strInstrumentID) && (resInfo.retObj["code"].asInt() != 35017)))
			{
				MessageBox("设置杠杆失败");
				return;
			}
		}
		else
		{
			if(resInfo.retObj.isObject())
				MessageBox("not object");
			if(resInfo.retObj["result"].isString())
				MessageBox("result is string");
			if(!resInfo.retObj.isObject() || (resInfo.retObj["result"].asString() != "true"))
			{
				MessageBox("设置杠杆失败");
				return;
			}
		}
	}
	if(OKEX_WEB_SOCKET)
	{
		OKEX_WEB_SOCKET->API_FuturesTickerData(true, m_bSwapFutures, m_strCoinType, m_strMoneyType, m_strFuturesCycle);
		OKEX_WEB_SOCKET->API_LoginFutures(m_apiKey, m_secretKey, time(NULL));
		OKEX_WEB_SOCKET->API_FuturesEntrustDepth(true, m_bSwapFutures, m_strCoinType, m_strMoneyType, m_strFuturesCycle);
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
	std::vector<SFuturesTradePairInfo>::iterator itB = m_vecTradePairInfo.begin();
	std::vector<SFuturesTradePairInfo>::iterator itE = m_vecTradePairInfo.end();
	while(itB != itE)
	{
		if(itB->strTriggerPrice != "")
		{
			if(itB->open.tradeType == eFuturesTradeType_OpenBull)
			{
				if(m_curTickData.last <= itB->triggerPrice)
				{
					OKEX_HTTP->API_FuturesTrade(true, m_bSwapFutures, itB->open.tradeType, m_strCoinType, m_strMoneyType, m_strFuturesCycle, itB->openPrice, itB->openSize, m_strLeverage, itB->open.strClientOrderID);
					CActionLog("trade", "条件触发 开%s单%s张, price=%s, client_oid=%s", ((itB->open.tradeType == eFuturesTradeType_OpenBull) ? "多" : "空"), itB->openSize.c_str(), itB->openPrice.c_str(), itB->open.strClientOrderID.c_str());

					itB->triggerPrice = 0.0;
					itB->strTriggerPrice = "";
					itB->openSize = "";
				}
			}
			else if(itB->open.tradeType == eFuturesTradeType_OpenBear)
			{
				if(m_curTickData.last >= itB->triggerPrice)
				{
					OKEX_HTTP->API_FuturesTrade(true, m_bSwapFutures, itB->open.tradeType, m_strCoinType, m_strMoneyType, m_strFuturesCycle, itB->openPrice, itB->openSize, m_strLeverage, itB->open.strClientOrderID);
					CActionLog("trade", "条件触发 开%s单%s张, price=%s, client_oid=%s", ((itB->open.tradeType == eFuturesTradeType_OpenBull) ? "多" : "空"), itB->openSize.c_str(), itB->openPrice.c_str(), itB->open.strClientOrderID.c_str());

					itB->triggerPrice = 0.0;
					itB->strTriggerPrice = "";
					itB->openSize = "";
				}
			}
		}
		++itB;
	}
	_UpdateTradeShow();
	_UpdateAccountShow();
}

void CManualOKExFuturesDlg::Pong()
{
	m_tListenPong = 0;
}

void CManualOKExFuturesDlg::OnLoginSuccess()
{
	OKEX_WEB_SOCKET->API_FuturesOrderInfo(true, m_bSwapFutures, m_strCoinType, m_strMoneyType, m_strFuturesCycle);
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
		SFuturesTradePairInfo info;
		if(strcmp(szOpenOrderID, "0") != 0)
		{
			info.open.strClientOrderID = szOpenClientID;
			info.open.orderID = szOpenOrderID;
			SHttpResponse resInfo;
			OKEX_HTTP->API_FuturesOrderInfo(false, m_bSwapFutures, m_strCoinType, m_strMoneyType, m_strFuturesCycle, info.open.orderID, &resInfo);
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
				else
					LOCAL_ERROR("未知交易类型[%s]", tradeType.c_str());
				if(strcmp(szOpenQTY, "0") != 0)
				{
					info.open.filledQTY = szOpenQTY;
					info.open.bModifyQTY = true;
				}
				CActionLog("trade", "http更新订单信息 client_order=%s, order=%s, filledQTY=%s, price=%s, priceAvg=%s, status=%s, tradeType=%s", info.open.strClientOrderID.c_str(), info.open.orderID.c_str(), info.open.filledQTY.c_str(), retObj["price"].asString().c_str(), retObj["price_avg"].asString().c_str(), info.open.status.c_str(), tradeType.c_str());
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
			OKEX_HTTP->API_FuturesOrderInfo(false, m_bSwapFutures, m_strCoinType, m_strMoneyType, m_strFuturesCycle, info.close.orderID, &resInfo);
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
				else
					LOCAL_ERROR("未知交易类型[%s]", tradeType.c_str());
				CActionLog("trade", "http更新订单信息 client_order=%s, order=%s, filledQTY=%s, price=%s, priceAvg=%s, status=%s, tradeType=%s", info.close.strClientOrderID.c_str(), info.close.orderID.c_str(), info.close.filledQTY.c_str(), retObj["price"].asString().c_str(), retObj["price_avg"].asString().c_str(), info.close.status.c_str(), tradeType.c_str());
			}
		}
		if(strcmp(szOpenOrderID, "0") != 0)
			m_vecTradePairInfo.push_back(info);
	}
	stream.close();
	_UpdateTradeShow();
	SHttpResponse resInfo;
	OKEX_HTTP->API_FuturesPositionInfo(false, m_bSwapFutures, m_strCoinType, m_strMoneyType, m_strFuturesCycle, &resInfo);
	if(resInfo.retObj["holding"].isArray())
	{
		SFuturesPositionInfo positionInfo;
		if(m_bSwapFutures)
		{


		}
		else
		{
			positionInfo.bullCount = resInfo.retObj["holding"][0]["long_qty"].asString();
			positionInfo.bullFreeCount = resInfo.retObj["holding"][0]["long_avail_qty"].asString();
			positionInfo.bullPriceAvg = resInfo.retObj["holding"][0]["long_avg_cost"].asString();
			positionInfo.bearCount = resInfo.retObj["holding"][0]["short_qty"].asString();
			positionInfo.bearFreeCount = resInfo.retObj["holding"][0]["short_avail_qty"].asString();
			positionInfo.bearPriceAvg = resInfo.retObj["holding"][0]["short_avg_cost"].asString();
			positionInfo.broken = resInfo.retObj["holding"][0]["liquidation_price"].asString();
		}
		UpdatePositionInfo(positionInfo);
		OKEX_WEB_SOCKET->API_FuturesPositionInfo(true, m_bSwapFutures, m_strCoinType, m_strMoneyType, m_strFuturesCycle);

	}
}

void CManualOKExFuturesDlg::UpdateTradeInfo(SFuturesTradeInfo& info)
{
	std::vector<SFuturesTradePairInfo>::iterator itB = m_vecTradePairInfo.begin();
	std::vector<SFuturesTradePairInfo>::iterator itE = m_vecTradePairInfo.end();
	while(itB != itE)
	{
		if(info.tradeType == itB->open.tradeType && itB->open.orderID == info.orderID)
		{
			if(itB->open.status != "2" && info.status == "2")
				itB->open.tLastALLFillTime = time(NULL);
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

void CManualOKExFuturesDlg::UpdatePositionInfo(SFuturesPositionInfo& info)
{
	m_positionInfo = info;
	m_positionInfo.bValid = true;
	_UpdatePositionShow();
}

void CManualOKExFuturesDlg::_UpdatePositionShow()
{
	if(m_positionInfo.bValid)
	{
		if(m_listCtrlPostionInfo.GetItemCount() == 0)
		{
			m_listCtrlPostionInfo.InsertItem(0, "");
		}
		m_listCtrlPostionInfo.SetItemText(0, 0, m_positionInfo.bullCount.c_str());
		m_listCtrlPostionInfo.SetItemText(0, 1, m_positionInfo.bullFreeCount.c_str());
		m_listCtrlPostionInfo.SetItemText(0, 2, m_positionInfo.bullPriceAvg.c_str());
		m_listCtrlPostionInfo.SetItemText(0, 3, m_positionInfo.bearCount.c_str());
		m_listCtrlPostionInfo.SetItemText(0, 4, m_positionInfo.bearFreeCount.c_str());
		m_listCtrlPostionInfo.SetItemText(0, 5, m_positionInfo.bearPriceAvg.c_str());
		m_listCtrlPostionInfo.SetItemText(0, 6, m_positionInfo.broken.c_str());
	}
}
void CManualOKExFuturesDlg::ClearDepth()
{
	m_mapDepthSell.clear();
	m_mapDepthBuy.clear();
}
bool _DepthSortB2S (const SFuturesDepth& a, const SFuturesDepth& b)
{ 
	double _a = atof(a.price.c_str());
	double _b = atof(b.price.c_str());
	return (_a>_b); 
}
bool _DepthSortS2B(const SFuturesDepth& a, const SFuturesDepth& b)
{
	double _a = atof(a.price.c_str());
	double _b = atof(b.price.c_str());
	return (_a < _b);
}

void CManualOKExFuturesDlg::UpdateDepthInfo(bool bBuy, SFuturesDepth& info)
{
	if(bBuy)
	{
		if(info.size == "0")
			m_mapDepthBuy.erase(info.price);
		else
			m_mapDepthBuy[info.price] = info;
		m_vecDepthBuy.clear();
		std::map<std::string, SFuturesDepth>::iterator itB = m_mapDepthBuy.begin();
		std::map<std::string, SFuturesDepth>::iterator itE = m_mapDepthBuy.end();
		while(itB != itE)
		{
			m_vecDepthBuy.push_back(itB->second);
			++itB;
		}
		std::sort(m_vecDepthBuy.begin(), m_vecDepthBuy.end(), _DepthSortS2B);
	}
	else
	{
		if(info.size == "0")
			m_mapDepthSell.erase(info.price);
		else
			m_mapDepthSell[info.price] = info;
		m_vecDepthSell.clear();
		std::map<std::string, SFuturesDepth>::iterator itB = m_mapDepthSell.begin();
		std::map<std::string, SFuturesDepth>::iterator itE = m_mapDepthSell.end();
		while(itB != itE)
		{
			m_vecDepthSell.push_back(itB->second);
			++itB;
		}
		std::sort(m_vecDepthSell.begin(), m_vecDepthSell.end(), _DepthSortS2B);
	}
	_UpdateDepthShow();
}

void CManualOKExFuturesDlg::OnTradeSuccess(std::string& clientOrderID, std::string& serverOrderID)
{
	std::vector<SFuturesTradePairInfo>::iterator itB = m_vecTradePairInfo.begin();
	std::vector<SFuturesTradePairInfo>::iterator itE = m_vecTradePairInfo.end();
	bool bUpdate = false;
	while(itB != itE)
	{
		if(itB->open.strClientOrderID == clientOrderID)
		{
			itB->open.orderID = serverOrderID;
			itB->open.waitClientOrderIDTime = 0;
			CActionLog("trade", "http下单成功 client_order=%s, order=%s", itB->open.strClientOrderID.c_str(), itB->open.orderID.c_str());
			OKEX_HTTP->API_FuturesOrderInfo(true, m_bSwapFutures, m_strCoinType, m_strMoneyType, m_strFuturesCycle, serverOrderID);
			bUpdate = true;
			break;
		}
		if(itB->close.strClientOrderID == clientOrderID)
		{
			itB->close.orderID = serverOrderID;
			itB->close.waitClientOrderIDTime = 0;
			CActionLog("trade", "http下单成功 client_order=%s, order=%s", itB->close.strClientOrderID.c_str(), itB->close.orderID.c_str());
			OKEX_HTTP->API_FuturesOrderInfo(true, m_bSwapFutures, m_strCoinType, m_strMoneyType, m_strFuturesCycle, serverOrderID);
			bUpdate = true;
			break;
		}
		++itB;
	}
	if(bUpdate)
		_SaveData();
}

void CManualOKExFuturesDlg::OnTradeFail(std::string& clientOrderID)
{
	std::vector<SFuturesTradePairInfo>::iterator itB = m_vecTradePairInfo.begin();
	std::vector<SFuturesTradePairInfo>::iterator itE = m_vecTradePairInfo.end();
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

void CManualOKExFuturesDlg::_UpdateAccountShow()
{
	if(m_accountInfo.bValid)
	{
		m_staticAccountInfo.SetWindowText(m_accountInfo.equity.c_str());
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
		m_staticAccountAvailInfo.SetWindowText(m_accountInfo.availBalance.c_str());
		if(m_curTickData.bValid)
		{
			if(m_strMoneyType == "USD")
			{
				int sizePrice = 10;
				if(m_strCoinType == "BTC")
					sizePrice = 100;
				double availBalance = stod(m_accountInfo.availBalance);
				availBalance *= m_dLeverage;
				availBalance *= m_curTickData.last;
				int canOpenSize = int(availBalance / sizePrice);
				CString szTemp;
				szTemp.Format("%d张", canOpenSize);
				m_staticCanOpenSize.SetWindowText(szTemp.GetBuffer());
			}
			else if(m_strMoneyType == "USDT")
			{
				double dRatio = 1;
				if(m_strCoinType == "TRX")
					dRatio = 1000;
				else if(m_strCoinType == "BTC")
					dRatio = 0.01;
				else if(m_strCoinType == "LTC")
					dRatio = 1;
				else if(m_strCoinType == "ETH")
					dRatio = 0.1;
				else if(m_strCoinType == "ETC")
					dRatio = 10;
				else if(m_strCoinType == "XRP")
					dRatio = 100;
				else if(m_strCoinType == "EOS")
					dRatio = 10;
				else if(m_strCoinType == "BCH")
					dRatio = 0.1;
				else if(m_strCoinType == "BSV")
					dRatio = 1;
				else if(m_strCoinType == "DOT")
					dRatio = 1;
				double availBalance = stod(m_accountInfo.availBalance);
				availBalance /= m_curTickData.last;
				availBalance /= dRatio;
				availBalance *= m_dLeverage;
				CString szTemp;
				szTemp.Format("%d张", (int)(availBalance));
				m_staticCanOpenSize.SetWindowText(szTemp.GetBuffer());
			}
		}
	}
}

void CManualOKExFuturesDlg::_UpdateTradeShow()
{
	CString szFormat;
	for(int i = 0; i<(int)m_vecTradePairInfo.size(); ++i)
	{
		SFuturesTradePairInfo& info = m_vecTradePairInfo[i];
		if(m_listCtrlOrderOpen.GetItemCount() <= i)
		{
			m_listCtrlOrderOpen.InsertItem(i, "");
			m_listCtrlOrderClose.InsertItem(i, "");
		}
		if(info.open.orderID != "" || info.strTriggerPrice != "")
		{
			szFormat.Format("%d", i+1);
			m_listCtrlOrderOpen.SetItemText(i, 0, szFormat);
			if(info.strTriggerPrice != "")
				szFormat.Format("%s|%s", info.strTriggerPrice.c_str(), info.openPrice.c_str());
			else
			{
				if(CFuncCommon::CheckEqual(info.open.priceAvg, 0.0))
					szFormat = CFuncCommon::Double2String(info.open.price+DOUBLE_PRECISION, m_nPriceDecimal).c_str();
				else
					szFormat = CFuncCommon::Double2String(info.open.priceAvg+DOUBLE_PRECISION, m_nPriceDecimal).c_str();
			}
			m_listCtrlOrderOpen.SetItemText(i, 1, szFormat);
			switch(info.open.tradeType)
			{
			case eFuturesTradeType_OpenBull:
				m_listCtrlOrderOpen.SetItemText(i, 2, "开多");
				break;
			case eFuturesTradeType_OpenBear:
				m_listCtrlOrderOpen.SetItemText(i, 2, "开空");
				break;
			default:
				break;
			}
			if(info.strTriggerPrice != "")
				szFormat.Format("%s", info.openSize.c_str());
			else
				szFormat.Format("%s/%s", info.open.filledQTY.c_str(), info.open.size.c_str());
			m_listCtrlOrderOpen.SetItemText(i, 3, szFormat);
			if(info.strTriggerPrice != "")
				m_listCtrlOrderOpen.SetItemText(i, 4, "plan");
			else if(info.open.status == "-1")
				m_listCtrlOrderOpen.SetItemText(i, 4, "cancelled");
			else if(info.open.status == "0")
				m_listCtrlOrderOpen.SetItemText(i, 4, "open");
			else if(info.open.status == "1")
				m_listCtrlOrderOpen.SetItemText(i, 4, "part_filled");
			else if(info.open.status == "2")
				m_listCtrlOrderOpen.SetItemText(i, 4, "filled");
			if(info.open.filledQTY != "0")
			{
				if(m_strMoneyType == "USD")
				{
					int count = stoi(info.open.filledQTY);
					int sizePrice = 10;
					if(m_strCoinType == "BTC")
						sizePrice = 100;
					double baozhengjin = (sizePrice/info.open.priceAvg)/m_dLeverage*count;
					if(info.open.tradeType == eFuturesTradeType_OpenBull)
					{
						double calcuPrice = stod(m_vecDepthBuy.rbegin()->price);
						double profitPersent = ((sizePrice/info.open.priceAvg - sizePrice/calcuPrice)*count)/baozhengjin;
						double profit = profitPersent*baozhengjin;
						if(calcuPrice >= info.open.priceAvg)
						{
							szFormat.Format("%s(%s%%)", CFuncCommon::Double2String(profit+DOUBLE_PRECISION, 5).c_str(), CFuncCommon::Double2String(profitPersent*100+DOUBLE_PRECISION, 2).c_str());
							m_listCtrlOrderOpen.SetItemText(i, 5, szFormat.GetBuffer());
						}
						else
						{
							szFormat.Format("-%s(-%s%%)", CFuncCommon::Double2String(-profit+DOUBLE_PRECISION, 5).c_str(), CFuncCommon::Double2String(-profitPersent*100+DOUBLE_PRECISION, 2).c_str());
							m_listCtrlOrderOpen.SetItemText(i, 5, szFormat.GetBuffer());
						}

					}
					else if(info.open.tradeType == eFuturesTradeType_OpenBear)
					{
						double calcuPrice = stod(m_vecDepthSell[0].price);
						double profitPersent = ((sizePrice/calcuPrice - sizePrice/info.open.priceAvg)*count)/baozhengjin;
						double profit = profitPersent*baozhengjin;
						if(calcuPrice <= info.open.priceAvg)
						{
							szFormat.Format("%s(%s%%)", CFuncCommon::Double2String(profit+DOUBLE_PRECISION, 4).c_str(), CFuncCommon::Double2String(profitPersent*100+DOUBLE_PRECISION, 2).c_str());
							m_listCtrlOrderOpen.SetItemText(i, 5, szFormat.GetBuffer());
						}
						else
						{
							szFormat.Format("-%s(-%s%%)", CFuncCommon::Double2String(-profit+DOUBLE_PRECISION, 4).c_str(), CFuncCommon::Double2String(-profitPersent*100+DOUBLE_PRECISION, 2).c_str());
							m_listCtrlOrderOpen.SetItemText(i, 5, szFormat.GetBuffer());
						}
					}
				}
				else if(m_strMoneyType == "USDT")
				{
					double dRatio = 1;
					if(m_strCoinType == "TRX")
						dRatio = 1000;
					else if(m_strCoinType == "BTC")
						dRatio = 0.01;
					else if(m_strCoinType == "LTC")
						dRatio = 1;
					else if(m_strCoinType == "ETH")
						dRatio = 0.1;
					else if(m_strCoinType == "ETC")
						dRatio = 10;
					else if(m_strCoinType == "XRP")
						dRatio = 100;
					else if(m_strCoinType == "EOS")
						dRatio = 10;
					else if(m_strCoinType == "BCH")
						dRatio = 0.1;
					else if(m_strCoinType == "BSV")
						dRatio = 1;
					else if(m_strCoinType == "DOT")
						dRatio = 1;
					int count = stoi(info.open.filledQTY);
					double baozhengjin = info.open.priceAvg*count/m_dLeverage;
					if(info.open.tradeType == eFuturesTradeType_OpenBull)
					{
						double calcuPrice = stod(m_vecDepthBuy.rbegin()->price);
						double profitPersent = (calcuPrice-info.open.priceAvg)/info.open.priceAvg*m_dLeverage;
						double profit = profitPersent*baozhengjin;
						if(calcuPrice >= info.open.priceAvg)
						{
							szFormat.Format("%s(%s%%)", CFuncCommon::Double2String(profit+DOUBLE_PRECISION, 4).c_str(), CFuncCommon::Double2String(profitPersent*100+DOUBLE_PRECISION, 2).c_str());
							m_listCtrlOrderOpen.SetItemText(i, 5, szFormat.GetBuffer());
						}
						else
						{
							szFormat.Format("-%s(-%s%%)", CFuncCommon::Double2String(-profit+DOUBLE_PRECISION, 4).c_str(), CFuncCommon::Double2String(-profitPersent*100+DOUBLE_PRECISION, 2).c_str());
							m_listCtrlOrderOpen.SetItemText(i, 5, szFormat.GetBuffer());
						}

					}
					else if(info.open.tradeType == eFuturesTradeType_OpenBear)
					{
						double calcuPrice = stod(m_vecDepthSell[0].price);
						double profitPersent = (calcuPrice-info.open.priceAvg)/info.open.priceAvg*m_dLeverage;
						double profit = profitPersent*baozhengjin;
						if(calcuPrice <= info.open.priceAvg)
						{
							szFormat.Format("%s(%s%%)", CFuncCommon::Double2String(profit+DOUBLE_PRECISION, 4).c_str(), CFuncCommon::Double2String(profitPersent*100+DOUBLE_PRECISION, 2).c_str());
							m_listCtrlOrderOpen.SetItemText(i, 5, szFormat.GetBuffer());
						}
						else
						{
							szFormat.Format("-%s(-%s%%)", CFuncCommon::Double2String(-profit+DOUBLE_PRECISION, 4).c_str(), CFuncCommon::Double2String(-profitPersent*100+DOUBLE_PRECISION, 2).c_str());
							m_listCtrlOrderOpen.SetItemText(i, 5, szFormat.GetBuffer());
						}
					}
				}
				
			}
			else
				m_listCtrlOrderOpen.SetItemText(i, 5, "");
			if(info.open.timeStamp != 0)
			{
				tm _tm;
				localtime_s(&_tm, ((const time_t*)&(info.open.timeStamp)));
				szFormat.Format("%02d-%02d %02d:%02d:%02d", _tm.tm_mon+1, _tm.tm_mday, _tm.tm_hour, _tm.tm_min, _tm.tm_sec);
				m_listCtrlOrderOpen.SetItemText(i, 6, szFormat.GetBuffer());
			}
			

			if(info.close.orderID != "")
			{
				szFormat.Format("%d", i+1);
				m_listCtrlOrderClose.SetItemText(i, 0, szFormat);
				if(CFuncCommon::CheckEqual(info.close.priceAvg, 0.0))
					szFormat = CFuncCommon::Double2String(info.close.price + DOUBLE_PRECISION, m_nPriceDecimal).c_str();
				else
					szFormat = CFuncCommon::Double2String(info.close.priceAvg + DOUBLE_PRECISION, m_nPriceDecimal).c_str();
				m_listCtrlOrderClose.SetItemText(i, 1, szFormat);
				szFormat.Format("%s/%s", info.close.filledQTY.c_str(), info.close.size.c_str());
				m_listCtrlOrderClose.SetItemText(i, 2, szFormat);
				if(info.close.status == "-1")
					m_listCtrlOrderClose.SetItemText(i, 3, "cancelled");
				else if(info.close.status == "0")
					m_listCtrlOrderClose.SetItemText(i, 3, "open");
				else if(info.close.status == "1")
					m_listCtrlOrderClose.SetItemText(i, 3, "part_filled");
				else if(info.close.status == "2")
					m_listCtrlOrderClose.SetItemText(i, 3, "filled");
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

void CManualOKExFuturesDlg::_UpdateDepthShow()
{
	const int showLines = 8;
	m_listCtrlDepth.DeleteAllItems();
	int sellLine = min(m_vecDepthSell.size(), showLines);
	for(int i = 0; i < sellLine; ++i)
	{
		m_listCtrlDepth.InsertItem(i, "");
	}
	std::vector<SFuturesDepth>::iterator itB = m_vecDepthSell.begin();
	std::vector<SFuturesDepth>::iterator itE = m_vecDepthSell.end();
	CString szFormat;
	int count = 0;
	while(itB != itE)
	{
		szFormat.Format("%d", count + 1);
		m_listCtrlDepth.SetItemText(sellLine - 1 - count, 0, szFormat);
		szFormat.Format("%s", itB->price.c_str());
		m_listCtrlDepth.SetItemText(sellLine - 1 - count, 1, szFormat);
		szFormat.Format("%s", itB->size.c_str());
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
	
	int buyLine = min(m_vecDepthBuy.size(), showLines);
	for(int i = 0; i < sellLine; ++i)
	{
		m_listCtrlDepth.InsertItem(sellLine + 1 + i, "");
	}
	if(buyLine > 0)
	{
		std::vector<SFuturesDepth>::iterator itB = m_vecDepthBuy.begin();
		std::vector<SFuturesDepth>::iterator itE = m_vecDepthBuy.end();
		CString _szFormat;
		int count = 0;
		itE--;
		while(itB != itE)
		{
			_szFormat.Format("%d", count + 1);
			m_listCtrlDepth.SetItemText(sellLine + 1 + count, 0, _szFormat);
			_szFormat.Format("%s", itE->price.c_str());
			m_listCtrlDepth.SetItemText(sellLine + 1 + count, 1, _szFormat);
			_szFormat.Format("%s", itE->size.c_str());
			m_listCtrlDepth.SetItemText(sellLine + 1 + count, 2, _szFormat);
			if(++count >= buyLine)
				break;
			itE--;
		}
		if(count < buyLine)
		{
			_szFormat.Format("%d", count + 1);
			m_listCtrlDepth.SetItemText(sellLine + 1 + count, 0, _szFormat);
			_szFormat.Format("%s", itB->price.c_str());
			m_listCtrlDepth.SetItemText(sellLine + 1 + count, 1, _szFormat);
			_szFormat.Format("%s", itB->size.c_str());
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
	_OpenOrder(eFuturesTradeType_OpenBull);
}


void CManualOKExFuturesDlg::OnBnClickedButtonOpenBear()
{
	_OpenOrder(eFuturesTradeType_OpenBear);
}

int CManualOKExFuturesDlg::_GetFreeOrderIndex()
{
	for(int i = 0; i<(int)m_vecTradePairInfo.size(); ++i)
	{
		if(m_vecTradePairInfo[i].open.orderID == "" && m_vecTradePairInfo[i].open.strClientOrderID == "")
			return i;
	}
	return -1;
}

void CManualOKExFuturesDlg::_OpenOrder(eFuturesTradeType type)
{
	if(!m_bRun)
	{
		MessageBox("未订阅行情");
		return;
	}
	if(!__SaveTradeConfigCtrl())
		return;
	std::string price;
	CString openPrice;
	m_editOpenPrice.GetWindowText(openPrice);
	if(openPrice == "")
		price = "-1";
	else
		price = openPrice.GetBuffer();
	CString triggerPrice;
	m_editTriggerPrice.GetWindowText(triggerPrice);
	double dTriggerPrice = 0.0;
	if(triggerPrice != "")
		dTriggerPrice = stod(triggerPrice.GetBuffer());
	std::string strClientOrderID = CFuncCommon::GenUUID();
	std::string strFuturesTradeSize = m_strFuturesTradeSize;
	if(m_strFuturesTradeSize == "" && m_accountInfo.bValid)
	{
		if(m_strMoneyType == "USDT")
		{
			double dRatio = 1;
			int delSize = 0;
			if(m_strCoinType == "TRX")
			{
				dRatio = 1000;
				delSize = 30;
			}
			else if(m_strCoinType == "BTC")
			{
				dRatio = 0.01;
				delSize = 1;
			}
			else if(m_strCoinType == "LTC")
			{
				dRatio = 1;
				delSize = 1;
			}
			else if(m_strCoinType == "ETH")
			{
				dRatio = 0.1;
				delSize = 1;
			}
			else if(m_strCoinType == "ETC")
			{
				dRatio = 10;
				delSize = 1;
			}
			else if(m_strCoinType == "XRP")
			{
				dRatio = 100;
				delSize = 3;
			}
			else if(m_strCoinType == "EOS")
			{
				dRatio = 10;
				delSize = 1;
			}
			else if(m_strCoinType == "BCH")
			{

				dRatio = 0.1;
				delSize = 1;
			}
			else if(m_strCoinType == "BSV")
			{

				dRatio = 1;
				delSize = 1;
			}
			else if(m_strCoinType == "DOT")
			{

				dRatio = 1;
				delSize = 1;
			}
			double availBalance = stod(m_accountInfo.availBalance);
			double dOpenPrice = stod(price);
			availBalance /= dOpenPrice;
			availBalance /= dRatio;
			availBalance *= m_dLeverage;
			int openSize = (int)availBalance;
			openSize -= delSize;
			strFuturesTradeSize = CFuncCommon::ToString(openSize);
		}
	}
	if(strFuturesTradeSize == "")
	{
		MessageBox("未填写下单张数");
		return;
	}
	if(triggerPrice != "")
	{
		CActionLog("trade", "设置条件开单 开%s单%s张, price=%s, client_oid=%s, tiggerPrice=%s", ((type == eFuturesTradeType_OpenBull) ? "多" : "空"), strFuturesTradeSize.c_str(), price.c_str(), strClientOrderID.c_str(), triggerPrice.GetBuffer());
	}
	else
	{
		OKEX_HTTP->API_FuturesTrade(true, m_bSwapFutures, type, m_strCoinType, m_strMoneyType, m_strFuturesCycle, price, strFuturesTradeSize, m_strLeverage, strClientOrderID);
		CActionLog("trade", "开%s单%s张, price=%s, client_oid=%s", ((type == eFuturesTradeType_OpenBull) ? "多" : "空"), strFuturesTradeSize.c_str(), price.c_str(), strClientOrderID.c_str());
	}
	SFuturesTradePairInfo* pInfo;
	int nIndex = _GetFreeOrderIndex();
	if(nIndex == -1)
	{
		SFuturesTradePairInfo temp;
		m_vecTradePairInfo.push_back(temp);
		pInfo = &m_vecTradePairInfo[m_vecTradePairInfo.size()-1];
	}
	else
		pInfo = &m_vecTradePairInfo[nIndex];
	if(triggerPrice != "")
	{
		pInfo->openPrice = price;
		pInfo->openSize = strFuturesTradeSize;
	}
	pInfo->open.strClientOrderID = strClientOrderID;
	pInfo->open.waitClientOrderIDTime = time(NULL);
	pInfo->open.tradeType = type;
	pInfo->strTriggerPrice = triggerPrice.GetBuffer();
	pInfo->triggerPrice = dTriggerPrice;
}

void CManualOKExFuturesDlg::_SaveData()
{
	std::string strFilePath = "./save.txt";
	std::ofstream stream(strFilePath);
	if(!stream.is_open())
		return;
	std::vector<SFuturesTradePairInfo>::iterator itB = m_vecTradePairInfo.begin();
	std::vector<SFuturesTradePairInfo>::iterator itE = m_vecTradePairInfo.end();
	while(itB != itE)
	{
		if(itB->open.orderID != "")
		{
			stream << itB->open.orderID << "	" << itB->open.strClientOrderID << "	";
			if(itB->open.bModifyQTY)
				stream << itB->open.filledQTY;
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

void CManualOKExFuturesDlg::OnBnClickedButtonClose()
{
	// TODO:  在此添加控件通知处理程序代码
	bool bUpdate = false;
	for(int i = 0; i<(int)m_vecTradePairInfo.size(); ++i)
	{
		SFuturesTradePairInfo& info = m_vecTradePairInfo[i];
		//if(info.open.orderID != "")
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
						size = info.open.filledQTY;
					else
						size = closeSize.GetBuffer();
					if(info.open.status == "0" || info.strTriggerPrice != "")
					{
						if(price == "-1")
							return;
						info.closePlanPrice = price;
						if(closeSize == "")
						{
							if(info.strTriggerPrice != "")
								info.closePlanSize = info.openSize;
							else
								info.closePlanSize = info.open.size;
						}
						else
							info.closePlanSize = size;
						bUpdate = true;
					}
					else if(info.open.status == "1")
					{
						OKEX_HTTP->API_FuturesCancelOrder(true, m_bSwapFutures, m_strCoinType, m_strMoneyType, m_strFuturesCycle, info.open.orderID);
						
						std::string strClientOrderID = CFuncCommon::GenUUID();
						eFuturesTradeType tradeType;
						if(info.open.tradeType == eFuturesTradeType_OpenBull)
							tradeType = eFuturesTradeType_CloseBull;
						else
							tradeType = eFuturesTradeType_CloseBear;
						OKEX_HTTP->API_FuturesTrade(true, m_bSwapFutures, tradeType, m_strCoinType, m_strMoneyType, m_strFuturesCycle, price, size, m_strLeverage, strClientOrderID);
						info.close.strClientOrderID = strClientOrderID;
						info.close.waitClientOrderIDTime = time(NULL);
						info.close.tradeType = tradeType;
						CActionLog("trade", "开平仓单");
					}
					else if(info.open.status == "2" || info.open.status == "-1")
					{
						std::string strClientOrderID = CFuncCommon::GenUUID();
						eFuturesTradeType tradeType;
						if(info.open.tradeType == eFuturesTradeType_OpenBull)
							tradeType = eFuturesTradeType_CloseBull;
						else
							tradeType = eFuturesTradeType_CloseBear;
						OKEX_HTTP->API_FuturesTrade(true, m_bSwapFutures, tradeType, m_strCoinType, m_strMoneyType, m_strFuturesCycle, price, size, m_strLeverage, strClientOrderID);
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


void CManualOKExFuturesDlg::OnBnClickedButtonCancel()
{
	bool bUpdate = false;
	for(int i = 0; i<(int)m_vecTradePairInfo.size(); ++i)
	{
		SFuturesTradePairInfo& info = m_vecTradePairInfo[i];
		if(info.open.orderID != "")
		{
			if(m_listCtrlOrderOpen.GetCheck(i))
			{
				if(info.open.status == "0" || info.open.status == "1")
					OKEX_HTTP->API_FuturesCancelOrder(true, m_bSwapFutures, m_strCoinType, m_strMoneyType, m_strFuturesCycle, info.open.orderID);
			}
			if(m_listCtrlOrderClose.GetCheck(i))
			{
				if(info.close.orderID != "")
				{
					if(info.close.status == "0" || info.close.status == "1")
						OKEX_HTTP->API_FuturesCancelOrder(true, m_bSwapFutures, m_strCoinType, m_strMoneyType, m_strFuturesCycle, info.close.orderID);
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


void CManualOKExFuturesDlg::_CheckAllOrder()
{
	time_t tNow = time(NULL);
	bool bUpdate = false;
	std::vector<SFuturesTradePairInfo>::iterator itB = m_vecTradePairInfo.begin();
	std::vector<SFuturesTradePairInfo>::iterator itE = m_vecTradePairInfo.end();
	while(itB != itE)
	{
		if(itB->open.orderID != "")
		{
			if(itB->open.status == "-1")
			{
				if(itB->close.orderID == "")
				{
					if(itB->open.filledQTY == "0")
					{
						itB->Reset();
						bUpdate = true;
					}
				}
				else
				{
					if(itB->close.status == "-1")
					{
						if(itB->close.filledQTY == "0")
						{
							itB->close.Reset();
							bUpdate = true;
						}
						else
						{
							int openQTY = stoi(itB->open.filledQTY);
							int closeQTY = stoi(itB->close.filledQTY);
							int last = openQTY - closeQTY;
							if(last)
							{
								itB->open.filledQTY = CFuncCommon::ToString(last);
								itB->open.bModifyQTY = true;
								bUpdate = true;
								itB->close.Reset();
							}
						}
					}
					else if(itB->close.status == "2")
					{
						int openQTY = stoi(itB->open.filledQTY);
						int closeQTY = stoi(itB->close.filledQTY);
						int last = openQTY - closeQTY;
						if(last)
						{
							itB->open.filledQTY = CFuncCommon::ToString(last);
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
			else if(itB->open.status == "2")
			{
				if(itB->close.orderID != "")
				{
					if(itB->close.status == "-1")
					{
						if(itB->close.filledQTY == "0")
						{
							itB->close.Reset();
							bUpdate = true;
						}
						else
						{
							int openQTY = stoi(itB->open.filledQTY);
							int closeQTY = stoi(itB->close.filledQTY);
							int last = openQTY - closeQTY;
							if(last == 0)
							{
								itB->Reset();
								bUpdate = true;
							}
							else
							{
								itB->open.filledQTY = CFuncCommon::ToString(last);
								itB->open.bModifyQTY = true;
								bUpdate = true;
								itB->close.Reset();
							}
						}
					}
					else if(itB->close.status == "2")
					{
						int openQTY = stoi(itB->open.filledQTY);
						int closeQTY = stoi(itB->close.filledQTY);
						int last = openQTY - closeQTY;
						if(last == 0)
						{
							itB->Reset();
							bUpdate = true;
						}
						else
						{
							itB->open.filledQTY = CFuncCommon::ToString(last);
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
						eFuturesTradeType tradeType;
						if(itB->open.tradeType == eFuturesTradeType_OpenBull)
							tradeType = eFuturesTradeType_CloseBull;
						else
							tradeType = eFuturesTradeType_CloseBear;
						OKEX_HTTP->API_FuturesTrade(true, m_bSwapFutures, tradeType, m_strCoinType, m_strMoneyType, m_strFuturesCycle, itB->closePlanPrice, itB->closePlanSize, m_strLeverage, strClientOrderID);
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

void CManualOKExFuturesDlg::OnBnClickedButtonUpdateBeginMoney()
{
	m_beginMoney = 0.0;
	CString szCost;
	m_editCapital.GetWindowText(szCost);
	m_beginMoney = stod(szCost.GetBuffer());
	m_config.set_value("futures", "beginMoney", szCost.GetBuffer());
	m_config.save("./config.ini");
}


bool CManualOKExFuturesDlg::CheckDepthInfo(int checkNum, std::string& checkSrc)
{
	std::vector<SFuturesDepth>::reverse_iterator itBB = m_vecDepthBuy.rbegin();
	std::vector<SFuturesDepth>::reverse_iterator itBE = m_vecDepthBuy.rend();
	std::vector<SFuturesDepth>::iterator itSB = m_vecDepthSell.begin();
	std::vector<SFuturesDepth>::iterator itSE = m_vecDepthSell.end();
	int nIndex = 0;
	checkSrc = "";
	while(nIndex < 25)
	{
		bool have = false;
		if(itBB != itBE)
		{
			if(nIndex != 0)
				checkSrc.append(":");
			checkSrc.append(itBB->price).append(":").append(itBB->size);
			have = true;
			++itBB;
		}
		if(itSB != itSE)
		{
			if(nIndex != 0 || have)
				checkSrc.append(":");
			checkSrc.append(itSB->price).append(":").append(itSB->size);
			++itSB;
		}
		++nIndex;
	}
	int crc = CFuncCommon::crc32((const unsigned char*)checkSrc.c_str(), checkSrc.size());
	if(checkNum != crc)
	{
		LOCAL_ERROR("crc校验失败 checknum=%d local=%d", checkNum, crc);
		OKEX_WEB_SOCKET->API_FuturesEntrustDepth(false, m_bSwapFutures, m_strCoinType, m_strMoneyType, m_strFuturesCycle);
		m_tWaitNewSubDepth = time(NULL);
		m_bWaitDepthBegin = true;
		return false;
	}
	return true;
}



void CManualOKExFuturesDlg::OnBnClickedButtonSaveProfit()
{
	if(!CFuncCommon::CheckEqual(m_beginMoney, 0.0))
	{
		double equity = stod(m_accountInfo.equity);
		if(equity >= m_beginMoney)
		{
			CFixActionLog("profit", "收益%.4f个(%.2f%%)", equity - m_beginMoney, (equity - m_beginMoney) / m_beginMoney * 100);
			MessageBox("收益保存成功");
		}
	}
	// TODO:  在此添加控件通知处理程序代码
}


void CManualOKExFuturesDlg::OnBnClickedButtonClearFreeLine()
{
	std::vector<SFuturesTradePairInfo>::iterator itB = m_vecTradePairInfo.begin();
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

bool CompareLowPrice(const SFuturesTradePairInfo &value1, const SFuturesTradePairInfo &value2)
{
	if(CFuncCommon::CheckEqual(value1.open.priceAvg, 0.0))
		return value1.open.price < value2.open.price;
	else
		return value1.open.priceAvg < value2.open.priceAvg;
}

bool CompareHightPrice(const SFuturesTradePairInfo &value1, const SFuturesTradePairInfo &value2)
{
	if(CFuncCommon::CheckEqual(value1.open.priceAvg, 0.0))
		return value1.open.price > value2.open.price;
	else
		return value1.open.priceAvg > value2.open.priceAvg;
}

void CManualOKExFuturesDlg::OnBnClickedButtonBearFirst()
{
	std::vector<SFuturesTradePairInfo> bearFinish;
	std::vector<SFuturesTradePairInfo> bearNotFinish;
	std::vector<SFuturesTradePairInfo> bullFinish;
	std::vector<SFuturesTradePairInfo> bullNotFinish;

	for(int i = 0; i<(int)m_vecTradePairInfo.size(); ++i)
	{
		if(m_vecTradePairInfo[i].open.tradeType == eFuturesTradeType_OpenBull)
		{
			if(m_vecTradePairInfo[i].open.status == "2" || m_vecTradePairInfo[i].open.status == "1")
				bullFinish.push_back(m_vecTradePairInfo[i]);
			else
				bullNotFinish.push_back(m_vecTradePairInfo[i]);
				
		}
		else if(m_vecTradePairInfo[i].open.tradeType == eFuturesTradeType_OpenBear)
		{
			if(m_vecTradePairInfo[i].open.status == "2" || m_vecTradePairInfo[i].open.status == "1")
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
	std::vector<SFuturesTradePairInfo>::iterator itB = bearFinish.begin();
	std::vector<SFuturesTradePairInfo>::iterator itE = bearFinish.end();
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


void CManualOKExFuturesDlg::OnBnClickedButtonBullFirst()
{
	std::list<SFuturesTradePairInfo> bearFinish;
	std::list<SFuturesTradePairInfo> bearNotFinish;
	std::list<SFuturesTradePairInfo> bullFinish;
	std::list<SFuturesTradePairInfo> bullNotFinish;
	for(int i = 0; i<(int)m_vecTradePairInfo.size(); ++i)
	{
		if(m_vecTradePairInfo[i].open.tradeType == eFuturesTradeType_OpenBull)
		{
			if(m_vecTradePairInfo[i].open.status == "2")
				bullFinish.push_back(m_vecTradePairInfo[i]);
			else
				bullNotFinish.push_back(m_vecTradePairInfo[i]);

		}
		else if(m_vecTradePairInfo[i].open.tradeType == eFuturesTradeType_OpenBear)
		{
			if(m_vecTradePairInfo[i].open.status == "2")
				bearFinish.push_back(m_vecTradePairInfo[i]);
			else
				bearNotFinish.push_back(m_vecTradePairInfo[i]);
		}
	}
	m_vecTradePairInfo.clear();
	std::list<SFuturesTradePairInfo>::iterator itB = bullFinish.begin();
	std::list<SFuturesTradePairInfo>::iterator itE = bullFinish.end();
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
