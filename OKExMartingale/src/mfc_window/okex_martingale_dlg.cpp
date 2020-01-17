
// OKExMartingaleDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "okex_martingale.h"
#include "okex_martingale_dlg.h"
#include "afxdialogex.h"
#include <clib/lib/file/file_util.h>
#include "log/local_log.h"
#include "log/local_action_log.h"
#include "exchange/okex/okex_exchange.h"
#include "exchange/okex/okex_websocket_api.h"
#include "exchange/okex/okex_http_api.h"
#include "common/func_common.h"
#include "api_callback.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define state_cancelled "-1"
#define state_open "0"
#define state_part_filled "1"
#define state_filled "2"
#define DEFAULT_TRADE_TIMER_OVER 10
#define KLINE_DATA m_vecKlineData
#define KLINE_DATA_SIZE ((int)m_vecKlineData.size())
#define OKEX_CHANGE ((COkexExchange*)pExchange)
#define OKEX_WEB_SOCKET ((COkexWebsocketAPI*)pExchange->GetWebSocket())
#define OKEX_HTTP ((COkexHttpAPI*)pExchange->GetHttp())
#define OKEX_TRADE_HTTP ((COkexHttpAPI*)pExchange->GetTradeHttp())

#define BEGIN_API_CHECK {\
							int _checkIndex = 0;\
							std::string _checkStr = "";\
							for(; _checkIndex<5; ++_checkIndex)\
							{
#define API_OK	break;

#define API_CHECK		}\
						if(_checkIndex == 5)\
							__asm int 3;
#define END_API_CHECK }
						

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框
CExchange* pExchange = NULL;
COKExMartingaleDlg* g_pDlg = NULL;
int LocalLogCallBackFunc(LOG_TYPE type, const char* szLog)
{
	if(g_pDlg->m_ctrlListLog.GetCount() > 1500)
		g_pDlg->m_ctrlListLog.ResetContent();
	g_pDlg->m_ctrlListLog.InsertString(0, szLog);
	g_pDlg->SetHScroll();
	return 0;
}

class CAboutDlg : public CDialog
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

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// COKExMartingaleDlg 对话框






COKExMartingaleDlg::COKExMartingaleDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COKExMartingaleDlg::IDD, pParent)
{
	g_pDlg = this;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_nPriceDecimal = 4;
	m_bRun = false;
	m_tListenPong = 0;
	m_strCoinType = "BTC";
	m_strStandardCurrency = "USD";
	m_eTradeState = eTradeState_WaitOpen;
	m_martingaleStepCnt = 5;
	m_martingaleMovePersent = 0.02;
	m_curOpenFinishIndex = -1;
	m_bStopWhenFinish = false;
	m_beginMoney = 0.0;
	m_stopProfitFactor = 0.005;
	m_bStopProfitMove = true;
	m_tOpenTime = 0;
	m_bExit = false;
	m_tLastUpdate15Sec = 0;
	m_tLastUpdate3Sec = 0;
	m_nStopProfitTimes = 0;
	m_nFinishTimes = 0;
	m_strKlineCycle = "candle180s";
	m_nKlineCycle = 180;
	m_tWaitNewSubDepth = 0;
	m_bWaitDepthBegin = true;
	m_bFirstKLine = true;
	m_bOpenBull = true;
	m_nFirstTradeSize = 1;
	m_strFuturesCycle = "190927";
	m_strLeverage = "20";
	m_nLeverage = 20;
	m_bSwapFutures = false;
	m_bSaveData = false;
	m_bLoginSuccess = false;
	m_nTrendType = 0;
	m_bNeedSubscribe = true;
	m_nLastCheckKline = -1;
	m_nTradeTimeOver = DEFAULT_TRADE_TIMER_OVER;
}

void COKExMartingaleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_ctrlListLog);
	DDX_Control(pDX, IDC_COMBO1, m_combCoinType);
	DDX_Control(pDX, IDC_EDIT1, m_editMartingaleStepCnt);
	DDX_Control(pDX, IDC_EDIT2, m_editMartingaleMovePersent);
	DDX_Control(pDX, IDC_EDIT5, m_editStopProfitFactor);
	DDX_Control(pDX, IDC_EDIT6, m_editCoin);
	DDX_Control(pDX, IDC_EDIT7, m_editProfit);
	DDX_Control(pDX, IDC_STATIC_COIN, m_staticCoin);
	DDX_Control(pDX, IDC_EDIT8, m_editCost);
	DDX_Control(pDX, IDC_LIST3, m_listCtrlOpen);
	DDX_Control(pDX, IDC_LIST2, m_listCtrlClose);
	DDX_Control(pDX, IDC_STATIC_PRICE, m_staticPrice);
	DDX_Control(pDX, IDC_STATIC_STOP_PROFIT_TIME, m_staticStopProfitTimes);
	DDX_Control(pDX, IDC_STATIC_FINISH_TIME, m_staticFinishTimes);
	DDX_Control(pDX, IDC_RADIO1, m_btnStopProfitMove);
	DDX_Control(pDX, IDC_RADIO2, m_btnStopProfitFix);
	DDX_Control(pDX, IDC_EDIT3, m_editFuturesCycle);
	DDX_Control(pDX, IDC_COMBO2, m_combLeverage);
	DDX_Control(pDX, IDC_COMBO3, m_combFuturesType);
	DDX_Control(pDX, IDC_EDIT10, m_editFirstTradeSize);
	DDX_Control(pDX, IDC_COMBO4, m_combKLineCycle);
	DDX_Control(pDX, IDC_RADIO3, m_btnTrendBull);
	DDX_Control(pDX, IDC_RADIO4, m_btnTrendBear);
	DDX_Control(pDX, IDC_RADIO5, m_btnTrendAuto);
	DDX_Control(pDX, IDC_STATIC_DINGDAN, m_staticDingDan);
	DDX_Control(pDX, IDC_LIST_OPEN2, m_listCtrlPostionInfo);
	DDX_Control(pDX, IDC_EDIT4, m_editTradeTimerOver);
	DDX_Control(pDX, IDC_COMBO5, m_combStandardCurrency);
}

BEGIN_MESSAGE_MAP(COKExMartingaleDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &COKExMartingaleDlg::OnBnClickedButtonStart)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON3, &COKExMartingaleDlg::OnBnClickedButtonStopWhenFinish)
	ON_BN_CLICKED(IDC_BUTTON4, &COKExMartingaleDlg::OnBnClickedButtonUpdateCost)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_RADIO1, &COKExMartingaleDlg::OnBnClickedStopProfitMove)
	ON_BN_CLICKED(IDC_RADIO2, &COKExMartingaleDlg::OnBnClickedRadioStopProfitFix)
	ON_BN_CLICKED(IDC_RADIO3, &COKExMartingaleDlg::OnBnClickedTrendBull)
	ON_BN_CLICKED(IDC_RADIO4, &COKExMartingaleDlg::OnBnClickedTrendBear)
	ON_BN_CLICKED(IDC_RADIO5, &COKExMartingaleDlg::OnBnClickedTrendAuto)
	ON_BN_CLICKED(IDC_BUTTON2, &COKExMartingaleDlg::OnBnClickedButtonUpdateTrend)
END_MESSAGE_MAP()


// COKExMartingaleDlg 消息处理程序

BOOL COKExMartingaleDlg::OnInitDialog()
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

	m_combStandardCurrency.InsertString(0, "USD");
	m_combStandardCurrency.InsertString(1, "USDT");

	m_combLeverage.InsertString(0, "10");
	m_combLeverage.InsertString(1, "20");

	m_combFuturesType.InsertString(0, "交割合约");
	m_combFuturesType.InsertString(1, "永续合约");

	m_listCtrlOpen.InsertColumn(0, "价格", LVCFMT_CENTER, 80);
	m_listCtrlOpen.InsertColumn(1, "成交量", LVCFMT_CENTER, 100);
	m_listCtrlOpen.InsertColumn(2, "售出量", LVCFMT_CENTER, 50);
	m_listCtrlOpen.InsertColumn(3, "状态", LVCFMT_CENTER, 75);
	m_listCtrlOpen.InsertColumn(4, "参考利润", LVCFMT_CENTER, 80);
	m_listCtrlOpen.InsertColumn(5, "最低价", LVCFMT_CENTER, 80);
	m_listCtrlOpen.InsertColumn(6, "最高价", LVCFMT_CENTER, 80);
	m_listCtrlOpen.InsertColumn(7, "止盈", LVCFMT_CENTER, 70);
	m_listCtrlOpen.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

	m_listCtrlClose.InsertColumn(0, "价格", LVCFMT_CENTER, 70);
	m_listCtrlClose.InsertColumn(1, "成交量", LVCFMT_CENTER, 100);
	m_listCtrlClose.InsertColumn(2, "状态", LVCFMT_CENTER, 70);
	m_listCtrlClose.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

	m_combKLineCycle.InsertString(0, "1分钟");
	m_combKLineCycle.InsertString(1, "3分钟");
	m_combKLineCycle.InsertString(2, "5分钟");
	m_combKLineCycle.InsertString(3, "15分钟");
	m_combKLineCycle.InsertString(4, "30分钟");
	m_combKLineCycle.InsertString(5, "1小时");
	m_combKLineCycle.InsertString(6, "2小时");
	m_combKLineCycle.InsertString(7, "4小时");
	m_combKLineCycle.InsertString(8, "6小时");
	m_combKLineCycle.InsertString(9, "12小时");
	m_combKLineCycle.InsertString(10, "1日");
	m_combKLineCycle.InsertString(11, "1周");

	m_listCtrlPostionInfo.InsertColumn(0, "多仓", LVCFMT_CENTER, 55);
	m_listCtrlPostionInfo.InsertColumn(1, "多可平", LVCFMT_CENTER, 54);
	m_listCtrlPostionInfo.InsertColumn(2, "多均价", LVCFMT_CENTER, 80);
	m_listCtrlPostionInfo.InsertColumn(3, "多保证金", LVCFMT_CENTER, 80);
	m_listCtrlPostionInfo.InsertColumn(4, "空仓", LVCFMT_CENTER, 55);
	m_listCtrlPostionInfo.InsertColumn(5, "空可平", LVCFMT_CENTER, 54);
	m_listCtrlPostionInfo.InsertColumn(6, "空均价", LVCFMT_CENTER, 80);
	m_listCtrlPostionInfo.InsertColumn(7, "空保证金", LVCFMT_CENTER, 80);
	m_listCtrlPostionInfo.InsertColumn(8, "爆仓价", LVCFMT_CENTER, 80);
	m_listCtrlPostionInfo.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

	if(!m_config.open("./config.ini"))
		return FALSE;
	m_apiKey = m_config.get("futures", "apiKey", "");
	m_secretKey = m_config.get("futures", "secretKey", "");
	m_passphrase = m_config.get("futures", "passphrase", "");
	__InitConfigCtrl();
	m_logicThread = boost::thread(boost::bind(&COKExMartingaleDlg::_LogicThread, this));
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void COKExMartingaleDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void COKExMartingaleDlg::OnPaint()
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
HCURSOR COKExMartingaleDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void COKExMartingaleDlg::OnBnClickedButtonStart()
{
	if(!__SaveConfigCtrl())
		return;
	if(m_bRun)
		return;
	bool bFound = false;
	std::string instrumentID;
	if (m_bSwapFutures)
		instrumentID = m_strCoinType + "-" + m_strStandardCurrency + "-SWAP";
	else
		instrumentID = m_strCoinType + "-" + m_strStandardCurrency + "-" + m_strFuturesCycle;
	for(int i=0; i<3; ++i)
	{
		SHttpResponse resInfo;
		OKEX_HTTP->API_FuturesInstruments(false, m_bSwapFutures, &resInfo);
		bFound = false;
		if(resInfo.retObj.isArray())
		{
			for(int j=0; j<(int)resInfo.retObj.size(); ++j)
			{
				if (resInfo.retObj[j]["instrument_id"].asString() == instrumentID)
				{
					std::string strTickSize = resInfo.retObj[j]["tick_size"].asString();
					int pos = strTickSize.find_first_of(".");
					if (pos != std::string::npos)
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
					if (bFound)
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
		OKEX_HTTP->API_FuturesSetLeverage(false, m_bSwapFutures, m_strCoinType, m_strStandardCurrency, strLeverage, &resInfo);
		if (m_bSwapFutures)
		{
			std::string strInstrumentID = m_strCoinType + "-" + m_strStandardCurrency + "-SWAP";
			if (!resInfo.retObj.isObject() || ((resInfo.retObj["instrument_id"].asString() != strInstrumentID) && (resInfo.retObj["code"].asInt() != 35017)))
			{
				MessageBox("设置杠杆失败");
				return;
			}
		}
		else
		{
			if (!resInfo.retObj.isObject() || (resInfo.retObj["result"].asString() != "true"))
			{
				MessageBox("设置杠杆失败");
				return;
			}
		}
	}
	m_accountInfo.bValid = false;
	m_positionInfo.bValid = false;
	{
		if(!_CheckMoney(m_strCoinType))
		{
			std::string msg = "未查询到币种信息[" + m_strCoinType + "]";
			MessageBox(msg.c_str());
			return;
		}
	}
	_LoadData();
	if(OKEX_WEB_SOCKET)
	{
		if(m_bNeedSubscribe)
		{
			OKEX_WEB_SOCKET->API_FuturesTickerData(true, m_bSwapFutures, m_strCoinType, m_strStandardCurrency, m_strFuturesCycle);
			OKEX_WEB_SOCKET->API_FuturesEntrustDepth(true, m_bSwapFutures, m_strCoinType, m_strStandardCurrency, m_strFuturesCycle);
			OKEX_WEB_SOCKET->API_FuturesKlineData(true, m_bSwapFutures, m_strKlineCycle, m_strCoinType, m_strStandardCurrency, m_strFuturesCycle);
			m_bNeedSubscribe = false;
		}
		if(!m_bLoginSuccess)
			OKEX_WEB_SOCKET->API_LoginFutures(m_apiKey, m_secretKey, time(NULL));
	}
	CString strTitle;
	strTitle.Format("%s-%s", (m_bSwapFutures ? "永续合约" : "交割合约"), m_strCoinType.c_str());
	CWnd *m_pMainWnd;
	m_pMainWnd = AfxGetMainWnd();
	if(m_pMainWnd)
		m_pMainWnd->SetWindowText(strTitle);
	m_bRun = true;
}


void COKExMartingaleDlg::SetHScroll()
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

void COKExMartingaleDlg::AddKlineData(SKlineData& data)
{
	if(KLINE_DATA_SIZE)
	{
		if(data.time - KLINE_DATA[KLINE_DATA_SIZE - 1].time != m_nKlineCycle)
		{
			CActionLog("all_kline", "差距%d秒", data.time - KLINE_DATA[KLINE_DATA_SIZE - 1].time);
			ComplementedKLine(data.time, int(data.time - KLINE_DATA[KLINE_DATA_SIZE-1].time - m_nKlineCycle)/m_nKlineCycle);
		}
	}
	tm _tm;
	localtime_s(&_tm, &data.time);
	_snprintf(data.szTime, 20, "%d-%02d-%02d %02d:%02d:%02d", _tm.tm_year + 1900, _tm.tm_mon + 1, _tm.tm_mday, _tm.tm_hour, _tm.tm_min, _tm.tm_sec);
	KLINE_DATA.push_back(data);
	_MakeMA();
}

void COKExMartingaleDlg::_MakeMA()
{
	int nIndex = 0;
	double numPrice = 0.0;
	for(int i=KLINE_DATA_SIZE-1; i>=0; i--)
	{
		numPrice += KLINE_DATA[i].closePrice;
		nIndex++;
		if(nIndex == 5)
			KLINE_DATA[KLINE_DATA_SIZE-1].ma5 = numPrice/5;
		else if(nIndex == 7)
			KLINE_DATA[KLINE_DATA_SIZE-1].ma7 = numPrice/7;
		else if(nIndex == 15)
			KLINE_DATA[KLINE_DATA_SIZE-1].ma15 = numPrice/15;
		else if(nIndex == 30)
			KLINE_DATA[KLINE_DATA_SIZE-1].ma30 = numPrice/30;
		else if(nIndex == 60)
			KLINE_DATA[KLINE_DATA_SIZE-1].ma60 = numPrice/60;
		else if(nIndex == 90)
		{
			KLINE_DATA[KLINE_DATA_SIZE-1].ma90 = numPrice/90;
			break;
		}
	}
}

void COKExMartingaleDlg::ComplementedKLine(time_t tNowKlineTick, int kLineCnt)
{
	time_t endTick = tNowKlineTick - m_nKlineCycle;
	time_t beginTick = endTick - (kLineCnt - 1)*m_nKlineCycle;
	std::string strFrom = CFuncCommon::LocaltimeToISO8601(beginTick);
	std::string strTo = CFuncCommon::LocaltimeToISO8601(endTick);
	std::string strKlineCycle = CFuncCommon::ToString(m_nKlineCycle);
	SHttpResponse resInfo;
	OKEX_HTTP->API_GetFuturesSomeKline(false, m_bSwapFutures, m_strCoinType, m_strStandardCurrency, m_strFuturesCycle, strKlineCycle, strFrom, strTo, &resInfo);
	Json::Value& retObj = resInfo.retObj;
	if(retObj.isArray())
	{
		for(int i = retObj.size() - 1; i >= 0; --i)
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
				strlocalLog.Format("{\"table\":\"swap/%s\",\"data\":[{\"candle\":[\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%d\",\"%s\"],\"instrument_id\":\"%s-%s-SWAP\"}]}",
					m_strKlineCycle.c_str(), CFuncCommon::LocaltimeToISO8601(data.time).c_str(),
					CFuncCommon::Double2String(data.openPrice + DOUBLE_PRECISION, m_nPriceDecimal).c_str(),
					CFuncCommon::Double2String(data.highPrice + DOUBLE_PRECISION, m_nPriceDecimal).c_str(),
					CFuncCommon::Double2String(data.lowPrice + DOUBLE_PRECISION, m_nPriceDecimal).c_str(),
					CFuncCommon::Double2String(data.closePrice + DOUBLE_PRECISION, m_nPriceDecimal).c_str(),
					data.volume,
					CFuncCommon::Double2String(data.volumeByCurrency + DOUBLE_PRECISION, m_nPriceDecimal).c_str(),
					m_strCoinType.c_str(),
					m_strStandardCurrency.c_str());
			}
			else
			{
				strlocalLog.Format("{\"table\":\"futures/%s\",\"data\":[{\"candle\":[\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%d\",\"%s\"],\"instrument_id\":\"%s-%s-%s\"}]}",
					m_strKlineCycle.c_str(), CFuncCommon::LocaltimeToISO8601(data.time).c_str(),
					CFuncCommon::Double2String(data.openPrice + DOUBLE_PRECISION, m_nPriceDecimal).c_str(),
					CFuncCommon::Double2String(data.highPrice + DOUBLE_PRECISION, m_nPriceDecimal).c_str(),
					CFuncCommon::Double2String(data.lowPrice + DOUBLE_PRECISION, m_nPriceDecimal).c_str(),
					CFuncCommon::Double2String(data.closePrice + DOUBLE_PRECISION, m_nPriceDecimal).c_str(),
					data.volume,
					CFuncCommon::Double2String(data.volumeByCurrency + DOUBLE_PRECISION, m_nPriceDecimal).c_str(),
					m_strCoinType.c_str(),
					m_strStandardCurrency.c_str(),
					m_strFuturesCycle.c_str());
			}
			CActionLog("all_kline", "%s", strlocalLog.GetBuffer());
			g_pDlg->AddKlineData(data);
		}
	}
}


void COKExMartingaleDlg::OnRevTickerInfo(STickerData &data)
{
	m_curTickData = data;
	m_curTickData.bValid = true;
	for(int i = 0; i < (int)m_vectorTradePairs.size(); ++i)
	{
		if(m_vectorTradePairs[i].open.orderID != "" && !CFuncCommon::CheckEqual(m_vectorTradePairs[i].open.minPrice, 0.0) && !CFuncCommon::CheckEqual(m_vectorTradePairs[i].open.maxPrice, 0.0))
		{
			if(m_curTickData.last < m_vectorTradePairs[i].open.minPrice)
			{
				m_vectorTradePairs[i].open.minPrice = m_curTickData.last;
				m_bSaveData = true;
			}
			if(m_curTickData.last > m_vectorTradePairs[i].open.maxPrice)
			{
				m_vectorTradePairs[i].open.maxPrice = m_curTickData.last;
				m_bSaveData = true;
			}
		}
	}
	_UpdateTradeShow();
	_UpdateProfitShow();
	std::string price = CFuncCommon::Double2String(m_curTickData.last+DOUBLE_PRECISION, m_nPriceDecimal);
	price += "[";
	price += CFuncCommon::FormatTimeStr(m_curTickData.time).c_str();
	price += "]";
	m_staticPrice.SetWindowText(price.c_str());
}

void COKExMartingaleDlg::Pong()
{
	m_tListenPong = 0;
}

void COKExMartingaleDlg::_QueryOrderInfo(std::string& orderID, const char* szLogTitle, const char* state_check)
{
	BEGIN_API_CHECK;
	SHttpResponse _resInfo;
	OKEX_HTTP->API_FuturesOrderInfo(false, m_bSwapFutures, m_strCoinType, m_strStandardCurrency, m_strFuturesCycle, orderID, &_resInfo);
	Json::Value& _retObj = _resInfo.retObj;
	if(_retObj.isObject() && _retObj["order_id"].isString() && (state_check==NULL || (_retObj["state"].isString() && _retObj["state"].asString() == state_check)))
	{
		SFuturesTradeInfo info;
		info.strClientOrderID = _retObj["client_oid"].asString();
		info.timeStamp = CFuncCommon::ISO8601ToTime(_retObj["timestamp"].asString());
		info.filledQTY = CFuncCommon::ToString(stoi(_retObj["filled_qty"].asString()));
		info.orderID = _retObj["order_id"].asString();
		info.price = stod(_retObj["price"].asString());
		info.priceAvg = stod(_retObj["price_avg"].asString());
		info.state = _retObj["state"].asString();
		std::string tradeType = _retObj["type"].asString();
		if(tradeType == "1")
			info.tradeType = eFuturesTradeType_OpenBull;
		else if(tradeType == "2")
			info.tradeType = eFuturesTradeType_OpenBear;
		else if(tradeType == "3")
			info.tradeType = eFuturesTradeType_CloseBull;
		else if(tradeType == "4")
			info.tradeType = eFuturesTradeType_CloseBear;
		else
			LOCAL_ERROR("未知交易类型[%s]", tradeType.c_str());
		info.size = CFuncCommon::ToString(stoi(_retObj["size"].asString()));
		g_pDlg->UpdateTradeInfo(info);
		CActionLog("trade", "[%s] order=%s, size=%s, filledQTY=%s, price=%s, state=%s, type=%d", szLogTitle, info.orderID.c_str(), info.size.c_str(), info.filledQTY.c_str(), CFuncCommon::Double2String(info.price, m_nPriceDecimal).c_str(), info.state.c_str(), info.tradeType);
		API_OK;
	}
	else
	{
		_checkStr = _resInfo.strRet;
		boost::this_thread::sleep(boost::posix_time::seconds(1));
	}
	API_CHECK;
	END_API_CHECK;

}

void COKExMartingaleDlg::OnLoginSuccess()
{
	if(m_vectorTradePairs.size())
	{
		for(int i=0; i<(int)m_vectorTradePairs.size(); ++i)
		{
			if(m_vectorTradePairs[i].open.orderID != "" && m_vectorTradePairs[i].open.state != state_filled && m_vectorTradePairs[i].open.state != state_cancelled)
				_QueryOrderInfo(m_vectorTradePairs[i].open.orderID, "订单初始化更新");
			if(m_vectorTradePairs[i].close.orderID != "" && m_vectorTradePairs[i].close.state != state_filled && m_vectorTradePairs[i].close.state != state_cancelled)
				_QueryOrderInfo(m_vectorTradePairs[i].close.orderID, "订单初始化更新");
		}
	}
	OKEX_WEB_SOCKET->API_FuturesOrderInfo(true, m_bSwapFutures, m_strCoinType, m_strStandardCurrency, m_strFuturesCycle);
	SHttpResponse resInfo;
	OKEX_HTTP->API_FuturesPositionInfo(false, m_bSwapFutures, m_strCoinType, m_strStandardCurrency, m_strFuturesCycle, &resInfo);
	if(resInfo.retObj["holding"].isArray())
	{
		SFuturesPositionInfo positionInfo;
		if(m_bSwapFutures)
		{
			for(int i=0; i<(int)resInfo.retObj["holding"].size(); ++i)
			{
				if(resInfo.retObj["holding"][i]["side"].asString() == "short")
				{
					positionInfo.bearCount = resInfo.retObj["holding"][i]["position"].asString();
					if(positionInfo.bearCount != "0")
					{
						positionInfo.bearFreeCount = resInfo.retObj["holding"][i]["avail_position"].asString();
						double priceAvg = stod(resInfo.retObj["holding"][i]["avg_cost"].asString());
						positionInfo.bearPriceAvg = CFuncCommon::Double2String(priceAvg, m_nPriceDecimal);
						positionInfo.bearMargin = resInfo.retObj["holding"][i]["margin"].asString();
						positionInfo.broken = resInfo.retObj["holding"][i]["liquidation_price"].asString();
					}
					
				}
				else if(resInfo.retObj["holding"][i]["side"].asString() == "long")
				{
					positionInfo.bullCount = resInfo.retObj["holding"][i]["position"].asString();
					if(positionInfo.bullCount != "0")
					{
						positionInfo.bullFreeCount = resInfo.retObj["holding"][i]["avail_position"].asString();
						double priceAvg = stod(resInfo.retObj["holding"][i]["avg_cost"].asString());
						positionInfo.bullPriceAvg = CFuncCommon::Double2String(priceAvg, m_nPriceDecimal);
						positionInfo.bullMargin = resInfo.retObj["holding"][i]["margin"].asString();
						positionInfo.broken = resInfo.retObj["holding"][i]["liquidation_price"].asString();
					}
				}
			}
		}
		else
		{
			positionInfo.bullCount = resInfo.retObj["holding"][0]["long_qty"].asString();
			if(positionInfo.bullCount != "0")
			{
				positionInfo.bullFreeCount = resInfo.retObj["holding"][0]["long_avail_qty"].asString();
				double priceAvg = stod(resInfo.retObj["holding"][0]["long_avg_cost"].asString());
				positionInfo.bullPriceAvg = CFuncCommon::Double2String(priceAvg, m_nPriceDecimal);
				positionInfo.bullMargin = resInfo.retObj["holding"][0]["long_margin"].asString();
			}
			
			positionInfo.bearCount = resInfo.retObj["holding"][0]["short_qty"].asString();
			if(positionInfo.bearCount != "0")
			{
				positionInfo.bearFreeCount = resInfo.retObj["holding"][0]["short_avail_qty"].asString();
				double priceAvg = stod(resInfo.retObj["holding"][0]["short_avg_cost"].asString());
				positionInfo.bearPriceAvg = CFuncCommon::Double2String(priceAvg, m_nPriceDecimal);
				positionInfo.bearMargin = resInfo.retObj["holding"][0]["short_margin"].asString();
			}
			positionInfo.broken = resInfo.retObj["holding"][0]["liquidation_price"].asString();
		}
		UpdatePositionInfo(positionInfo);
		OKEX_WEB_SOCKET->API_FuturesPositionInfo(true, m_bSwapFutures, m_strCoinType, m_strStandardCurrency, m_strFuturesCycle);
	}
	m_bLoginSuccess = true;
}

void COKExMartingaleDlg::UpdateAccountInfo(SFuturesAccountInfo& info)
{
	m_accountInfo = info;
	m_accountInfo.bValid = true;
	m_editCoin.SetWindowText(m_accountInfo.equity.c_str());
	std::string strCoin = m_strCoinType;
	if(m_strStandardCurrency == "USDT")
		strCoin = m_strStandardCurrency;
	strCoin += ":";
	m_staticCoin.SetWindowText(strCoin.c_str());
	_UpdateProfitShow();
}

void COKExMartingaleDlg::UpdatePositionInfo(SFuturesPositionInfo& info)
{
	m_positionInfo = info;
	m_positionInfo.bValid = true;
	_UpdatePositionShow();
}

void COKExMartingaleDlg::_UpdatePositionShow()
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
		m_listCtrlPostionInfo.SetItemText(0, 3, m_positionInfo.bullMargin.c_str());
		m_listCtrlPostionInfo.SetItemText(0, 4, m_positionInfo.bearCount.c_str());
		m_listCtrlPostionInfo.SetItemText(0, 5, m_positionInfo.bearFreeCount.c_str());
		m_listCtrlPostionInfo.SetItemText(0, 6, m_positionInfo.bearPriceAvg.c_str());
		m_listCtrlPostionInfo.SetItemText(0, 7, m_positionInfo.bearMargin.c_str());
		m_listCtrlPostionInfo.SetItemText(0, 8, m_positionInfo.broken.c_str());
	}
}

void COKExMartingaleDlg::__CheckTrade()
{
	if(!m_bRun)
		return;
	if(!m_curTickData.bValid)
		return;
	if(!m_accountInfo.bValid)
		return;
	if(!m_bLoginSuccess)
		return;
	switch(m_eTradeState)
	{
	case eTradeState_WaitOpen:
		{
			if(m_bStopWhenFinish)
			{
				m_bRun = false;
				break;
			}
			if(m_accountInfo.availBalance == "0")
				break;
			m_curOpenFinishIndex = -1;
			m_vectorTradePairs.clear();
			_CheckTrendDir();
			if(!_CheckTradeChance())
				break;;
			if(m_bOpenBull)
				m_staticDingDan.SetWindowText("做多");
			else
				m_staticDingDan.SetWindowText("做空");

			//开始下单
			double lastPrice = 0;
			for(int i = 0; i<m_martingaleStepCnt; ++i)
			{
				double offset;
				if(i == 0)
				{
					offset = 0.005;
					if(m_bOpenBull)
						lastPrice = m_curTickData.buy;
					else
						lastPrice = m_curTickData.sell;
				}
				else
					offset = m_martingaleMovePersent*(int)pow(2, i-1);
				double price;
				eFuturesTradeType tradeType;
				if(m_bOpenBull)
				{
					tradeType = eFuturesTradeType_OpenBull;
					price = lastPrice*(1 - offset);
				}
				else
				{
					tradeType = eFuturesTradeType_OpenBear;
					price = lastPrice*(1 + offset);
				}
				lastPrice = price;
				std::string strPrice = CFuncCommon::Double2String(price+DOUBLE_PRECISION, m_nPriceDecimal);
				int size = m_nFirstTradeSize*(i+1);
				std::string strSize = CFuncCommon::ToString(size);
				BEGIN_API_CHECK;
				SHttpResponse resInfo;
				std::string clientOrderID = CFuncCommon::GenUUID();
				OKEX_HTTP->API_FuturesTrade(false, m_bSwapFutures, tradeType, m_strCoinType, m_strStandardCurrency, m_strFuturesCycle, strPrice, strSize, m_strLeverage, clientOrderID, &resInfo);
				Json::Value& retObj = resInfo.retObj;
				if(retObj.isObject() && retObj["result"].isBool() && retObj["result"].asBool())
				{
					std::string strOrderID = retObj["order_id"].asString();
					SFuturesTradePairInfo pairs;
					pairs.open.orderID = strOrderID;
					pairs.open.strClientOrderID = clientOrderID;
					m_vectorTradePairs.push_back(pairs);
					m_bSaveData = true;
					_QueryOrderInfo(strOrderID, "新批次 开仓单");
					API_OK;
				}
				else
				{
					_checkStr = resInfo.strRet;
					boost::this_thread::sleep(boost::posix_time::seconds(1));
				}
				API_CHECK;
				END_API_CHECK;
			}
			m_tOpenTime = time(NULL);
			_SetTradeState(eTradeState_Trading);
		}
		break;
	case eTradeState_Trading:
		{
			int m_lastOpenFinishIndex = m_curOpenFinishIndex;
			double finishPrice = 0.0;
			for(int i = 0; i < (int)m_vectorTradePairs.size(); i++)
			{
				if(m_vectorTradePairs[i].open.orderID != "" && m_vectorTradePairs[i].open.state == state_filled)
				{
					m_curOpenFinishIndex = i;
					finishPrice = m_vectorTradePairs[i].open.price;
				}
			}
			if(m_curOpenFinishIndex != m_lastOpenFinishIndex)
			{
				if(m_curOpenFinishIndex == 0)
				{
					m_vectorTradePairs[0].open.bBeginStopProfit = true;
					m_bSaveData = true;
				}
				else
				{
					m_vectorTradePairs[0].open.bBeginStopProfit = false;
					m_bSaveData = true;
					//先把当前挂的卖单撤销了
					for(int i=0; i<m_curOpenFinishIndex; ++i)
					{
						if(m_vectorTradePairs[i].close.orderID != "")
						{
							if(m_vectorTradePairs[i].close.state == state_filled || m_vectorTradePairs[i].close.state == state_part_filled)
							{
								int closeFinish = atoi(m_vectorTradePairs[i].close.filledQTY.c_str());
								if(closeFinish != 0)
								{
									int openFinish = atoi(m_vectorTradePairs[i].open.filledQTY.c_str());
									openFinish -= closeFinish;
									if(m_vectorTradePairs[i].open.closeSize == "0")
										m_vectorTradePairs[i].open.closeSize = m_vectorTradePairs[i].close.filledQTY;
									else
									{
										int oldClose = atoi(m_vectorTradePairs[i].open.closeSize.c_str());
										oldClose += closeFinish;
										m_vectorTradePairs[i].open.closeSize = CFuncCommon::ToString(oldClose);
									}
									m_vectorTradePairs[i].open.filledQTY = CFuncCommon::ToString(openFinish);
								}
							}
							if(m_vectorTradePairs[i].close.state != state_filled && m_vectorTradePairs[i].close.state != state_cancelled)
							{
								BEGIN_API_CHECK;
								SHttpResponse resInfo;
								OKEX_HTTP->API_FuturesCancelOrder(false, m_bSwapFutures, m_strCoinType, m_strStandardCurrency, m_strFuturesCycle, m_vectorTradePairs[i].close.orderID, &resInfo);
								Json::Value& retObj = resInfo.retObj;
								if(retObj.isObject() && retObj["result"].isBool() && retObj["result"].asBool())
								{
									_QueryOrderInfo(m_vectorTradePairs[i].close.orderID, "新批次成交 老订单撤销成功", state_cancelled);
									API_OK;
								}
								else
									_checkStr = resInfo.strRet;
								API_CHECK;
								END_API_CHECK;
							}
							m_vectorTradePairs[i].close.Reset();
						}
					}
					//重新挂卖单
					finishPrice = m_vectorTradePairs[m_curOpenFinishIndex-1].open.priceAvg;
					std::string strPrice = CFuncCommon::Double2String(finishPrice+DOUBLE_PRECISION, m_nPriceDecimal);
					for(int i = 0; i <= m_curOpenFinishIndex; ++i)
					{
						if(m_vectorTradePairs[i].open.filledQTY != "0")
						{
							BEGIN_API_CHECK;
							SHttpResponse resInfo;
							std::string clientOrderID = CFuncCommon::GenUUID();
							eFuturesTradeType tradeType;
							if(m_bOpenBull)
								tradeType = eFuturesTradeType_CloseBull;
							else
								tradeType = eFuturesTradeType_CloseBear;
							OKEX_HTTP->API_FuturesTrade(false, m_bSwapFutures, tradeType, m_strCoinType, m_strStandardCurrency, m_strFuturesCycle, strPrice, m_vectorTradePairs[i].open.filledQTY, m_strLeverage, clientOrderID, &resInfo);
							Json::Value& retObj = resInfo.retObj;
							if(retObj.isObject() && retObj["result"].isBool() && retObj["result"].asBool())
							{
								std::string strOrderID = retObj["order_id"].asString();
								m_vectorTradePairs[i].close.orderID = strOrderID;
								m_vectorTradePairs[i].close.strClientOrderID = clientOrderID;
								m_bSaveData = true;
								_QueryOrderInfo(strOrderID, "新批次成交 开平仓单");
								API_OK;
							}
							else
							{
								_checkStr = resInfo.strRet;
								boost::this_thread::sleep(boost::posix_time::seconds(1));
							}
							API_CHECK;
							END_API_CHECK;
						}
					}
				}
			}
			if(m_vectorTradePairs.size())
			{
				//长时间未成交就撤销订单
				if(m_vectorTradePairs[0].open.state == state_open || m_vectorTradePairs[0].open.state == state_part_filled)
				{
					time_t tNow = time(NULL);
					bool bTimerOver = false;
					if(tNow - m_tOpenTime > m_nTradeTimeOver*60)
						bTimerOver = true;
					else
					{
						if(m_bOpenBull)
						{
							if(m_curTickData.last/m_vectorTradePairs[0].open.price > 1.025 && tNow - m_tOpenTime > DEFAULT_TRADE_TIMER_OVER*60)
								bTimerOver = true;
						}
						else
						{

							if(m_curTickData.last/m_vectorTradePairs[0].open.price < 0.975 && tNow - m_tOpenTime > DEFAULT_TRADE_TIMER_OVER*60)
								bTimerOver = true;
						}
					}
					if(bTimerOver)
					{
						bool bCancelAll = false;
						if(m_vectorTradePairs[0].open.state == state_open)
							bCancelAll = true;
						for(int i = 0; i<(int)m_vectorTradePairs.size(); ++i)
						{
							if(m_vectorTradePairs[i].open.orderID != "")
							{
								std::string strClientOrderID = "0";
								BEGIN_API_CHECK;
								SHttpResponse resInfo;
								OKEX_HTTP->API_FuturesCancelOrder(false, m_bSwapFutures, m_strCoinType, m_strStandardCurrency, m_strFuturesCycle, m_vectorTradePairs[i].open.orderID, &resInfo);
								Json::Value& retObj = resInfo.retObj;
								if (retObj.isObject() && retObj["result"].isBool() && retObj["result"].asBool())
								{
									_QueryOrderInfo(m_vectorTradePairs[i].open.orderID, "超时未成交 老订单撤销成功", state_cancelled);
									API_OK;
								}
								else
									_checkStr = resInfo.strRet;
								API_CHECK;
								END_API_CHECK;
							}
						}
						if(bCancelAll)
						{
							CActionLog("trade", "超时未成交");
							m_vectorTradePairs.clear();
							_SetTradeState(eTradeState_WaitOpen);
						}
						else
						{
							m_vectorTradePairs[0].open.bBeginStopProfit = true;
							m_bSaveData = true;
						}
					}
				}
				else
				{
					//检测完成状态,包括所有单子都成交,或剩最后一单未成交或成交中
					bool bAllFinish = true;
					for(int i = 0; i <= m_curOpenFinishIndex; ++i)
					{
						if(m_vectorTradePairs[i].open.orderID != "" && m_vectorTradePairs[i].open.state != state_filled && m_vectorTradePairs[i].open.state != state_cancelled)
						{
							bAllFinish = false;
							break;
						}
						if(m_vectorTradePairs[i].close.orderID != "" && m_vectorTradePairs[i].close.state != state_filled && m_vectorTradePairs[i].close.state != state_cancelled)
						{
							bAllFinish = false;
							break;
						}
					}
					if(m_curOpenFinishIndex == -1 || m_curOpenFinishIndex == 0)
						bAllFinish = false;
					if(bAllFinish)
					{
						//所有批次的订单都完成了
						if(m_curOpenFinishIndex == m_vectorTradePairs.size()-1)
						{
							_SetTradeState(eTradeState_WaitOpen);
							CActionLog("finish_trade", "[成功交易] 单批次");
							CActionLog("trade", "[成功交易] 单批次");
							LOCAL_INFO("成功交易一个批次");
							m_nFinishTimes++;
							m_staticFinishTimes.SetWindowText(CFuncCommon::ToString(m_nFinishTimes));
							m_bSaveData = true;
						}
						else
						{
							//只有前一单的卖单成交完了才判断下一单的情况
							if(m_vectorTradePairs[m_curOpenFinishIndex].close.orderID != "" && m_vectorTradePairs[m_curOpenFinishIndex].close.state == state_filled)
							{
								SFuturesTradePairInfo& pairsInfo = m_vectorTradePairs[m_curOpenFinishIndex + 1];
								//下一单未成交
								if(pairsInfo.open.state == state_open)
								{
									//把所有单子撤了
									for(int cancelIndex=m_curOpenFinishIndex+1; cancelIndex<(int)m_vectorTradePairs.size(); ++cancelIndex)
									{
										BEGIN_API_CHECK;
										SHttpResponse resInfo;
										OKEX_HTTP->API_FuturesCancelOrder(false, m_bSwapFutures, m_strCoinType, m_strStandardCurrency, m_strFuturesCycle, m_vectorTradePairs[cancelIndex].open.orderID, &resInfo);
										Json::Value& retObj = resInfo.retObj;
										if(retObj.isObject() && retObj["result"].isBool() && retObj["result"].asBool())
										{
											CActionLog("trade", "[最后一单 撤销订单成功] order=%s", m_vectorTradePairs[cancelIndex].open.orderID.c_str());
											API_OK;
										}
										else
											_checkStr = resInfo.strRet;
										API_CHECK;
										END_API_CHECK;
									}
									_SetTradeState(eTradeState_WaitOpen);
									CActionLog("finish_trade", "[成功交易] 单批次");
									CActionLog("trade", "[成功交易] 单批次");
									LOCAL_INFO("成功交易一个批次");
									m_nFinishTimes++;
									m_bSaveData = true;
									m_staticFinishTimes.SetWindowText(CFuncCommon::ToString(m_nFinishTimes));
								}
								else if(pairsInfo.open.state == state_cancelled || pairsInfo.open.state == state_part_filled)
								{
									//如果未成交先撤单
									if(pairsInfo.open.state == state_part_filled)
									{
										BEGIN_API_CHECK;
										SHttpResponse resInfo;
										OKEX_HTTP->API_FuturesCancelOrder(false, m_bSwapFutures, m_strCoinType, m_strStandardCurrency, m_strFuturesCycle, pairsInfo.open.orderID, &resInfo);
										Json::Value& retObj = resInfo.retObj;
										if (retObj.isObject() && retObj["result"].isBool() && retObj["result"].asBool())
										{
											_QueryOrderInfo(pairsInfo.open.orderID, "最后一单 撤销订单成功", state_cancelled);
											API_OK;
										}
										else
											_checkStr = resInfo.strRet;
										API_CHECK;
										END_API_CHECK;
										//直接用卖一价挂卖单
										eFuturesTradeType type;
										std::string strPrice;
										if(m_bOpenBull)
										{
											type = eFuturesTradeType_CloseBull;
											strPrice = CFuncCommon::Double2String(m_curTickData.sell + DOUBLE_PRECISION, m_nPriceDecimal);
										}
										else
										{
											type = eFuturesTradeType_CloseBear;
											strPrice = CFuncCommon::Double2String(m_curTickData.buy + DOUBLE_PRECISION, m_nPriceDecimal);
										}
										BEGIN_API_CHECK;
										SHttpResponse resInfo;
										std::string clientOrderID = CFuncCommon::GenUUID();
										OKEX_HTTP->API_FuturesTrade(false, m_bSwapFutures, type, m_strCoinType, m_strStandardCurrency, m_strFuturesCycle, strPrice, pairsInfo.open.filledQTY, m_strLeverage, clientOrderID, &resInfo);
										Json::Value& retObj = resInfo.retObj;
										if (retObj.isObject() && retObj["result"].isBool() && retObj["result"].asBool())
										{
											std::string strOrderID = retObj["order_id"].asString();
											pairsInfo.close.strClientOrderID = clientOrderID;
											pairsInfo.close.orderID = strOrderID;
											m_bSaveData = true;
											_QueryOrderInfo(strOrderID, "最后一单 开平仓单");
											API_OK;
										}
										else
											_checkStr = resInfo.strRet;
										API_CHECK;
										END_API_CHECK;
									}
									else
									{
										if(pairsInfo.close.orderID == "")
										{
											_SetTradeState(eTradeState_WaitOpen);
											CActionLog("finish_trade", "[成功交易] 单批次");
											CActionLog("trade", "[成功交易] 单批次");
											LOCAL_INFO("成功交易一个批次");
											m_nFinishTimes++;
											m_staticFinishTimes.SetWindowText(CFuncCommon::ToString(m_nFinishTimes));
											m_bSaveData = true;
										}
										else
										{
											if(pairsInfo.close.state == state_cancelled || pairsInfo.close.state == state_filled)
											{
												_SetTradeState(eTradeState_WaitOpen);
												CActionLog("finish_trade", "[成功交易] 单批次");
												CActionLog("trade", "[成功交易] 单批次");
												LOCAL_INFO("成功交易一个批次");
												m_nFinishTimes++;
												m_staticFinishTimes.SetWindowText(CFuncCommon::ToString(m_nFinishTimes));
												m_bSaveData = true;
											}
										}
									}
								}
							}
						}
					}
					else
					{
						SFuturesTradePairInfo& pairsInfo = m_vectorTradePairs[0];
						if(pairsInfo.open.bBeginStopProfit)
						{
							if(pairsInfo.close.orderID == "")
							{
								double openPrice = pairsInfo.open.price;
								if(m_bStopProfitMove)
								{
									if(m_bOpenBull)
									{
										if(m_curTickData.last > openPrice)
										{
											double up = (m_curTickData.last - openPrice) / openPrice;
											int nowStep = int(up / m_stopProfitFactor);
											if(pairsInfo.open.stopProfit == 0)
											{
												if(nowStep >= 3)
												{
													pairsInfo.open.stopProfit = 1;
													m_bSaveData = true;
												}
											}
											else
											{
												if(nowStep - pairsInfo.open.stopProfit >= 2)
												{
													pairsInfo.open.stopProfit = nowStep - 1;
													m_bSaveData = true;
												}
											}
										}
									}
									else
									{
										if(m_curTickData.last < openPrice)
										{
											double down = (openPrice - m_curTickData.last) / openPrice;
											int nowStep = int(down / m_stopProfitFactor);
											if(pairsInfo.open.stopProfit == 0)
											{
												if(nowStep >= 3)
												{
													pairsInfo.open.stopProfit = 1;
													m_bSaveData = true;
												}
											}
											else
											{
												if(nowStep - pairsInfo.open.stopProfit >= 2)
												{
													pairsInfo.open.stopProfit = nowStep - 1;
													m_bSaveData = true;
												}
											}
										}
									}
									if(pairsInfo.open.stopProfit)
									{
										bool bCloseProfit = false;
										std::string strPrice;
										eFuturesTradeType type;
										if(m_bOpenBull)
										{
											bCloseProfit = (m_curTickData.last <= (openPrice*(1+pairsInfo.open.stopProfit*m_stopProfitFactor+m_stopProfitFactor/2)));
											strPrice = CFuncCommon::Double2String(m_curTickData.sell+DOUBLE_PRECISION, m_nPriceDecimal);
											type = eFuturesTradeType_CloseBull;
										}
										else
										{
											bCloseProfit = (m_curTickData.last >= (openPrice*(1-pairsInfo.open.stopProfit*m_stopProfitFactor-m_stopProfitFactor/2)));
											strPrice = CFuncCommon::Double2String(m_curTickData.buy+DOUBLE_PRECISION, m_nPriceDecimal);
											type = eFuturesTradeType_CloseBear;
										}
										if(bCloseProfit)
										{
											//把所有成交的open单子市价处理,其余撤单
											for(int i = 0; i<(int)m_vectorTradePairs.size(); ++i)
											{
												if(m_vectorTradePairs[i].open.orderID != "")
												{
													//先市价处理
													int size = atoi(m_vectorTradePairs[i].open.filledQTY.c_str());
													if(m_vectorTradePairs[i].open.state == state_part_filled || m_vectorTradePairs[i].open.state == state_filled || (m_vectorTradePairs[i].open.state == state_cancelled && size > 0))
													{
														BEGIN_API_CHECK;
														SHttpResponse resInfo;
														std::string clientOrderID = CFuncCommon::GenUUID();
														OKEX_HTTP->API_FuturesTrade(false, m_bSwapFutures, type, m_strCoinType, m_strStandardCurrency, m_strFuturesCycle, strPrice, m_vectorTradePairs[i].open.filledQTY, m_strLeverage, clientOrderID, &resInfo);
														Json::Value& retObj = resInfo.retObj;
														if (retObj.isObject() && retObj["result"].isBool() && retObj["result"].asBool())
														{
															std::string strOrderID = retObj["order_id"].asString();
															m_vectorTradePairs[i].close.strClientOrderID = clientOrderID;
															m_vectorTradePairs[i].close.orderID = strOrderID;
															m_bSaveData = true;
															_QueryOrderInfo(strOrderID, "止盈 开平仓单");
															API_OK;
														}
														else
															_checkStr = resInfo.strRet;
														API_CHECK;
														END_API_CHECK;
													}
												}
											}
										}
									}

								}
								else
								{
									std::string strPrice;
									eFuturesTradeType type;
									if(m_bOpenBull)
									{
										strPrice = CFuncCommon::Double2String(openPrice*(1+m_stopProfitFactor)+DOUBLE_PRECISION, m_nPriceDecimal);
										type = eFuturesTradeType_CloseBull;
									}
									else
									{
										strPrice = CFuncCommon::Double2String(openPrice*(1-m_stopProfitFactor)+DOUBLE_PRECISION, m_nPriceDecimal);
										type = eFuturesTradeType_CloseBear;
									}
									BEGIN_API_CHECK;
									SHttpResponse resInfo;
									std::string clientOrderID = CFuncCommon::GenUUID();
									OKEX_HTTP->API_FuturesTrade(false, m_bSwapFutures, eFuturesTradeType_OpenBear, m_strCoinType, m_strStandardCurrency, m_strFuturesCycle, strPrice, pairsInfo.open.filledQTY, m_strLeverage, clientOrderID, &resInfo);
									Json::Value& retObj = resInfo.retObj;
									if (retObj.isObject() && retObj["result"].isBool() && retObj["result"].asBool())
									{
										std::string strOrderID = retObj["order_id"].asString();
										pairsInfo.close.strClientOrderID = clientOrderID;
										pairsInfo.close.orderID = strOrderID;
										m_bSaveData = true;
										_QueryOrderInfo(strOrderID, "止盈 开平仓单");
										API_OK;
									}
									else
									{
										_checkStr = resInfo.strRet;
										boost::this_thread::sleep(boost::posix_time::seconds(1));
									}
									API_CHECK;
									END_API_CHECK;
								}

							}
							
						}
						//检测头单止盈情况
						if(m_vectorTradePairs[0].close.orderID != "" && m_vectorTradePairs[0].close.state == state_filled)
						{
							bool bAllEmpty = true;
							for(int i = 1; i < (int)m_vectorTradePairs.size(); i++)
							{
								if(m_vectorTradePairs[i].close.orderID != "" && m_vectorTradePairs[i].close.state != state_filled)
								{
									bAllEmpty = false;
									break;
								}
							}
							if(bAllEmpty)
							{
								m_nStopProfitTimes++;
								m_bSaveData = true;
								m_staticStopProfitTimes.SetWindowText(CFuncCommon::ToString(m_nStopProfitTimes));
								CActionLog("finish_trade", "[止盈成功] 单批次");
								CActionLog("trade", "[止盈成功] 单批次");
								LOCAL_INFO("成功止盈一个批次");
								//把所有未成交的单子撤了
								for(int i = 0; i < (int)m_vectorTradePairs.size(); ++i)
								{
									if(m_vectorTradePairs[i].open.orderID != "")
									{
										if(m_vectorTradePairs[i].open.state != state_filled)
										{
											BEGIN_API_CHECK;
											SHttpResponse resInfo;
											OKEX_HTTP->API_FuturesCancelOrder(false, m_bSwapFutures, m_strCoinType, m_strStandardCurrency, m_strFuturesCycle, m_vectorTradePairs[i].open.orderID, &resInfo);
											Json::Value& retObj = resInfo.retObj;
											if (retObj.isObject() && retObj["result"].isBool() && retObj["result"].asBool())
											{
												CActionLog("trade", "[止盈 撤销订单成功] order=%s", pairsInfo.open.orderID.c_str());
												API_OK;
											}
											else
												_checkStr = resInfo.strRet;
											API_CHECK;
											END_API_CHECK;
										}
									}
									if(m_vectorTradePairs[i].close.orderID != "")
									{
										if(m_vectorTradePairs[i].close.state != state_filled)
										{
											BEGIN_API_CHECK;
											SHttpResponse resInfo;
											OKEX_HTTP->API_FuturesCancelOrder(false, m_bSwapFutures, m_strCoinType, m_strStandardCurrency, m_strFuturesCycle, m_vectorTradePairs[i].close.orderID, &resInfo);
											Json::Value& retObj = resInfo.retObj;
											if (retObj.isObject() && retObj["result"].isBool() && retObj["result"].asBool())
											{
												CActionLog("trade", "[止盈 撤销订单成功] order=%s", pairsInfo.open.orderID.c_str());
												API_OK;
											}
											else
												_checkStr = resInfo.strRet;
											API_CHECK;
											END_API_CHECK;
										}
									}
								}
								_SetTradeState(eTradeState_WaitOpen);
							}
						}
						
					}
				}
			}
		}
		break;
	default:
		break;
	}

}

void COKExMartingaleDlg::_CheckTrendDir()
{
	if(m_nTrendType == 0)
		m_bOpenBull = true;
	else if(m_nTrendType == 1)
		m_bOpenBull = false;
	else if(m_nTrendType == 2)
		m_bOpenBull = true;
}

bool COKExMartingaleDlg::_CheckTradeChance()
{
	if(m_nLastCheckKline == KLINE_DATA_SIZE)
		return false;
	m_nLastCheckKline = KLINE_DATA_SIZE;
	if(m_bOpenBull)
	{
		//价格8个周期中有5个在MA5以上就不下单了
		CString szBuffer = "下单条件检测";
		int ma5Num = 0;
		double minPrice = m_curTickData.last;
		double maxPrice = m_curTickData.last;
		int allCheckCnt = 0;
		for(int i=KLINE_DATA_SIZE-1; i>KLINE_DATA_SIZE-1-8; --i)
		{
			CString _szBuffer;
			_szBuffer.Format("\n[time=%s][close=%.2f][ma5=%.2f]", KLINE_DATA[i].szTime, CFuncCommon::Round(KLINE_DATA[i].closePrice, m_nPriceDecimal), CFuncCommon::Round(KLINE_DATA[i].ma5, m_nPriceDecimal));
			szBuffer.Append(_szBuffer);
			if(KLINE_DATA[i].closePrice > KLINE_DATA[i].ma5)
				ma5Num++;
			if(i>KLINE_DATA_SIZE-1-4)
			{
				if(KLINE_DATA[i].closePrice > maxPrice)
					maxPrice = KLINE_DATA[i].closePrice;
				if(KLINE_DATA[i].closePrice < minPrice)
					minPrice = KLINE_DATA[i].closePrice;
			}
			allCheckCnt++;
		}
		LOCAL_INFO(szBuffer.GetBuffer());
		if(allCheckCnt == 8)
		{
			if(ma5Num > 5)
			{
				LOCAL_INFO("上升期,暂停下单");
				return false;
			}
			if(maxPrice/minPrice > 1.025)
			{
				LOCAL_INFO("快速上升期,暂停下单");
				return false;
			}
		}
		else
			LOCAL_INFO("检测周期不满足");
	}
	else
	{
		//价格8个周期中有5个在MA5以下就不下单了
		CString szBuffer = "下单条件检测";
		int ma5Num = 0;
		double minPrice = m_curTickData.last;
		double maxPrice = m_curTickData.last;
		int allCheckCnt = 0;
		for(int i=KLINE_DATA_SIZE-1; i>KLINE_DATA_SIZE-1-8; --i)
		{
			CString _szBuffer;
			_szBuffer.Format("\n[time=%s][close=%.2f][ma5=%.2f]", KLINE_DATA[i].szTime, CFuncCommon::Round(KLINE_DATA[i].closePrice, m_nPriceDecimal), CFuncCommon::Round(KLINE_DATA[i].ma5, m_nPriceDecimal));
			szBuffer.Append(_szBuffer);
			if(KLINE_DATA[i].closePrice < KLINE_DATA[i].ma5)
				ma5Num++;
			if(i>KLINE_DATA_SIZE-1-4)
			{
				if(KLINE_DATA[i].closePrice > maxPrice)
					maxPrice = KLINE_DATA[i].closePrice;
				if(KLINE_DATA[i].closePrice < minPrice)
					minPrice = KLINE_DATA[i].closePrice;
			}
			allCheckCnt++;
		}
		if(allCheckCnt == 8)
		{
			if(ma5Num > 5)
			{
				LOCAL_INFO("下跌期,暂停下单");
				return false;
			}
			if(maxPrice/minPrice > 1.025)
			{
				LOCAL_INFO("快速下跌期,暂停下单");
				return false;
			}
		}
		else
			LOCAL_INFO("检测周期不满足");
	}
	return true;
}

void COKExMartingaleDlg::UpdateTradeInfo(SFuturesTradeInfo& info)
{
	for(int i = 0; i<(int)m_vectorTradePairs.size(); i++)
	{
		if(m_vectorTradePairs[i].open.orderID == info.orderID && 
		   m_vectorTradePairs[i].open.state != state_filled)
		{
			if((m_vectorTradePairs[i].open.state==state_open || m_vectorTradePairs[i].open.state=="") && info.state!=state_open && m_curTickData.bValid && CFuncCommon::CheckEqual(m_vectorTradePairs[i].open.minPrice, 0.0) && CFuncCommon::CheckEqual(m_vectorTradePairs[i].open.maxPrice, 0.0))
			{
				m_vectorTradePairs[i].open.minPrice = m_curTickData.last;
				m_vectorTradePairs[i].open.maxPrice = m_curTickData.last;
				m_bSaveData = true;
			}
			m_vectorTradePairs[i].open.timeStamp = info.timeStamp;
			if(m_vectorTradePairs[i].open.closeSize != "0")
			{
				int closeFinish = atoi(m_vectorTradePairs[i].open.closeSize.c_str());
				int filledQTY = atoi(info.filledQTY.c_str());
				filledQTY -= closeFinish;
				m_vectorTradePairs[i].open.filledQTY = CFuncCommon::ToString(filledQTY);
			}
			else
				m_vectorTradePairs[i].open.filledQTY = info.filledQTY;
			m_vectorTradePairs[i].open.price = info.price;
			m_vectorTradePairs[i].open.priceAvg = info.priceAvg;
			m_vectorTradePairs[i].open.state = info.state;
			m_vectorTradePairs[i].open.tradeType = info.tradeType;
			m_vectorTradePairs[i].open.size = info.size;
			info.strClientOrderID = m_vectorTradePairs[i].open.strClientOrderID;
			break;
		}
		else if(m_vectorTradePairs[i].close.orderID == info.orderID &&
				m_vectorTradePairs[i].close.state != state_filled)
		{
			m_vectorTradePairs[i].close.timeStamp = info.timeStamp;
			m_vectorTradePairs[i].close.filledQTY = info.filledQTY;
			m_vectorTradePairs[i].close.price = info.price;
			m_vectorTradePairs[i].close.priceAvg = info.priceAvg;
			m_vectorTradePairs[i].close.state = info.state;
			m_vectorTradePairs[i].close.tradeType = info.tradeType;
			m_vectorTradePairs[i].close.size = info.size;
			info.strClientOrderID = m_vectorTradePairs[i].close.strClientOrderID;
			break;
		}
	}
	_UpdateTradeShow();
}

void COKExMartingaleDlg::_UpdateTradeShow()
{
	m_listCtrlOpen.DeleteAllItems();
	m_listCtrlClose.DeleteAllItems();
	CString szFormat;
	for(int i = 0; i<(int)m_vectorTradePairs.size(); i++)
	{
		if(m_vectorTradePairs[i].open.orderID != "")
		{
			m_listCtrlOpen.InsertItem(i, "");
			szFormat.Format("%s", CFuncCommon::Double2String(m_vectorTradePairs[i].open.price, m_nPriceDecimal).c_str());
			m_listCtrlOpen.SetItemText(i, 0, szFormat);
			szFormat.Format("%s/%s", m_vectorTradePairs[i].open.filledQTY.c_str(), m_vectorTradePairs[i].open.size.c_str());
			m_listCtrlOpen.SetItemText(i, 1, szFormat);
			szFormat.Format("%s", m_vectorTradePairs[i].open.closeSize.c_str());
			m_listCtrlOpen.SetItemText(i, 2, szFormat);

			if(m_vectorTradePairs[i].open.state == state_cancelled)
				m_listCtrlOpen.SetItemText(i, 3, "cancelled");
			else if(m_vectorTradePairs[i].open.state == state_open)
				m_listCtrlOpen.SetItemText(i, 3, "open");
			else if(m_vectorTradePairs[i].open.state == state_part_filled)
				m_listCtrlOpen.SetItemText(i, 3, "part_filled");
			else if(m_vectorTradePairs[i].open.state == state_filled)
				m_listCtrlOpen.SetItemText(i, 3, "filled");

			if(m_curTickData.bValid)
			{
				int filledSize = atoi(m_vectorTradePairs[i].open.filledQTY.c_str());
				if(filledSize > 0)
				{
					double openPrice = m_vectorTradePairs[i].open.price;
					double will_sell = m_curTickData.last;
					if(m_vectorTradePairs[i].open.tradeType == eFuturesTradeType_OpenBull)
					{
						if(will_sell >= openPrice)
						{
							szFormat.Format("%.4f%%", ((will_sell-openPrice)/openPrice)*m_nLeverage*100);
							m_listCtrlOpen.SetItemText(i, 4, szFormat);
						}
						else
						{
							szFormat.Format("-%.4f%%", ((openPrice-will_sell)/openPrice)*m_nLeverage*100);
							m_listCtrlOpen.SetItemText(i, 4, szFormat);
						}
					}
					else
					{
						if(will_sell <= openPrice)
						{
							szFormat.Format("%.4f%%", ((openPrice-will_sell)/openPrice)*m_nLeverage*100);
							m_listCtrlOpen.SetItemText(i, 4, szFormat);
						}
						else
						{
							szFormat.Format("-%.4f%%", ((will_sell-openPrice)/openPrice)*m_nLeverage*100);
							m_listCtrlOpen.SetItemText(i, 4, szFormat);
						}
					}
				}
			}
			szFormat.Format("%s", CFuncCommon::Double2String(m_vectorTradePairs[i].open.minPrice + DOUBLE_PRECISION, m_nPriceDecimal).c_str());
			m_listCtrlOpen.SetItemText(i, 5, szFormat);
			szFormat.Format("%s", CFuncCommon::Double2String(m_vectorTradePairs[i].open.maxPrice + DOUBLE_PRECISION, m_nPriceDecimal).c_str());
			m_listCtrlOpen.SetItemText(i, 6, szFormat);

			if(m_vectorTradePairs[i].open.stopProfit)
			{
				std::string strPrice;
				if(m_bOpenBull)
					strPrice = CFuncCommon::Double2String((m_vectorTradePairs[i].open.price*(1 + m_vectorTradePairs[i].open.stopProfit*m_stopProfitFactor + m_stopProfitFactor / 2)) + DOUBLE_PRECISION, m_nPriceDecimal);
				else
					strPrice = CFuncCommon::Double2String((m_vectorTradePairs[i].open.price*(1 - m_vectorTradePairs[i].open.stopProfit*m_stopProfitFactor - m_stopProfitFactor / 2)) + DOUBLE_PRECISION, m_nPriceDecimal);
				m_listCtrlOpen.SetItemText(i, 7, strPrice.c_str());
			}
			else
				m_listCtrlOpen.SetItemText(i, 7, "0");

			szFormat.Format("%d", m_vectorTradePairs[i].open.stopProfit);
			m_listCtrlOpen.SetItemText(i, 7, szFormat);
		}
		if(m_vectorTradePairs[i].close.orderID != "")
		{
			m_listCtrlClose.InsertItem(i, "");
			szFormat.Format("%s", CFuncCommon::Double2String(m_vectorTradePairs[i].close.price, m_nPriceDecimal).c_str());
			m_listCtrlClose.SetItemText(i, 0, szFormat);
			szFormat.Format("%s/%s", m_vectorTradePairs[i].close.filledQTY.c_str(), m_vectorTradePairs[i].close.size.c_str());
			m_listCtrlClose.SetItemText(i, 1, szFormat);

			if(m_vectorTradePairs[i].close.state == state_cancelled)
				m_listCtrlClose.SetItemText(i, 2, "cancelled");
			else if(m_vectorTradePairs[i].close.state == state_open)
				m_listCtrlClose.SetItemText(i, 2, "open");
			else if(m_vectorTradePairs[i].close.state == state_part_filled)
				m_listCtrlClose.SetItemText(i, 2, "part_filled");
			else if(m_vectorTradePairs[i].close.state == state_filled)
				m_listCtrlClose.SetItemText(i, 2, "filled");
		}
	}
}

void COKExMartingaleDlg::__InitConfigCtrl()
{
	std::string strTemp = m_config.get("futures", "coinType", "");
	if (strTemp == "BTC")
		m_combCoinType.SetCurSel(0);
	else if (strTemp == "LTC")
		m_combCoinType.SetCurSel(1);
	else if (strTemp == "ETH")
		m_combCoinType.SetCurSel(2);
	else if (strTemp == "ETC")
		m_combCoinType.SetCurSel(3);
	else if (strTemp == "BTG")
		m_combCoinType.SetCurSel(4);
	else if (strTemp == "XRP")
		m_combCoinType.SetCurSel(5);
	else if (strTemp == "EOS")
		m_combCoinType.SetCurSel(6);
	else if (strTemp == "BCH")
		m_combCoinType.SetCurSel(7);
	else if (strTemp == "TRX")
		m_combCoinType.SetCurSel(8);

	strTemp = m_config.get("futures", "standardCurrency", "");
	if (strTemp == "USD")
		m_combStandardCurrency.SetCurSel(0);
	else if(strTemp == "USDT")
		m_combStandardCurrency.SetCurSel(1);

	strTemp = m_config.get("futures", "futuresCycle", "");
	m_editFuturesCycle.SetWindowText(strTemp.c_str());
	strTemp = m_config.get("futures", "leverage", "");
	if (strTemp == "10")
		m_combLeverage.SetCurSel(0);
	else if (strTemp == "20")
		m_combLeverage.SetCurSel(1);
	strTemp = m_config.get("futures", "futuresType", "");
	if (strTemp == "交割合约")
		m_combFuturesType.SetCurSel(0);
	else if (strTemp == "永续合约")
		m_combFuturesType.SetCurSel(1);

	strTemp = m_config.get("futures", "firstTradeSize", "");
	m_editFirstTradeSize.SetWindowText(strTemp.c_str());

	strTemp = m_config.get("futures", "martingaleStepCnt", "");
	m_editMartingaleStepCnt.SetWindowText(strTemp.c_str());

	strTemp = m_config.get("futures", "martingaleMovePersent", "");
	m_editMartingaleMovePersent.SetWindowText(strTemp.c_str());

	strTemp = m_config.get("futures", "stopProfitFactor", "");
	m_editStopProfitFactor.SetWindowText(strTemp.c_str());
	
	strTemp = m_config.get("futures", "stopProfitType", "");
	if(strTemp == "")
		m_btnStopProfitMove.SetCheck(1);
	else if(strTemp == "move")
		m_btnStopProfitMove.SetCheck(1);
	else if(strTemp == "fix")
		m_btnStopProfitFix.SetCheck(1);

	strTemp = m_config.get("futures", "trend", "");
	if(strTemp == "")
		m_btnTrendAuto.SetCheck(1);
	else if(strTemp == "bull")
		m_btnTrendBull.SetCheck(1);
	else if(strTemp == "bear")
		m_btnTrendBear.SetCheck(1);
	else if(strTemp == "auto")
		m_btnTrendAuto.SetCheck(1);

	strTemp = m_config.get("futures", "beginMoney", "");
	m_editCost.SetWindowText(strTemp.c_str());

	strTemp = m_config.get("futures", "cycle", "");
	if(strTemp == "1分钟")
		m_combKLineCycle.SetCurSel(0);
	else if(strTemp == "3分钟")
		m_combKLineCycle.SetCurSel(1);
	else if(strTemp == "5分钟")
		m_combKLineCycle.SetCurSel(2);
	else if(strTemp == "15分钟")
		m_combKLineCycle.SetCurSel(3);
	else if(strTemp == "30分钟")
		m_combKLineCycle.SetCurSel(4);
	else if(strTemp == "1小时")
		m_combKLineCycle.SetCurSel(5);
	else if(strTemp == "2小时")
		m_combKLineCycle.SetCurSel(6);
	else if(strTemp == "4小时")
		m_combKLineCycle.SetCurSel(7);
	else if(strTemp == "6小时")
		m_combKLineCycle.SetCurSel(8);
	else if(strTemp == "12小时")
		m_combKLineCycle.SetCurSel(9);
	else if(strTemp == "1日")
		m_combKLineCycle.SetCurSel(10);
	else if(strTemp == "1周")
		m_combKLineCycle.SetCurSel(11);

	strTemp = m_config.get("futures", "tradeTimerOver", "");
	m_editTradeTimerOver.SetWindowText(strTemp.c_str());
}

bool COKExMartingaleDlg::__SaveConfigCtrl()
{
	CString strFuturesType;
	m_combFuturesType.GetWindowText(strFuturesType);
	if (strFuturesType == "")
	{
		MessageBox("未选择合约类型");
		return false;
	}
	CString strCoinType;
	m_combCoinType.GetWindowText(strCoinType);
	if (strCoinType == "")
	{
		MessageBox("未选择合约货币");
		return false;
	}

	CString strStandardCurrency;
	m_combStandardCurrency.GetWindowText(strStandardCurrency);
	if(strStandardCurrency == "")
	{
		MessageBox("未选择本位货币");
		return false;
	}

	CString strFuturesCycle;
	m_editFuturesCycle.GetWindowText(strFuturesCycle);
	if (strFuturesCycle == "")
	{
		MessageBox("未填写合约交割期");
		return false;
	}
	CString strLeverage;
	m_combLeverage.GetWindowText(strLeverage);
	if (strLeverage == "")
	{
		MessageBox("未选择杠杆");
		return false;
	}
	CString strKLineCycle;
	m_combKLineCycle.GetWindowText(strKLineCycle);
	if(strKLineCycle == "")
	{
		MessageBox("未选择K线周期");
		return false;
	}

	CString strMartingaleStepCnt;
	m_editMartingaleStepCnt.GetWindowText(strMartingaleStepCnt);
	if(strMartingaleStepCnt == "")
	{
		MessageBox("未填写马丁格尔交易次数");
		return false;
	}
	CString strMartingaleMovePersent;
	m_editMartingaleMovePersent.GetWindowText(strMartingaleMovePersent);
	if(strMartingaleMovePersent == "")
	{
		MessageBox("未填写马丁格尔跌幅");
		return false;
	}
	CString strStopProfitFactor;
	m_editStopProfitFactor.GetWindowText(strStopProfitFactor);
	if(strStopProfitFactor == "")
	{
		MessageBox("未填写止盈系数");
		return false;
	}

	CString strFirstTradeSize;
	m_editFirstTradeSize.GetWindowText(strFirstTradeSize);
	if(strFirstTradeSize == "")
	{
		MessageBox("未填写初始下单张数");
		return false;
	}

	CString strTradeTimerOver;
	m_editTradeTimerOver.GetWindowText(strTradeTimerOver);
	if(strTradeTimerOver == "")
	{
		MessageBox("未填写挂单超时时间");
		return false;
	}

	CString szCost = "";
	m_editCost.GetWindowText(szCost);
	
	m_strCoinType = strCoinType.GetBuffer();
	m_strStandardCurrency = strStandardCurrency.GetBuffer();
	m_strFuturesCycle = strFuturesCycle.GetBuffer();
	m_strLeverage = strLeverage.GetBuffer();
	m_nLeverage = stoi(m_strLeverage);
	if (strFuturesType == "永续合约")
		m_bSwapFutures = true;
	else
		m_bSwapFutures = false;

	m_martingaleStepCnt = atoi(strMartingaleStepCnt.GetBuffer());
	m_martingaleMovePersent = stod(strMartingaleMovePersent.GetBuffer());

	m_stopProfitFactor = stod(strStopProfitFactor.GetBuffer());
	if(szCost != "")
		m_beginMoney = stod(szCost.GetBuffer());
	if(m_btnStopProfitMove.GetCheck())
		m_bStopProfitMove = true;
	else
		m_bStopProfitMove = false;

	m_nTrendType = 0;
	if(m_btnTrendAuto.GetCheck())
		m_nTrendType = 2;
	else if(m_btnTrendBull.GetCheck())
		m_nTrendType = 0;
	else if(m_btnTrendBear.GetCheck())
		m_nTrendType = 1;

	m_nFirstTradeSize = atoi(strFirstTradeSize.GetBuffer());
	if(strKLineCycle == "1分钟")
	{
		m_strKlineCycle = "candle60s";
		m_nKlineCycle = 60;
	}
	else if(strKLineCycle == "3分钟")
	{
		m_strKlineCycle = "candle180s";
		m_nKlineCycle = 180;
	}
	else if(strKLineCycle == "5分钟")
	{
		m_strKlineCycle = "candle300s";
		m_nKlineCycle = 300;
	}
	else if(strKLineCycle == "15分钟")
	{
		m_strKlineCycle = "candle900s";
		m_nKlineCycle = 900;
	}
	else if(strKLineCycle == "30分钟")
	{
		m_strKlineCycle = "candle1800s";
		m_nKlineCycle = 1800;
	}
	else if(strKLineCycle == "1小时")
	{
		m_strKlineCycle = "candle3600s";
		m_nKlineCycle = 3600;
	}
	else if(strKLineCycle == "2小时")
	{
		m_strKlineCycle = "candle7200s";
		m_nKlineCycle = 7200;
	}
	else if(strKLineCycle == "4小时")
	{
		m_strKlineCycle = "candle14400s";
		m_nKlineCycle = 14400;
	}
	else if(strKLineCycle == "6小时")
	{
		m_strKlineCycle = "candle21600s";
		m_nKlineCycle = 21600;
	}
	else if(strKLineCycle == "12小时")
	{
		m_strKlineCycle = "candle43200s";
		m_nKlineCycle = 43200;
	}
	else if(strKLineCycle == "1日")
	{
		m_strKlineCycle = "candle86400s";
		m_nKlineCycle = 86400;
	}
	else if(strKLineCycle == "1周")
	{
		m_strKlineCycle = "candle604800s";
		m_nKlineCycle = 604800;
	}
	m_nTradeTimeOver = atoi(strTradeTimerOver.GetBuffer());
	m_config.set_value("futures", "coinType", m_strCoinType.c_str());
	m_config.set_value("futures", "standardCurrency", m_strStandardCurrency.c_str());
	m_config.set_value("futures", "futuresCycle", m_strFuturesCycle.c_str());
	m_config.set_value("futures", "leverage", m_strLeverage.c_str());
	m_config.set_value("futures", "futuresType", strFuturesType.GetBuffer());
	m_config.set_value("futures", "martingaleStepCnt", strMartingaleStepCnt.GetBuffer());
	m_config.set_value("futures", "martingaleMovePersent", strMartingaleMovePersent.GetBuffer());
	m_config.set_value("futures", "stopProfitFactor", strStopProfitFactor.GetBuffer());
	m_config.set_value("futures", "firstTradeSize", strFirstTradeSize.GetBuffer());
	m_config.set_value("futures", "cycle", strKLineCycle.GetBuffer());
	m_config.set_value("futures", "tradeTimerOver", strTradeTimerOver.GetBuffer());
	if(m_nTrendType == 0)
		m_config.set_value("futures", "trend", "bull");
	else if(m_nTrendType == 1)
		m_config.set_value("futures", "trend", "bear");
	else if(m_nTrendType == 2)
		m_config.set_value("futures", "trend", "auto");

	if(m_bStopProfitMove)
		m_config.set_value("futures", "stopProfitType", "move");
	else
		m_config.set_value("futures", "stopProfitType", "fix");
	m_config.set_value("futures", "beginMoney", szCost.GetBuffer());
	m_config.save("./config.ini");
	return true;
}

void COKExMartingaleDlg::OnBnClickedButtonStopWhenFinish()
{
	m_bStopWhenFinish = true;
	bool bCancelAll = true;
	for(int i = 0; i<(int)m_vectorTradePairs.size(); ++i)
	{
		if(m_vectorTradePairs[i].open.orderID != "" && m_vectorTradePairs[i].open.state != state_open)
			bCancelAll = false;
	}
	if(bCancelAll)
	{
		for(int i = 0; i<(int)m_vectorTradePairs.size(); ++i)
		{
			if(m_vectorTradePairs[i].open.orderID != "")
			{
				std::string strClientOrderID = "0";
				BEGIN_API_CHECK;
				SHttpResponse resInfo;
				OKEX_HTTP->API_FuturesCancelOrder(false, m_bSwapFutures, m_strCoinType, m_strStandardCurrency, m_strFuturesCycle, m_vectorTradePairs[i].open.orderID, &resInfo);
				Json::Value& retObj = resInfo.retObj;
				if(retObj.isObject() && retObj["result"].isBool() && retObj["result"].asBool())
				{
					_QueryOrderInfo(m_vectorTradePairs[i].open.orderID, "功能暂停 删除订单成功", state_cancelled);
					API_OK;
				}
				else
					_checkStr = resInfo.strRet;
				API_CHECK;
				END_API_CHECK;
			}
		}
		m_vectorTradePairs.clear();
		m_bSaveData = true;
	}
}


void COKExMartingaleDlg::_LogicThread()
{
	pExchange = new COkexExchange(m_apiKey, m_secretKey, m_passphrase, true);
	pExchange->SetHttpCallBackMessage(local_http_callbak_message);
	pExchange->SetWebSocketCallBackOpen(local_websocket_callbak_open);
	pExchange->SetWebSocketCallBackClose(local_websocket_callbak_close);
	pExchange->SetWebSocketCallBackFail(local_websocket_callbak_fail);
	pExchange->SetWebSocketCallBackMessage(local_websocket_callbak_message);
	pExchange->Run();

	clib::string log_path = "log/";
	bool bRet = clib::file_util::mkfiledir(log_path.c_str(), true);

	CLocalLogger& _localLogger = CLocalLogger::GetInstance();
	_localLogger.SetBatchMode(true);
	_localLogger.SetLogPath(log_path.c_str());
	_localLogger.Start();
	_localLogger.SetCallBackFunc(LocalLogCallBackFunc);

	CLocalActionLog::GetInstancePt()->set_log_path(log_path.c_str());
	CLocalActionLog::GetInstancePt()->start();

	while(!m_bExit)
	{
		CLocalLogger::GetInstancePt()->SwapFront2Middle();
		_Update15Sec();
		_Update3Sec();
		if(OKEX_CHANGE)
			OKEX_CHANGE->Update();
		time_t tNow = time(NULL);
		if(m_tListenPong && tNow - m_tListenPong > 15)
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
		if(m_tWaitNewSubDepth && tNow - m_tWaitNewSubDepth >= 5)
		{
			OKEX_WEB_SOCKET->API_FuturesEntrustDepth(true, m_bSwapFutures, m_strCoinType, m_strStandardCurrency, m_strFuturesCycle);
			m_tWaitNewSubDepth = 0;
		}
		__CheckTrade();
		if(m_bSaveData)
		{
			_SaveData();
			m_bSaveData = false;
		}
		boost::this_thread::sleep(boost::posix_time::milliseconds(10));
	}

}

void COKExMartingaleDlg::_Update15Sec()
{
	time_t tNow = time(NULL);
	if(m_tLastUpdate15Sec == 0)
		m_tLastUpdate15Sec = tNow;
	if(tNow - m_tLastUpdate15Sec < 15)
		return;
	m_tLastUpdate15Sec = tNow;
	if(OKEX_WEB_SOCKET->Ping())
		m_tListenPong = time(NULL);
}

void COKExMartingaleDlg::_Update3Sec()
{
	time_t tNow = time(NULL);
	if (m_tLastUpdate3Sec == 0)
		m_tLastUpdate3Sec = tNow;
	if (tNow - m_tLastUpdate3Sec < 3)
		return;
	m_tLastUpdate3Sec = tNow;
	if(m_bRun)
		OKEX_HTTP->API_FuturesAccountInfoByCurrency(true, m_bSwapFutures, m_strCoinType, m_strStandardCurrency);
}


void COKExMartingaleDlg::OnBnClickedButtonUpdateCost()
{
	m_beginMoney = 0.0;
	CString szCost;
	m_editCost.GetWindowText(szCost);
	m_beginMoney = stod(szCost.GetBuffer());
	m_config.set_value("futures", "beginMoney", szCost.GetBuffer());
	m_config.save("./config.ini");
}


void COKExMartingaleDlg::_UpdateProfitShow()
{
	if(m_accountInfo.bValid && m_curTickData.bValid)
	{
		double now = stod(m_accountInfo.equity);
		std::string profit = CFuncCommon::Double2String(now - m_beginMoney + DOUBLE_PRECISION, 7);
		{
			char szTemp[128];
			strcpy(szTemp, profit.c_str());
			for(int i = strlen(szTemp) - 1; i >= 0; --i)
			{
				if(szTemp[i] == '0')
					szTemp[i] = '\0';
				else if(szTemp[i] == '.')
				{
					szTemp[i] = '\0';
					break;
				}
				else
					break;
			}
			profit = szTemp;
		}
		m_editProfit.SetWindowText(profit.c_str());
	}
}

void COKExMartingaleDlg::OnDestroy()
{
	CDialog::OnDestroy();

	m_bExit = true;
	m_logicThread.join();
}

void COKExMartingaleDlg::OnBnClickedStopProfitMove()
{
	m_btnStopProfitFix.SetCheck(0);
}


void COKExMartingaleDlg::OnBnClickedRadioStopProfitFix()
{
	m_btnStopProfitMove.SetCheck(0);
}

bool COKExMartingaleDlg::_CheckMoney(std::string& strCurrency)
{
	SHttpResponse resInfo;
	OKEX_HTTP->API_FuturesAccountInfoByCurrency(false, m_bSwapFutures, strCurrency, m_strStandardCurrency, &resInfo);
	if(resInfo.retObj.isObject())
	{
		SFuturesAccountInfo info;
		if(m_bSwapFutures)
		{
			if(resInfo.retObj["info"].isObject() && resInfo.retObj["info"]["equity"].isString())
			{
				info.equity = resInfo.retObj["info"]["equity"].asString();
				info.availBalance = resInfo.retObj["info"]["total_avail_balance"].asString();
			}
		}
		else
		{
			if(resInfo.retObj["equity"].isString())
			{
				info.equity = resInfo.retObj["equity"].asString();
				info.availBalance = resInfo.retObj["total_avail_balance"].asString();
			}
		}
		g_pDlg->UpdateAccountInfo(info);
		return true;
	}
	return false;
}

void COKExMartingaleDlg::_SetTradeState(eTradeState state)
{
	m_eTradeState = state;
	if (state == eTradeState_WaitOpen)
	{
		m_nLastCheckKline = -1;
		m_bSaveData = true;
	}
}


void COKExMartingaleDlg::ClearDepth()
{
	m_mapDepthSell.clear();
	m_mapDepthBuy.clear();
}

void COKExMartingaleDlg::UpdateDepthInfo(bool bBuy, SFuturesDepth& info)
{
	info.dprice = stod(info.price);
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
}

bool B2S(SFuturesDepth i, SFuturesDepth j) { return (i.dprice>=j.dprice); }
bool S2B(SFuturesDepth i, SFuturesDepth j) { return (i.dprice<j.dprice); }

bool COKExMartingaleDlg::CheckDepthInfo(int checkNum, std::string& checkSrc)
{
	std::vector<SFuturesDepth> vecBuy;
	std::map<std::string, SFuturesDepth>::iterator _itB = m_mapDepthBuy.begin();
	std::map<std::string, SFuturesDepth>::iterator _itE = m_mapDepthBuy.end();
	while(_itB != _itE)
	{
		vecBuy.push_back(_itB->second);
		_itB++;
	}
	std::sort(vecBuy.begin(), vecBuy.end(), B2S);
	std::vector<SFuturesDepth> vecSell;
	_itB = m_mapDepthSell.begin();
	_itE = m_mapDepthSell.end();
	while(_itB != _itE)
	{
		vecSell.push_back(_itB->second);
		_itB++;
	}
	std::sort(vecSell.begin(), vecSell.end(), S2B);
	std::vector<SFuturesDepth>::iterator itBB = vecBuy.begin();
	std::vector<SFuturesDepth>::iterator itBE = vecBuy.end();
	std::vector<SFuturesDepth>::iterator itSB = vecSell.begin();
	std::vector<SFuturesDepth>::iterator itSE = vecSell.end();
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
		OKEX_WEB_SOCKET->API_FuturesEntrustDepth(false, m_bSwapFutures, m_strCoinType, m_strStandardCurrency, m_strFuturesCycle);
		m_tWaitNewSubDepth = time(NULL);
		return false;
	}
	return true;
}

void COKExMartingaleDlg::_SaveData()
{
	std::string strFilePath = "./save.txt";
	std::ofstream stream(strFilePath);
	if(!stream.is_open())
		return;
	stream << m_curOpenFinishIndex << "	" << m_eTradeState << "	" << (m_bOpenBull ? "0" : "1") <<  "	" << m_nStopProfitTimes << "	" << m_nFinishTimes << std::endl;
	for(int i=0; i<(int)m_vectorTradePairs.size(); ++i)
	{
		if(m_vectorTradePairs[i].open.orderID != "")
			stream << m_vectorTradePairs[i].open.orderID << "	" << m_vectorTradePairs[i].open.closeSize << "	";
		else
			stream << "0	0	";
		if(m_vectorTradePairs[i].open.bBeginStopProfit)
			stream << "1	";
		else
			stream << "0	";
		stream << m_vectorTradePairs[i].open.stopProfit << "	" << m_vectorTradePairs[i].open.maxPrice << "	" << m_vectorTradePairs[i].open.minPrice << "	";
		if(m_vectorTradePairs[i].close.orderID != "")
			stream << m_vectorTradePairs[i].close.orderID;
		else
			stream << "0";
		stream << std::endl;
	}
	stream.close();
}

void COKExMartingaleDlg::_LoadData()
{
	std::string strFilePath="./save.txt";
	std::ifstream stream(strFilePath);
	if(!stream.is_open())
		return;
	char lineBuffer[4096]={0};
	if(stream.fail())
		return;
	m_vectorTradePairs.clear();
	int index = 0;
	while(!stream.eof())
	{
		stream.getline(lineBuffer, sizeof(lineBuffer));
		if(*lineBuffer == 0 || (lineBuffer[0] == '/' && lineBuffer[1] == '/') || (lineBuffer[0] == '-' && lineBuffer[1] == '-'))
			continue;
		std::stringstream lineStream(lineBuffer, std::ios_base::in);
		if(index == 0)
		{
			int state;
			int openDir;
			lineStream >> m_curOpenFinishIndex >> state >> openDir >> m_nStopProfitTimes >> m_nFinishTimes;
			m_eTradeState = (eTradeState)state;
			m_bOpenBull = (openDir==0);
			if(m_bOpenBull)
				m_staticDingDan.SetWindowText("做多");
			else
				m_staticDingDan.SetWindowText("做空");
			m_staticFinishTimes.SetWindowText(CFuncCommon::ToString(m_nFinishTimes));
			m_staticStopProfitTimes.SetWindowText(CFuncCommon::ToString(m_nStopProfitTimes));
		}
		else
		{
			char szOpenOrderID[128]={0};
			char szOpenCloseSize[128]={0};
			char szCloseOrderID[128]={0};
			double openMin=0.0;
			double openMax=0.0;
			int beginStopProfit=0;
			int stopProfitStep=0;
			lineStream >> szOpenOrderID >> szOpenCloseSize >> beginStopProfit >> stopProfitStep >> openMax >> openMin >> szCloseOrderID;
			if(strcmp(szOpenOrderID, "0") != 0)
			{
				SFuturesTradePairInfo temp;
				m_vectorTradePairs.push_back(temp);
				SFuturesTradePairInfo& info = m_vectorTradePairs[m_vectorTradePairs.size()-1];
				info.open.orderID = szOpenOrderID;
				info.open.closeSize = szOpenCloseSize;
				info.open.bBeginStopProfit = (beginStopProfit==1);
				info.open.stopProfit = stopProfitStep;
				info.open.maxPrice = openMax;
				info.open.minPrice = openMin;
				if(strcmp(szCloseOrderID, "0") != 0)
					info.close.orderID = szCloseOrderID;
				m_tOpenTime = time(NULL);
			}
		}
		index++;
	}
	stream.close();
}

void COKExMartingaleDlg::OnBnClickedTrendBull()
{
	m_btnTrendBear.SetCheck(0);
	m_btnTrendAuto.SetCheck(0);
}


void COKExMartingaleDlg::OnBnClickedTrendBear()
{
	m_btnTrendBull.SetCheck(0);
	m_btnTrendAuto.SetCheck(0);
}


void COKExMartingaleDlg::OnBnClickedTrendAuto()
{
	m_btnTrendBull.SetCheck(0);
	m_btnTrendBear.SetCheck(0);
}


void COKExMartingaleDlg::OnBnClickedButtonUpdateTrend()
{
	m_nTrendType = 0;
	if(m_btnTrendAuto.GetCheck())
		m_nTrendType = 2;
	else if(m_btnTrendBull.GetCheck())
		m_nTrendType = 0;
	else if(m_btnTrendBear.GetCheck())
		m_nTrendType = 1;
	if(m_nTrendType == 0)
		m_config.set_value("futures", "trend", "bull");
	else if(m_nTrendType == 1)
		m_config.set_value("futures", "trend", "bear");
	else if(m_nTrendType == 2)
		m_config.set_value("futures", "trend", "auto");
	m_config.save("./config.ini");
}
