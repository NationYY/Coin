
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

#define KLINE_DATA m_vecKlineData
#define KLINE_DATA_SIZE ((int)m_vecKlineData.size())
#define OKEX_CHANGE ((COkexExchange*)pExchange)
#define OKEX_WEB_SOCKET ((COkexWebsocketAPI*)pExchange->GetWebSocket())
#define OKEX_HTTP ((COkexHttpAPI*)pExchange->GetHttp())
#define OKEX_TRADE_HTTP ((COkexHttpAPI*)pExchange->GetTradeHttp())
#define MAX_API_TRY_TIME 3

#define BEGIN_API_CHECK {\
							int _checkIndex = 0;\
							std::string _checkStr = "";\
							for(; _checkIndex<3; ++_checkIndex)\
							{
#define API_OK	break;

#define API_CHECK		}\
						if(_checkIndex == 3)\
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


// COKExMartingaleDlg 对话框






COKExMartingaleDlg::COKExMartingaleDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(COKExMartingaleDlg::IDD, pParent)
{
	g_pDlg = this;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_nPriceDecimal = 4;
	m_nVolumeDecimal = 3;
	m_bRun = false;
	m_tListenPong = 0;
	m_strCoinType = "BTC";
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
}

void COKExMartingaleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_ctrlListLog);
	DDX_Control(pDX, IDC_COMBO1, m_combCoinType);
	DDX_Control(pDX, IDC_EDIT1, m_editMartingaleStepCnt);
	DDX_Control(pDX, IDC_EDIT2, m_editMartingaleMovePersent);
	DDX_Control(pDX, IDC_EDIT5, m_editStopProfitFactor);
	DDX_Control(pDX, IDC_EDIT6, m_editCoin);
	DDX_Control(pDX, IDC_EDIT7, m_editProfit);
	DDX_Control(pDX, IDC_STATIC_COIN, m_staticCoin);
	DDX_Control(pDX, IDC_STATIC_MONEY, m_staticMoney);
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
}

BEGIN_MESSAGE_MAP(COKExMartingaleDlg, CDialogEx)
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
END_MESSAGE_MAP()


// COKExMartingaleDlg 消息处理程序

BOOL COKExMartingaleDlg::OnInitDialog()
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

	m_listCtrlOpen.InsertColumn(0, "价格", LVCFMT_CENTER, 90);
	m_listCtrlOpen.InsertColumn(1, "成交量", LVCFMT_CENTER, 100);
	m_listCtrlOpen.InsertColumn(2, "售出量", LVCFMT_CENTER, 60);
	m_listCtrlOpen.InsertColumn(3, "状态", LVCFMT_CENTER, 75);
	m_listCtrlOpen.InsertColumn(4, "参考利润", LVCFMT_CENTER, 100);
	m_listCtrlOpen.InsertColumn(5, "最低价", LVCFMT_CENTER, 90);
	m_listCtrlOpen.InsertColumn(6, "最高价", LVCFMT_CENTER, 90);
	m_listCtrlOpen.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

	m_listCtrlClose.InsertColumn(0, "价格", LVCFMT_CENTER, 70);
	m_listCtrlClose.InsertColumn(1, "成交量", LVCFMT_CENTER, 100);
	m_listCtrlClose.InsertColumn(2, "状态", LVCFMT_CENTER, 70);
	m_listCtrlClose.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

	if(!m_config.open("./config.ini"))
		return FALSE;
	m_apiKey = m_config.get("spot", "apiKey", "");
	m_secretKey = m_config.get("spot", "secretKey", "");
	m_passphrase = m_config.get("spot", "passphrase", "");
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
		CDialogEx::OnSysCommand(nID, lParam);
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
		CDialogEx::OnPaint();
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
		instrumentID = m_strCoinType + "-USD-SWAP";
	else
		instrumentID = m_strCoinType + "-USD-" + m_strFuturesCycle;
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
		OKEX_HTTP->API_FuturesSetLeverage(false, m_bSwapFutures, m_strCoinType, strLeverage, &resInfo);
		if (m_bSwapFutures)
		{
			std::string strInstrumentID = m_strCoinType + "-USD-SWAP";
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
	if(OKEX_WEB_SOCKET)
	{
		OKEX_WEB_SOCKET->API_FuturesTickerData(true, m_bSwapFutures, m_strCoinType, m_strFuturesCycle);
		OKEX_WEB_SOCKET->API_LoginFutures(m_apiKey, m_secretKey, time(NULL));
		OKEX_WEB_SOCKET->API_FuturesEntrustDepth(true, m_bSwapFutures, m_strCoinType, m_strFuturesCycle);
		OKEX_WEB_SOCKET->API_FuturesKlineData(true, m_bSwapFutures, m_strKlineCycle, m_strCoinType, m_strFuturesCycle);
	}
	CString strTitle;
	strTitle.Format("%s-%s", (m_bSwapFutures ? "永续合约" : "交割合约"), m_strCoinType.c_str());
	CWnd *m_pMainWnd;
	m_pMainWnd = AfxGetMainWnd();
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
			CActionLog("kline", "差距%d秒", data.time - KLINE_DATA[KLINE_DATA_SIZE - 1].time);
			ComplementedKLine(data.time, int(data.time - KLINE_DATA[KLINE_DATA_SIZE-1].time - m_nKlineCycle)/m_nKlineCycle);
		}
	}
	tm _tm;
	localtime_s(&_tm, &data.time);
	_snprintf(data.szTime, 20, "%d-%02d-%02d %02d:%02d:%02d", _tm.tm_year + 1900, _tm.tm_mon + 1, _tm.tm_mday, _tm.tm_hour, _tm.tm_min, _tm.tm_sec);
	KLINE_DATA.push_back(data);
}

void COKExMartingaleDlg::ComplementedKLine(time_t tNowKlineTick, int kLineCnt)
{
	time_t endTick = tNowKlineTick - m_nKlineCycle;
	time_t beginTick = endTick - (kLineCnt - 1)*m_nKlineCycle;
	std::string strFrom = CFuncCommon::LocaltimeToISO8601(beginTick);
	std::string strTo = CFuncCommon::LocaltimeToISO8601(endTick);
	std::string strKlineCycle = CFuncCommon::ToString(m_nKlineCycle);
	SHttpResponse resInfo;
	OKEX_HTTP->API_GetFuturesSomeKline(false, m_bSwapFutures, m_strCoinType, m_strFuturesCycle, strKlineCycle, strFrom, strTo, &resInfo);
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
					CFuncCommon::Double2String(data.openPrice + DOUBLE_PRECISION, m_nPriceDecimal).c_str(),
					CFuncCommon::Double2String(data.highPrice + DOUBLE_PRECISION, m_nPriceDecimal).c_str(),
					CFuncCommon::Double2String(data.lowPrice + DOUBLE_PRECISION, m_nPriceDecimal).c_str(),
					CFuncCommon::Double2String(data.closePrice + DOUBLE_PRECISION, m_nPriceDecimal).c_str(),
					data.volume,
					CFuncCommon::Double2String(data.volumeByCurrency + DOUBLE_PRECISION, m_nPriceDecimal).c_str(),
					m_strCoinType.c_str(),
					m_strFuturesCycle.c_str());
			}
			CActionLog("market", "%s", strlocalLog.GetBuffer());
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
				m_vectorTradePairs[i].open.minPrice = m_curTickData.last;
			if(m_curTickData.last > m_vectorTradePairs[i].open.maxPrice)
				m_vectorTradePairs[i].open.maxPrice = m_curTickData.last;
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


void COKExMartingaleDlg::OnLoginSuccess()
{
	if(m_vectorTradePairs.size())
	{
		for(int i=0; i<(int)m_vectorTradePairs.size(); ++i)
		{
			if(m_vectorTradePairs[i].open.orderID != "" && m_vectorTradePairs[i].open.state != state_filled && m_vectorTradePairs[i].open.state != state_cancelled)
			{
				BEGIN_API_CHECK;
				SHttpResponse _resInfo;
				OKEX_HTTP->API_FuturesOrderInfo(false, m_bSwapFutures, m_strCoinType, m_strFuturesCycle, m_vectorTradePairs[i].open..orderID, &resInfo);
				Json::Value& _retObj = _resInfo.retObj;
				if(_retObj.isObject() && _retObj["order_id"].isString())
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
					if (tradeType == "1")
						info.tradeType = eFuturesTradeType_OpenBull;
					else if (tradeType == "2")
						info.tradeType = eFuturesTradeType_OpenBear;
					else if (tradeType == "3")
						info.tradeType = eFuturesTradeType_CloseBull;
					else if (tradeType == "4")
						info.tradeType = eFuturesTradeType_CloseBear;
					else
						LOCAL_ERROR("未知交易类型[%s]", tradeType.c_str());
					info.size = CFuncCommon::ToString(stoi(_retObj["size"].asString()));
					g_pDlg->UpdateTradeInfo(info);
					CActionLog("trade", "[订单手动更新] order=%s, size=%s, filledQTY=%s, price=%s, state=%s, type=%d", info.orderID.c_str(), info.size.c_str(), info.filledQTY.c_str(), info.price.c_str(), info.state.c_str(), info.tradeType);
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
			if(m_vectorTradePairs[i].close.orderID != "" && m_vectorTradePairs[i].close.status != state_filled && m_vectorTradePairs[i].close.state != state_cancelled)
			{
				BEGIN_API_CHECK;
				SHttpResponse _resInfo;
				OKEX_HTTP->API_FuturesOrderInfo(false, m_bSwapFutures, m_strCoinType, m_strFuturesCycle, m_vectorTradePairs[i].close..orderID, &resInfo);
				Json::Value& _retObj = _resInfo.retObj;
				if(_retObj.isObject() && _retObj["order_id"].isString())
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
					if (tradeType == "1")
						info.tradeType = eFuturesTradeType_OpenBull;
					else if (tradeType == "2")
						info.tradeType = eFuturesTradeType_OpenBear;
					else if (tradeType == "3")
						info.tradeType = eFuturesTradeType_CloseBull;
					else if (tradeType == "4")
						info.tradeType = eFuturesTradeType_CloseBear;
					else
						LOCAL_ERROR("未知交易类型[%s]", tradeType.c_str());
					info.size = CFuncCommon::ToString(stoi(_retObj["size"].asString()));
					g_pDlg->UpdateTradeInfo(info);
					CActionLog("trade", "[订单手动更新] order=%s, size=%s, filledQTY=%s, price=%s, state=%s, type=%d", info.orderID.c_str(), info.size.c_str(), info.filledQTY.c_str(), info.price.c_str(), info.state.c_str(), info.tradeType);
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
		}
		
	}
	OKEX_WEB_SOCKET->API_FuturesOrderInfo(true, m_bSwapFutures, m_strCoinType, m_strFuturesCycle);
}

void COKExMartingaleDlg::UpdateAccountInfo(SFuturesAccountInfo& info)
{
	m_accountInfo = info;
	m_accountInfo.bValid = true;
	m_editCoin.SetWindowText(m_accountInfo.equity.c_str());
	std::string strCoin = m_strCoinType;
	strCoin += ":";
	m_staticCoin.SetWindowText(strCoin.c_str());
	_UpdateProfitShow();
}

void CManualOKExFuturesDlg::UpdatePositionInfo(SFuturesPositionInfo& info)
{
	m_positionInfo = info;
	m_positionInfo.bValid = true;
}

void COKExMartingaleDlg::__CheckTrade()
{
	if(!m_bRun)
		return;
	switch(m_eTradeState)
	{
	case eTradeState_WaitOpen:
		{
			if(m_bStopWhenFinish)
				break;
			if(!m_curTickData.bValid)
				break;
			if(!m_accountInfo.bValid)
				break;
			if(m_accountInfo.availBalance == "0")
				break;
			m_curOpenFinishIndex = -1;
			//开始下单
			m_vectorTradePairs.clear();
			for(int i = 0; i<m_martingaleStepCnt; ++i)
			{
				int multi = pow(2, i);
				double price = m_curTickData.buy*(1 - m_martingaleMovePersent*multi);
				std::string strPrice = CFuncCommon::Double2String(price+DOUBLE_PRECISION, m_nPriceDecimal);
				int size = m_nFirstTradeSize*(i+1);
				std::string strSize = CFuncCommon::ToString(size);

				BEGIN_API_CHECK;
				SHttpResponse resInfo;
				std::string clientOrderID = CFuncCommon::GenUUID();
				OKEX_HTTP->API_FuturesTrade(false, m_bSwapFutures, eFuturesTradeType_OpenBear, m_strCoinType, m_strFuturesCycle, strPrice, strSize, m_strLeverage, strClientOrderID, &resInfo);
				Json::Value& retObj = resInfo.retObj;
				if(retObj.isObject() && retObj["result"].isBool() && retObj["result"].asBool())
				{
					std::string strOrderID = retObj["order_id"].asString();
					BEGIN_API_CHECK;
					SHttpResponse _resInfo;
					OKEX_HTTP->API_FuturesOrderInfo(false, m_bSwapFutures, m_strCoinType, m_strFuturesCycle, strOrderID, &_resInfo);
					Json::Value& _retObj = _resInfo.retObj;
					if(_retObj.isObject()&&_retObj["order_id"].isString())
					{
						SFuturesTradePairInfo pairs;
						pairs.open.orderID = strOrderID;
						pairs.open.strClientOrderID = clientOrderID;
						m_vectorTradePairs.push_back(pairs);

						SFuturesTradeInfo info;
						info.strClientOrderID = "";
						info.timeStamp = CFuncCommon::ISO8601ToTime(_retObj["timestamp"].asString());
						info.filledQTY = CFuncCommon::ToString(stoi(_retObj["filled_qty"].asString()));
						info.orderID = _retObj["order_id"].asString();
						info.price = stod(_retObj["price"].asString());
						info.priceAvg = stod(_retObj["price_avg"].asString());
						info.status = _retObj["status"].asString();
						info.size = CFuncCommon::ToString(stoi(_retObj["size"].asString()));
						std::string tradeType = _retObj["type"].asString();
						if(tradeType == "1")
							info.tradeType = eFuturesTradeType_OpenBull;
						else if(tradeType == "2")
							info.tradeType = eFuturesTradeType_OpenBear;
						else if(tradeType == "3")
							info.tradeType = eFuturesTradeType_CloseBull;
						else if(tradeType == "4")
							info.tradeType = eFuturesTradeType_CloseBear;
						g_pDlg->UpdateTradeInfo(info);
						CActionLog("trade", "[新批次 开买单] order=%s, size=%s, filled_size=%s, price=%s, status=%s, type=%d", info.orderID.c_str(), info.size.c_str(), info.filledQTY.c_str(), info.price.c_str(), info.state.c_str(), info.tradeType);
						API_OK;
					}
					else
					{
						_checkStr = _resInfo.strRet;
						boost::this_thread::sleep(boost::posix_time::seconds(1));
					}
					API_CHECK;
					END_API_CHECK;
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
					m_vectorTradePairs[0].open.bBeginStopProfit = true;
				else
				{
					m_vectorTradePairs[0].open.bBeginStopProfit = false;
					//先把当前挂的卖单撤销了
					for(int i=0; i<m_curOpenFinishIndex; ++i)
					{
						if(m_vectorTradePairs[i].close.orderID != "")
						{
							if(m_vectorTradePairs[i].close.status != state_filled && m_vectorTradePairs[i].close.status != state_cancelled)
							{
								int closeFinish = atoi(m_vectorTradePairs[i].close.filledQTY.c_str());
								if(!CFuncCommon::CheckEqual(closeFinish, 0.0))
								{
									int openFinish = atoi(m_vectorTradePairs[i].open.filledQTY.c_str());
									openFinish -= closeFinish;
									if(m_vectorTradePairs[i].open.closeSize == "0")
										m_vectorTradePairs[i].open.closeSize = m_vectorTradePairs[i].close.filledQTY;
									else
									{
										int oldClose = atoi(m_vectorTradePairs[i].open.closeSize);
										oldClose += closeFinish;
										m_vectorTradePairs[i].open.closeSize = CFuncCommon::ToString(oldClose);
									}
									m_vectorTradePairs[i].open.filledSize = CFuncCommon::ToString(openFinish);
								}
								BEGIN_API_CHECK;
								SHttpResponse resInfo;
								OKEX_HTTP->API_SpotCancelOrder(false, m_strInstrumentID, m_vectorTradePairs[i].close.orderID, &resInfo);
								Json::Value& retObj = resInfo.retObj;
								if (retObj.isObject() && retObj["result"].isBool() && retObj["result"].asBool())
								{
									BEGIN_API_CHECK;
									SHttpResponse _resInfo;
									OKEX_HTTP->API_SpotOrderInfo(false, m_strInstrumentID, m_vectorTradePairs[i].close.orderID, &_resInfo);
									Json::Value& _retObj = _resInfo.retObj;
									if (_retObj.isObject() && _retObj["order_id"].isString())
									{
										SSPotTradeInfo info;
										info.orderID = _retObj["order_id"].asString();
										info.price = _retObj["price"].asString();
										info.size = _retObj["size"].asString();
										info.side = _retObj["side"].asString();
										info.strTimeStamp = _retObj["timestamp"].asString();
										info.timeStamp = CFuncCommon::ISO8601ToTime(info.strTimeStamp);
										info.filledSize = _retObj["filled_size"].asString();
										info.filledNotional = _retObj["filled_notional"].asString();
										info.status = _retObj["status"].asString();
										g_pDlg->UpdateTradeInfo(info);
										if (info.status == "cancelled")
										{
											CActionLog("trade", "[新批次成交 老订单撤销成功] order=%s", m_vectorTradePairs[i].close.orderID.c_str());
											API_OK;
										}
										else
											boost::this_thread::sleep(boost::posix_time::seconds(1));
									}
									else
									{
										_checkStr = _resInfo.strRet;
										boost::this_thread::sleep(boost::posix_time::seconds(1));
									}
									API_CHECK;
									END_API_CHECK;

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
					double scale = m_curOpenFinishIndex/double(m_martingaleStepCnt-1);
					if(scale > 0.5)
						scale = 0.5;
					finishPrice *= (1+m_martingaleMovePersent+(m_martingaleMovePersent*scale));
					std::string strPrice = CFuncCommon::Double2String(finishPrice+DOUBLE_PRECISION, m_nPriceDecimal);
					for(int i = 0; i <= m_curOpenFinishIndex; ++i)
					{
						BEGIN_API_CHECK;
						SHttpResponse resInfo;
						strTradeType = "sell";
						clientOrderID = CFuncCommon::GenUUID();
						OKEX_HTTP->API_SpotTrade(false, m_strInstrumentID, strTradeType, strPrice, m_vectorTradePairs[i].open.filledSize, clientOrderID, &resInfo);
						Json::Value& retObj = resInfo.retObj;
						if (retObj.isObject() && retObj["result"].isBool() && retObj["result"].asBool())
						{
							std::string strOrderID = retObj["order_id"].asString();
							BEGIN_API_CHECK;
							SHttpResponse _resInfo;
							OKEX_HTTP->API_SpotOrderInfo(false, m_strInstrumentID, strOrderID, &_resInfo);
							Json::Value& _retObj = _resInfo.retObj;
							if (_retObj.isObject() && _retObj["order_id"].isString())
							{
								m_vectorTradePairs[i].close.orderID = strOrderID;
								SSPotTradeInfo info;
								info.orderID = _retObj["order_id"].asString();
								info.price = _retObj["price"].asString();
								info.size = _retObj["size"].asString();
								info.side = _retObj["side"].asString();
								info.strTimeStamp = _retObj["timestamp"].asString();
								info.timeStamp = CFuncCommon::ISO8601ToTime(info.strTimeStamp);
								info.filledSize = _retObj["filled_size"].asString();
								info.filledNotional = _retObj["filled_notional"].asString();
								info.status = _retObj["status"].asString();
								g_pDlg->UpdateTradeInfo(info);
								CActionLog("trade", "[新批次成交 开卖单] order=%s, size=%s, filled_size=%s, price=%s, status=%s, side=%s", info.orderID.c_str(), info.size.c_str(), info.filledSize.c_str(), info.price.c_str(), info.status.c_str(), info.side.c_str());
								API_OK;
							}
							else
							{
								_checkStr = _resInfo.strRet;
								boost::this_thread::sleep(boost::posix_time::seconds(1));
							}
							API_CHECK;
							END_API_CHECK;

							API_OK;
						}
						else
							_checkStr = resInfo.strRet;
						API_CHECK;
						END_API_CHECK;
					}
				}
			}
			if(m_vectorTradePairs.size())
			{
				//长时间未成交就撤销订单
				if(m_vectorTradePairs[0].open.status == "open" || m_vectorTradePairs[0].open.status == "part_filled")
				{
					time_t tNow = time(NULL);
					if(tNow - m_tOpenTime > 10*60)
					{
						bool bCancelAll = false;
						if(m_vectorTradePairs[0].open.status == "open")
							bCancelAll = true;
						for(int i = 0; i<(int)m_vectorTradePairs.size(); ++i)
						{
							if(m_vectorTradePairs[i].open.orderID != "")
							{
								std::string strClientOrderID = "0";
								BEGIN_API_CHECK;
								SHttpResponse resInfo;
								OKEX_HTTP->API_SpotCancelOrder(false, m_strInstrumentID, m_vectorTradePairs[i].open.orderID, &resInfo);
								Json::Value& retObj = resInfo.retObj;
								if (retObj.isObject() && retObj["result"].isBool() && retObj["result"].asBool())
								{
									BEGIN_API_CHECK;
									SHttpResponse _resInfo;
									OKEX_HTTP->API_SpotOrderInfo(false, m_strInstrumentID, m_vectorTradePairs[i].open.orderID, &_resInfo);
									Json::Value& _retObj = _resInfo.retObj;
									if (_retObj.isObject() && _retObj["order_id"].isString())
									{
										SSPotTradeInfo info;
										info.orderID = _retObj["order_id"].asString();
										info.price = _retObj["price"].asString();
										info.size = _retObj["size"].asString();
										info.side = _retObj["side"].asString();
										info.strTimeStamp = _retObj["timestamp"].asString();
										info.timeStamp = CFuncCommon::ISO8601ToTime(info.strTimeStamp);
										info.filledSize = _retObj["filled_size"].asString();
										info.filledNotional = _retObj["filled_notional"].asString();
										info.status = _retObj["status"].asString();
										g_pDlg->UpdateTradeInfo(info);
										if (info.status == "cancelled")
										{
											CActionLog("trade", "[超时未成交 撤销订单成功] order=%s", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), m_vectorTradePairs[i].open.orderID.c_str());
											API_OK;
										}
										else
											boost::this_thread::sleep(boost::posix_time::seconds(1));
									}
									else
									{
										_checkStr = _resInfo.strRet;
										boost::this_thread::sleep(boost::posix_time::seconds(1));
									}
									API_CHECK;
									END_API_CHECK;
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
							m_vectorTradePairs[0].open.bBeginStopProfit = true;
					}
				}
				else
				{
					//检测完成状态,包括所有单子都成交,或剩最后一单未成交或成交中
					bool bAllFinish = true;
					for(int i = 0; i <= m_curOpenFinishIndex; ++i)
					{
						if(m_vectorTradePairs[i].open.orderID != "" && m_vectorTradePairs[i].open.status != "filled" && m_vectorTradePairs[i].open.status != "cancelled")
						{
							bAllFinish = false;
							break;
						}
						if(m_vectorTradePairs[i].close.orderID != "" && m_vectorTradePairs[i].close.status != "filled" && m_vectorTradePairs[i].close.status != "cancelled")
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
						}
						else
						{
							//只有前一单的卖单成交完了才判断下一单的情况
							if(m_vectorTradePairs[m_curOpenFinishIndex].close.orderID != "" && m_vectorTradePairs[m_curOpenFinishIndex].close.status == "filled")
							{
								SSPotTradePairInfo& pairsInfo = m_vectorTradePairs[m_curOpenFinishIndex + 1];
								//下一单未成交
								if(pairsInfo.open.status == "open")
								{
									BEGIN_API_CHECK;
									SHttpResponse resInfo;
									OKEX_HTTP->API_SpotCancelOrder(false, m_strInstrumentID, pairsInfo.open.orderID, &resInfo);
									Json::Value& retObj = resInfo.retObj;
									if (retObj.isObject() && retObj["result"].isBool() && retObj["result"].asBool())
									{
										CActionLog("trade", "[最后一单 撤销订单成功] order=%s", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), pairsInfo.open.orderID.c_str());
										API_OK;
									}
									else
										_checkStr = resInfo.strRet;
									API_CHECK;
									END_API_CHECK;
									_SetTradeState(eTradeState_WaitOpen);
									CActionLog("finish_trade", "[成功交易] 单批次");
									CActionLog("trade", "[成功交易] 单批次");
									LOCAL_INFO("成功交易一个批次");
									m_nFinishTimes++;
									m_staticFinishTimes.SetWindowText(CFuncCommon::ToString(m_nFinishTimes));
								}
								else if(pairsInfo.open.status == "cancelled" || pairsInfo.open.status == "part_filled")
								{
									//如果未成交先撤单
									if(pairsInfo.open.status == "part_filled")
									{
										BEGIN_API_CHECK;
										SHttpResponse resInfo;
										OKEX_HTTP->API_SpotCancelOrder(false, m_strInstrumentID, pairsInfo.open.orderID, &resInfo);
										Json::Value& retObj = resInfo.retObj;
										if (retObj.isObject() && retObj["result"].isBool() && retObj["result"].asBool())
										{
											BEGIN_API_CHECK;
											SHttpResponse _resInfo;
											OKEX_HTTP->API_SpotOrderInfo(false, m_strInstrumentID, pairsInfo.open.orderID, &_resInfo);
											Json::Value& _retObj = _resInfo.retObj;
											if (_retObj.isObject() && _retObj["order_id"].isString())
											{
												SSPotTradeInfo info;
												info.orderID = _retObj["order_id"].asString();
												info.price = _retObj["price"].asString();
												info.size = _retObj["size"].asString();
												info.side = _retObj["side"].asString();
												info.strTimeStamp = _retObj["timestamp"].asString();
												info.timeStamp = CFuncCommon::ISO8601ToTime(info.strTimeStamp);
												info.filledSize = _retObj["filled_size"].asString();
												info.filledNotional = _retObj["filled_notional"].asString();
												info.status = _retObj["status"].asString();
												g_pDlg->UpdateTradeInfo(info);
												if (info.status == "cancelled")
												{
													CActionLog("trade", "[最后一单 撤销订单成功] order=%s", pairsInfo.open.orderID.c_str());
													API_OK;
												}
												else
													boost::this_thread::sleep(boost::posix_time::seconds(1));
											}
											else
											{
												_checkStr = _resInfo.strRet;
												boost::this_thread::sleep(boost::posix_time::seconds(1));
											}
											API_CHECK;
											END_API_CHECK;
											API_OK;
										}
										else
											_checkStr = resInfo.strRet;
										API_CHECK;
										END_API_CHECK;
										//直接用卖一价挂卖单
										std::string strPrice = CFuncCommon::Double2String(m_curTickData.sell + DOUBLE_PRECISION, m_nPriceDecimal);
										BEGIN_API_CHECK;
										SHttpResponse resInfo;
										strTradeType = "sell";
										clientOrderID = CFuncCommon::GenUUID();
										OKEX_HTTP->API_SpotTrade(false, m_strInstrumentID, strTradeType, strPrice, pairsInfo.open.filledSize, clientOrderID, &resInfo);
										Json::Value& retObj = resInfo.retObj;
										if (retObj.isObject() && retObj["result"].isBool() && retObj["result"].asBool())
										{
											std::string strOrderID = retObj["order_id"].asString();
											BEGIN_API_CHECK;
											SHttpResponse _resInfo;
											OKEX_HTTP->API_SpotOrderInfo(false, m_strInstrumentID, strOrderID, &_resInfo);
											Json::Value& _retObj = _resInfo.retObj;
											if (_retObj.isObject() && _retObj["order_id"].isString())
											{
												pairsInfo.close.orderID = strOrderID;
												SSPotTradeInfo info;
												info.orderID = _retObj["order_id"].asString();
												info.price = _retObj["price"].asString();
												info.size = _retObj["size"].asString();
												info.side = _retObj["side"].asString();
												info.strTimeStamp = _retObj["timestamp"].asString();
												info.timeStamp = CFuncCommon::ISO8601ToTime(info.strTimeStamp);
												info.filledSize = _retObj["filled_size"].asString();
												info.filledNotional = _retObj["filled_notional"].asString();
												info.status = _retObj["status"].asString();
												g_pDlg->UpdateTradeInfo(info);
												CActionLog("trade", "[最后一单 开卖单] order=%s, size=%s, filled_size=%s, price=%s, status=%s, side=%s", info.orderID.c_str(), info.size.c_str(), info.filledSize.c_str(), info.price.c_str(), info.status.c_str(), info.side.c_str());
												API_OK;
											}
											else
											{
												_checkStr = _resInfo.strRet;
												boost::this_thread::sleep(boost::posix_time::seconds(1));
											}
											API_CHECK;
											END_API_CHECK;

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
										}
										else
										{
											if(pairsInfo.close.status == "cancelled" || pairsInfo.close.status == "filled")
											{
												_SetTradeState(eTradeState_WaitOpen);
												CActionLog("finish_trade", "[成功交易] 单批次");
												CActionLog("trade", "[成功交易] 单批次");
												LOCAL_INFO("成功交易一个批次");
												m_nFinishTimes++;
												m_staticFinishTimes.SetWindowText(CFuncCommon::ToString(m_nFinishTimes));
											}
										}
									}
								}
							}
						}
					}
					else
					{
						SSPotTradePairInfo& pairsInfo = m_vectorTradePairs[0];
						if(pairsInfo.open.bBeginStopProfit)
						{
							if(pairsInfo.close.orderID == "")
							{
								double openPrice = stod(pairsInfo.open.price);
								if(m_bStopProfitMove)
								{
									if(m_curTickData.last > openPrice)
									{
										double up = (m_curTickData.last - openPrice) / openPrice;
										int nowStep = int(up / m_stopProfitFactor);
										if(nowStep - pairsInfo.open.stopProfit >= 2)
											pairsInfo.open.stopProfit = nowStep - 1;
									}
									if(pairsInfo.open.stopProfit)
									{
										if(m_curTickData.last <= (openPrice*(1+pairsInfo.open.stopProfit*m_stopProfitFactor+m_stopProfitFactor/2)))
										{
											//把所有成交的open单子市价处理,其余撤单
											std::string strPrice = CFuncCommon::Double2String(m_curTickData.sell+DOUBLE_PRECISION, m_nPriceDecimal);
											for(int i = 0; i<(int)m_vectorTradePairs.size(); ++i)
											{
												if(m_vectorTradePairs[i].open.orderID != "")
												{
													//先市价处理
													double size = stod(m_vectorTradePairs[i].open.filledSize);
													if(m_vectorTradePairs[i].open.status == "part_filled" || m_vectorTradePairs[i].open.status == "filled" || (m_vectorTradePairs[i].open.status == "cancelled" && size > 0))
													{
														BEGIN_API_CHECK;
														SHttpResponse resInfo;
														strTradeType = "sell";
														clientOrderID = CFuncCommon::GenUUID();
														OKEX_HTTP->API_SpotTrade(false, m_strInstrumentID, strTradeType, strPrice, m_vectorTradePairs[i].open.filledSize, clientOrderID, &resInfo);
														Json::Value& retObj = resInfo.retObj;
														if (retObj.isObject() && retObj["result"].isBool() && retObj["result"].asBool())
														{
															std::string strOrderID = retObj["order_id"].asString();
															BEGIN_API_CHECK;
															SHttpResponse _resInfo;
															OKEX_HTTP->API_SpotOrderInfo(false, m_strInstrumentID, strOrderID, &_resInfo);
															Json::Value& _retObj = _resInfo.retObj;
															if (_retObj.isObject() && _retObj["order_id"].isString())
															{
																m_vectorTradePairs[i].close.orderID = strOrderID;
																SSPotTradeInfo info;
																info.orderID = _retObj["order_id"].asString();
																info.price = _retObj["price"].asString();
																info.size = _retObj["size"].asString();
																info.side = _retObj["side"].asString();
																info.strTimeStamp = _retObj["timestamp"].asString();
																info.timeStamp = CFuncCommon::ISO8601ToTime(info.strTimeStamp);
																info.filledSize = _retObj["filled_size"].asString();
																info.filledNotional = _retObj["filled_notional"].asString();
																info.status = _retObj["status"].asString();
																g_pDlg->UpdateTradeInfo(info);
																CActionLog("trade", "[止盈 开卖单] order=%s, size=%s, filled_size=%s, price=%s, status=%s, side=%s", info.orderID.c_str(), info.size.c_str(), info.filledSize.c_str(), info.price.c_str(), info.status.c_str(), info.side.c_str());
																API_OK;
															}
															else
															{
																_checkStr = _resInfo.strRet;
																boost::this_thread::sleep(boost::posix_time::seconds(1));
															}
															API_CHECK;
															END_API_CHECK;

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
									std::string strPrice = CFuncCommon::Double2String(openPrice*(1+m_stopProfitFactor)+DOUBLE_PRECISION, m_nPriceDecimal);
									BEGIN_API_CHECK;
									SHttpResponse resInfo;
									strTradeType = "sell";
									clientOrderID = CFuncCommon::GenUUID();
									OKEX_HTTP->API_SpotTrade(false, m_strInstrumentID, strTradeType, strPrice, pairsInfo.open.filledSize, clientOrderID, &resInfo);
									Json::Value& retObj = resInfo.retObj;
									if (retObj.isObject() && retObj["result"].isBool() && retObj["result"].asBool())
									{
										std::string strOrderID = retObj["order_id"].asString();
										BEGIN_API_CHECK;
										SHttpResponse _resInfo;
										OKEX_HTTP->API_SpotOrderInfo(false, m_strInstrumentID, strOrderID, &_resInfo);
										Json::Value& _retObj = _resInfo.retObj;
										if (_retObj.isObject() && _retObj["order_id"].isString())
										{
											pairsInfo.close.orderID = strOrderID;
											SSPotTradeInfo info;
											info.orderID = _retObj["order_id"].asString();
											info.price = _retObj["price"].asString();
											info.size = _retObj["size"].asString();
											info.side = _retObj["side"].asString();
											info.strTimeStamp = _retObj["timestamp"].asString();
											info.timeStamp = CFuncCommon::ISO8601ToTime(info.strTimeStamp);
											info.filledSize = _retObj["filled_size"].asString();
											info.filledNotional = _retObj["filled_notional"].asString();
											info.status = _retObj["status"].asString();
											g_pDlg->UpdateTradeInfo(info);
											CActionLog("trade", "[止盈 开卖单] order=%s, size=%s, filled_size=%s, price=%s, status=%s, side=%s", info.orderID.c_str(), info.size.c_str(), info.filledSize.c_str(), info.price.c_str(), info.status.c_str(), info.side.c_str());
											API_OK;
										}
										else
										{
											_checkStr = _resInfo.strRet;
											boost::this_thread::sleep(boost::posix_time::seconds(1));
										}
										API_CHECK;
										END_API_CHECK;

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
						if(m_vectorTradePairs[0].close.orderID != "" && m_vectorTradePairs[0].close.status == "filled")
						{
							bool bAllEmpty = true;
							for(int i = 1; i < (int)m_vectorTradePairs.size(); i++)
							{
								if(m_vectorTradePairs[i].close.orderID != "" && m_vectorTradePairs[i].close.status != "filled")
								{
									bAllEmpty = false;
									break;
								}
							}
							if(bAllEmpty)
							{
								m_nStopProfitTimes++;
								m_staticStopProfitTimes.SetWindowText(CFuncCommon::ToString(m_nStopProfitTimes));
								CActionLog("finish_trade", "[止盈成功] 单批次");
								CActionLog("trade", "[止盈成功] 单批次");
								LOCAL_INFO("成功止盈一个批次");
								//把所有未成交的单子撤了
								for(int i = 0; i < (int)m_vectorTradePairs.size(); ++i)
								{
									if(m_vectorTradePairs[i].open.orderID != "")
									{
										if(m_vectorTradePairs[i].open.status != "filled")
										{
											BEGIN_API_CHECK;
											SHttpResponse resInfo;
											OKEX_HTTP->API_SpotCancelOrder(false, m_strInstrumentID, m_vectorTradePairs[i].open.orderID, &resInfo);
											Json::Value& retObj = resInfo.retObj;
											if (retObj.isObject() && retObj["result"].isBool() && retObj["result"].asBool())
											{
												CActionLog("trade", "[止盈 撤销订单成功] order=%s", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), pairsInfo.open.orderID.c_str());
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
										if(m_vectorTradePairs[i].close.status != "filled")
										{
											BEGIN_API_CHECK;
											SHttpResponse resInfo;
											OKEX_HTTP->API_SpotCancelOrder(false, m_strInstrumentID, m_vectorTradePairs[i].close.orderID, &resInfo);
											Json::Value& retObj = resInfo.retObj;
											if (retObj.isObject() && retObj["result"].isBool() && retObj["result"].asBool())
											{
												CActionLog("trade", "[止盈 撤销订单成功] order=%s", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), pairsInfo.open.orderID.c_str());
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

void COKExMartingaleDlg::UpdateTradeInfo(SFuturesTradeInfo& info)
{
	for(int i = 0; i<(int)m_vectorTradePairs.size(); i++)
	{
		if(m_vectorTradePairs[i].open.orderID == info.orderID && 
		   m_vectorTradePairs[i].open.state != state_filled)
		{
			if((m_vectorTradePairs[i].open.state==state_open || m_vectorTradePairs[i].open.state=="") && info.status!=state_open && m_curTickData.bValid)
			{
				m_vectorTradePairs[i].open.minPrice = m_curTickData.last;
				m_vectorTradePairs[i].open.maxPrice = m_curTickData.last;
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
			szFormat.Format("%s", m_vectorTradePairs[i].open.price.c_str());
			m_listCtrlOpen.SetItemText(i, 0, szFormat);
			szFormat.Format("%s/%s", m_vectorTradePairs[i].open.filledSize.c_str(), m_vectorTradePairs[i].open.size.c_str());
			m_listCtrlOpen.SetItemText(i, 1, szFormat);
			szFormat.Format("%s", m_vectorTradePairs[i].open.closeSize.c_str());
			m_listCtrlOpen.SetItemText(i, 2, szFormat);
			szFormat.Format("%s", m_vectorTradePairs[i].open.status.c_str());
			m_listCtrlOpen.SetItemText(i, 3, szFormat);
			if(m_curTickData.bValid)
			{
				double filledSize = stod(m_vectorTradePairs[i].open.filledSize);
				if(filledSize > 0)
				{
					double openPrice = stod(m_vectorTradePairs[i].open.price);
					double ben = filledSize*openPrice;
					double will_sell = m_curTickData.last * filledSize;
					if(will_sell >= ben)
					{
						szFormat.Format("%s", CFuncCommon::Double2String(will_sell - ben + DOUBLE_PRECISION, m_nPriceDecimal).c_str());
						m_listCtrlOpen.SetItemText(i, 4, szFormat);
					}
					else
					{
						szFormat.Format("-%s", CFuncCommon::Double2String(ben - will_sell + DOUBLE_PRECISION, m_nPriceDecimal).c_str());
						m_listCtrlOpen.SetItemText(i, 4, szFormat);
					}
				}
			}
			szFormat.Format("%s", CFuncCommon::Double2String(m_vectorTradePairs[i].open.minPrice + DOUBLE_PRECISION, m_nPriceDecimal).c_str());
			m_listCtrlOpen.SetItemText(i, 5, szFormat);
			szFormat.Format("%s", CFuncCommon::Double2String(m_vectorTradePairs[i].open.maxPrice + DOUBLE_PRECISION, m_nPriceDecimal).c_str());
			m_listCtrlOpen.SetItemText(i, 6, szFormat);
		}
		if(m_vectorTradePairs[i].close.orderID != "")
		{
			m_listCtrlClose.InsertItem(i, "");
			szFormat.Format("%s", m_vectorTradePairs[i].close.price.c_str());
			m_listCtrlClose.SetItemText(i, 0, szFormat);
			szFormat.Format("%s/%s", m_vectorTradePairs[i].close.filledSize.c_str(), m_vectorTradePairs[i].close.size.c_str());
			m_listCtrlClose.SetItemText(i, 1, szFormat);
			szFormat.Format("%s", m_vectorTradePairs[i].close.status.c_str());
			m_listCtrlClose.SetItemText(i, 2, szFormat);
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

	strTemp = m_config.get("futures", "beginMoney", "");
	m_editCost.SetWindowText(strTemp.c_str());
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
	CString szCost = "";
	m_editCost.GetWindowText(szCost);
	
	m_strCoinType = strCoinType.GetBuffer();
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
	m_nFirstTradeSize = atoi(strFirstTradeSize.GetBuffer());
	

	m_config.set_value("futures", "coinType", m_strCoinType.c_str());
	m_config.set_value("futures", "futuresCycle", m_strFuturesCycle.c_str());
	m_config.set_value("futures", "leverage", m_strLeverage.c_str());
	m_config.set_value("futures", "futuresType", strFuturesType.GetBuffer());
	m_config.set_value("futures", "martingaleStepCnt", strMartingaleStepCnt.GetBuffer());
	m_config.set_value("futures", "martingaleMovePersent", strMartingaleMovePersent.GetBuffer());
	m_config.set_value("futures", "stopProfitFactor", strStopProfitFactor.GetBuffer());
	m_config.set_value("futures", "firstTradeSize", strFirstTradeSize.GetBuffer());
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
		if(m_tWaitNewSubDepth && tNow - m_tWaitNewSubDepth >= 5)
		{
			OKEX_WEB_SOCKET->API_FuturesEntrustDepth(true, m_bSwapFutures, m_strCoinType, m_strFuturesCycle);
			m_tWaitNewSubDepth = 0;
		}
		__CheckTrade();
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
		OKEX_HTTP->API_FuturesAccountInfoByCurrency(true, m_bSwapFutures, m_strCoinType);
}


void COKExMartingaleDlg::OnBnClickedButtonUpdateCost()
{
	m_beginMoney = 0.0;
	CString szCost;
	m_editCost.GetWindowText(szCost);
	m_beginMoney = stod(szCost.GetBuffer());
	m_config.set_value("spot", "beginMoney", szCost.GetBuffer());
	m_config.save("./config.ini");
}


void COKExMartingaleDlg::_UpdateProfitShow()
{
	if(m_coinAccountInfo.bValid && m_moneyAccountInfo.bValid && m_curTickData.bValid)
	{
		double now = m_coinAccountInfo.balance * m_curTickData.last + m_moneyAccountInfo.balance;
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
	CDialogEx::OnDestroy();

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
	OKEX_HTTP->API_FuturesAccountInfoByCurrency(false, m_bSwapFutures, strCurrency, &resInfo);
	if(resInfo.retObj.isObject() && resInfo.retObj["currency"].isString())
	{
		SSpotAccountInfo info;
		info.balance = stod(resInfo.retObj["balance"].asString());
		info.hold = stod(resInfo.retObj["hold"].asString());
		info.available = stod(resInfo.retObj["available"].asString());
		info.currency = resInfo.retObj["currency"].asString();
		g_pDlg->UpdateAccountInfo(info);
		return true;
	}
	return false;
}

void COKExMartingaleDlg::_SetTradeState(eTradeState state)
{
	m_eTradeState = state;
}


void COKExMartingaleDlg::ClearDepth()
{
	m_mapDepthSell.clear();
	m_mapDepthBuy.clear();
}

void COKExMartingaleDlg::UpdateDepthInfo(bool bBuy, SFuturesDepth& info)
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
}


bool COKExMartingaleDlg::CheckDepthInfo(int checkNum, std::string& checkSrc)
{
	std::map<std::string, SFuturesDepth>::reverse_iterator itBB = m_mapDepthBuy.rbegin();
	std::map<std::string, SFuturesDepth>::reverse_iterator itBE = m_mapDepthBuy.rend();
	std::map<std::string, SFuturesDepth>::iterator itSB = m_mapDepthSell.begin();
	std::map<std::string, SFuturesDepth>::iterator itSE = m_mapDepthSell.end();
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
		OKEX_WEB_SOCKET->API_FuturesEntrustDepth(false, m_bSwapFutures, m_strCoinType, m_strFuturesCycle);
		m_tWaitNewSubDepth = time(NULL);
		return false;
	}
	return true;
}