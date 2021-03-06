
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

#define BOLL_DATA m_vecBollData
#define BOLL_DATA_SIZE ((int)m_vecBollData.size())
#define REAL_BOLL_DATA_SIZE ((int)m_vecBollData.size() - m_nBollCycle -1)
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
	m_nBollCycle = 20;
	m_nPriceDecimal = 4;
	m_nVolumeDecimal = 3;
	m_nZhangKouCheckCycle = 20;
	m_nShouKouCheckCycle = 20;
	m_nZhangKouTrendCheckCycle = 5;
	m_bRun = false;
	m_eBollState = eBollTrend_Normal;
	m_eLastBollState = eBollTrend_Normal;
	m_nZhangKouDoubleConfirmCycle = 2;
	m_nShoukouDoubleConfirmCycle = 3;
	m_tListenPong = 0;
	m_strInstrumentID = "btc-usdt";
	m_strCoinType = "BTC";
	m_strMoneyType = "USDT";
	m_bTest = false;
	m_eTradeState = eTradeState_WaitOpen;
	m_martingaleStepCnt = 5;
	m_martingaleMovePersent = 0.02;
	m_fixedMoneyCnt = -1;
	m_curOpenFinishIndex = -1;
	m_bStopWhenFinish = false;
	m_beginMoney = 0.0;
	m_stopProfitFactor = 0.005;
	m_bStopProfitMove = true;
	m_tOpenTime = 0;
	m_bExit = false;
	m_tLastUpdate15Sec = 0;
	m_nStopProfitTimes = 0;
	m_nFinishTimes = 0;
	m_bTestCfg = false;
	m_strKlineCycle = "candle180s";
	m_nKlineCycle = 180;
}

void COKExMartingaleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_ctrlListLog);
	DDX_Control(pDX, IDC_COMBO1, m_combInstrumentID);
	DDX_Control(pDX, IDC_EDIT1, m_editMartingaleStepCnt);
	DDX_Control(pDX, IDC_EDIT2, m_editMartingaleMovePersent);
	DDX_Control(pDX, IDC_EDIT4, m_editFixedMoneyCnt);
	DDX_Control(pDX, IDC_EDIT5, m_editStopProfitFactor);
	DDX_Control(pDX, IDC_EDIT6, m_editCoin);
	DDX_Control(pDX, IDC_EDIT7, m_editProfit);
	DDX_Control(pDX, IDC_STATIC_COIN, m_staticCoin);
	DDX_Control(pDX, IDC_STATIC_MONEY, m_staticMoney);
	DDX_Control(pDX, IDC_EDIT9, m_editMoney);
	DDX_Control(pDX, IDC_EDIT8, m_editCost);
	DDX_Control(pDX, IDC_LIST3, m_listCtrlOpen);
	DDX_Control(pDX, IDC_LIST2, m_listCtrlClose);
	DDX_Control(pDX, IDC_STATIC_PRICE, m_staticPrice);
	DDX_Control(pDX, IDC_STATIC_STOP_PROFIT_TIME, m_staticStopProfitTimes);
	DDX_Control(pDX, IDC_STATIC_FINISH_TIME, m_staticFinishTimes);
	DDX_Control(pDX, IDC_RADIO1, m_btnStopProfitMove);
	DDX_Control(pDX, IDC_RADIO2, m_btnStopProfitFix);
}

BEGIN_MESSAGE_MAP(COKExMartingaleDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &COKExMartingaleDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON2, &COKExMartingaleDlg::OnBnClickedButtonTest)
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
	CFuncCommon::GetAllFileInDirectory("./data", m_setAllTestFile);
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
	m_combInstrumentID.InsertString(0, "BTC-USDT");
	m_combInstrumentID.InsertString(1, "ETC-USDT");
	m_combInstrumentID.InsertString(2, "EOS-USDT");
	m_combInstrumentID.InsertString(3, "XRP-USDT");
	m_combInstrumentID.InsertString(4, "ETH-USDT");
	m_combInstrumentID.InsertString(5, "TRX-USDT");

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
	std::string test = m_config.get("spot", "test", "");
	if(test == "1")
		m_bTest = true;
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
	for(int i=0; i<3; ++i)
	{
		SHttpResponse resInfo;
		OKEX_HTTP->API_SpotInstruments(false, &resInfo);
		bFound = false;
		if(resInfo.retObj.isArray())
		{
			for(int j=0; j<(int)resInfo.retObj.size(); ++j)
			{
				if(resInfo.retObj[j]["instrument_id"].asString() == m_strInstrumentID)
				{
					std::string strMinSize = resInfo.retObj[j]["min_size"].asString();
					int pos = strMinSize.find_first_of(".");
					if(pos != std::string::npos)
					{
						m_nVolumeDecimal = strMinSize.size() - pos - 1;
						bFound = true;
					}
					else
					{
						m_nVolumeDecimal = 0;
						bFound = true;
					}
					std::string strQuoteIncrement = resInfo.retObj[j]["quote_increment"].asString();
					pos = strQuoteIncrement.find_first_of(".");
					if(pos != std::string::npos)
					{
						m_nPriceDecimal = strQuoteIncrement.size() - pos - 1;
						bFound = true;
					}
					else
					{
						m_nPriceDecimal = 0;
						bFound = true;
					}
					LOCAL_INFO("VolumeDecimal=%d PriceDecimal=%d", m_nVolumeDecimal, m_nPriceDecimal);
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
	//m_bTest = true;
	m_bRun = true;
	m_coinAccountInfo.bValid = false;
	m_moneyAccountInfo.bValid = false;
	{
		if(!_CheckMoney(m_strMoneyType))
		{
			std::string msg = "未查询到币种信息[" + m_strMoneyType + "]";
			MessageBox(msg.c_str());
			return;
		}
	}
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
		OKEX_WEB_SOCKET->API_SpotKlineData(true, m_strKlineCycle, m_strCoinType, m_strMoneyType);
		OKEX_WEB_SOCKET->API_SpotTickerData(true, m_strCoinType, m_strMoneyType);
		OKEX_WEB_SOCKET->API_LoginFutures(m_apiKey, m_secretKey, time(NULL));
	}
}


void COKExMartingaleDlg::OnBnClickedButtonTest()
{
	if(!__SaveConfigCtrl())
		return;
	Test();
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
			CActionLog("boll", "差距%d秒", data.time - KLINE_DATA[KLINE_DATA_SIZE - 1].time);
			KLINE_DATA.clear();
			BOLL_DATA.clear();
		}
	}
	tm _tm;
	localtime_s(&_tm, &data.time);
	_snprintf(data.szTime, 20, "%d-%02d-%02d %02d:%02d:%02d", _tm.tm_year + 1900, _tm.tm_mon + 1, _tm.tm_mday, _tm.tm_hour, _tm.tm_min, _tm.tm_sec);
	KLINE_DATA.push_back(data);
	if(KLINE_DATA_SIZE >= m_nBollCycle)
	{
		double totalClosePrice = 0.0;
		for(int i = KLINE_DATA_SIZE - 1; i >= KLINE_DATA_SIZE - m_nBollCycle; --i)
		{
			totalClosePrice += KLINE_DATA[i].closePrice;
		}
		double ma = totalClosePrice / m_nBollCycle;
		double totalDifClosePriceSQ = 0.0;
		for(int i = KLINE_DATA_SIZE - 1; i >= KLINE_DATA_SIZE - m_nBollCycle; --i)
		{
			totalDifClosePriceSQ += ((KLINE_DATA[i].closePrice - ma)*(KLINE_DATA[i].closePrice - ma));
		}
		double md = sqrt(totalDifClosePriceSQ / m_nBollCycle);
		SBollInfo info;
		info.mb = ma;
		info.up = info.mb + 2 * md;
		info.dn = info.mb - 2 * md;
		info.mb = CFuncCommon::Round(info.mb + DOUBLE_PRECISION, m_nPriceDecimal);
		info.up = CFuncCommon::Round(info.up + DOUBLE_PRECISION, m_nPriceDecimal);
		info.dn = CFuncCommon::Round(info.dn + DOUBLE_PRECISION, m_nPriceDecimal);
		info.time = data.time;
		tm _tm;
		localtime_s(&_tm, &info.time);
		_snprintf(info.szTime, 20, "%d-%02d-%02d %02d:%02d:%02d", _tm.tm_year + 1900, _tm.tm_mon + 1, _tm.tm_mday, _tm.tm_hour, _tm.tm_min, _tm.tm_sec);
		BOLL_DATA.push_back(info);
		OnBollUpdate();
	}
	else
	{
		SBollInfo info;
		BOLL_DATA.push_back(info);
	}

}


void COKExMartingaleDlg::OnRevTickerInfo(STickerData &data)
{
	m_curTickData = data;
	m_curTickData.bValid = true;
	if(m_bTest)
	{
		for(int i = 0; i<(int)m_vectorTradePairs.size(); ++i)
		{
			double price = stod(m_vectorTradePairs[i].open.price);
			if(m_vectorTradePairs[i].open.orderID != "" && m_curTickData.sell <= price && m_vectorTradePairs[i].open.status != "filled" && m_vectorTradePairs[i].open.status != "cancelled")
			{
				double size = stod(m_vectorTradePairs[i].open.size);
				double filledSize = stod(m_vectorTradePairs[i].open.filledSize);
				filledSize += size/3;
				if(filledSize >= size)
				{
					m_vectorTradePairs[i].open.filledSize = m_vectorTradePairs[i].open.size;
					m_vectorTradePairs[i].open.status = "filled";
				}
				else
				{
					m_vectorTradePairs[i].open.filledSize = CFuncCommon::Double2String(filledSize+DOUBLE_PRECISION, m_nVolumeDecimal);
					m_vectorTradePairs[i].open.status = "part_filled";
				}
				CActionLog("trade", "[%s]ws更新订单信息 order=%s, size=%s, filled_size=%s, price=%s, status=%s, side=%s", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), m_vectorTradePairs[i].open.orderID.c_str(), m_vectorTradePairs[i].open.size.c_str(), m_vectorTradePairs[i].open.filledSize.c_str(), m_vectorTradePairs[i].open.price.c_str(), m_vectorTradePairs[i].open.status.c_str(), m_vectorTradePairs[i].open.side.c_str());
				return;
			}
			price = stod(m_vectorTradePairs[i].close.price);
			if(m_vectorTradePairs[i].close.orderID != "" && m_curTickData.buy >= price && m_vectorTradePairs[i].close.status != "filled" && m_vectorTradePairs[i].close.status != "cancelled")
			{
				double size = stod(m_vectorTradePairs[i].close.size);
				double filledSize = stod(m_vectorTradePairs[i].close.filledSize);
				filledSize += size/3;
				if(filledSize >= size)
				{
					m_vectorTradePairs[i].close.filledSize = m_vectorTradePairs[i].close.size;
					m_vectorTradePairs[i].close.status = "filled";
				}
				else
				{
					m_vectorTradePairs[i].close.filledSize = CFuncCommon::Double2String(filledSize+DOUBLE_PRECISION, m_nVolumeDecimal);
					m_vectorTradePairs[i].close.status = "part_filled";
				}
				CActionLog("trade", "[%s]ws更新订单信息 order=%s, size=%s, filled_size=%s, price=%s, status=%s, side=%s", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), m_vectorTradePairs[i].close.orderID.c_str(), m_vectorTradePairs[i].close.size.c_str(), m_vectorTradePairs[i].close.filledSize.c_str(), m_vectorTradePairs[i].close.price.c_str(), m_vectorTradePairs[i].close.status.c_str(), m_vectorTradePairs[i].close.side.c_str());
				return;
			}
		}
		__CheckTrade();
	}
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
	if(m_vectorTradePairs.size() && !m_bTest)
	{
		for(int i=0; i<(int)m_vectorTradePairs.size(); ++i)
		{
			if(m_vectorTradePairs[i].open.orderID != "" && m_vectorTradePairs[i].open.status != "filled" && m_vectorTradePairs[i].open.status != "cancelled")
			{
				BEGIN_API_CHECK
					SHttpResponse _resInfo;
					OKEX_HTTP->API_SpotOrderInfo(false, m_strInstrumentID, m_vectorTradePairs[i].open.orderID, &_resInfo);
					Json::Value& _retObj = _resInfo.retObj;
					if(_retObj.isObject() && _retObj["order_id"].isString())
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
						CActionLog("trade", "[断线 订单更新] order=%s, size=%s, filled_size=%s, price=%s, status=%s, side=%s", info.orderID.c_str(), info.size.c_str(), info.filledSize.c_str(), info.price.c_str(), info.status.c_str(), info.side.c_str());
						API_OK
					}
					else
					{
						_checkStr = _resInfo.strRet;
						boost::this_thread::sleep(boost::posix_time::seconds(1));
					}
				API_CHECK
				END_API_CHECK
			
			}
			if(m_vectorTradePairs[i].close.orderID != "" && m_vectorTradePairs[i].close.status != "filled" && m_vectorTradePairs[i].close.status != "cancelled")
			{
				BEGIN_API_CHECK
					SHttpResponse _resInfo;
					OKEX_HTTP->API_SpotOrderInfo(false, m_strInstrumentID, m_vectorTradePairs[i].close.orderID, &_resInfo);
					Json::Value& _retObj = _resInfo.retObj;
					if(_retObj.isObject() && _retObj["order_id"].isString())
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
						CActionLog("trade", "[断线 订单更新] order=%s, size=%s, filled_size=%s, price=%s, status=%s, side=%s", info.orderID.c_str(), info.size.c_str(), info.filledSize.c_str(), info.price.c_str(), info.status.c_str(), info.side.c_str());
						API_OK
					}
					else
					{
						_checkStr = _resInfo.strRet;
						boost::this_thread::sleep(boost::posix_time::seconds(1));
					}
				API_CHECK
				END_API_CHECK
			}
		}
		
	}
	OKEX_WEB_SOCKET->API_SpotOrderInfo(true, m_strCoinType, m_strMoneyType);
	OKEX_WEB_SOCKET->API_SpotAccountInfoByCurrency(true, m_strCoinType);
	OKEX_WEB_SOCKET->API_SpotAccountInfoByCurrency(true, m_strMoneyType);
}


void COKExMartingaleDlg::OnBollUpdate()
{
	CheckBollTrend();
}

void COKExMartingaleDlg::CheckBollTrend()
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

void COKExMartingaleDlg::__CheckTrend_Normal()
{
	if(REAL_BOLL_DATA_SIZE >= m_nZhangKouCheckCycle)//判断张口
	{
		int minBar = 0;
		double minValue = 100000.0;
		for(int i = BOLL_DATA_SIZE - 1; i >= BOLL_DATA_SIZE - m_nZhangKouCheckCycle; --i)
		{
			double offset = BOLL_DATA[i].up - BOLL_DATA[i].dn;
			if(offset < minValue)
			{
				minValue = offset;
				minBar = i;
			}
		}
		double offset = BOLL_DATA[BOLL_DATA_SIZE - 1].up - BOLL_DATA[BOLL_DATA_SIZE - 1].dn;
		if(offset / minValue > 2.5)
		{
			__SetBollState(eBollTrend_ZhangKou, 0, minValue);
			return;
		}
		else if(offset / minValue > 1.5)
		{
			int check = m_nZhangKouTrendCheckCycle / 2 + 1;
			if(KLINE_DATA_SIZE >= check)
			{
				int up = 0;
				int down = 0;
				for(int i = KLINE_DATA_SIZE - 1; i >= KLINE_DATA_SIZE - check; --i)
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
				if(up == check && KLINE_DATA[KLINE_DATA_SIZE - 1].closePrice > BOLL_DATA[BOLL_DATA_SIZE - 1].up)
				{
					double min_up = 100.0;
					for(int i = BOLL_DATA_SIZE - 1; i >= BOLL_DATA_SIZE - m_nZhangKouCheckCycle; --i)
					{
						if(BOLL_DATA[i].up < min_up)
							min_up = BOLL_DATA[i].up;
					}
					if((BOLL_DATA[BOLL_DATA_SIZE - 1].up / min_up) >= 1.005)
					{
						__SetBollState(eBollTrend_ZhangKou, 1, minValue);
						return;
					}

				}
				else if(down == check && KLINE_DATA[KLINE_DATA_SIZE - 1].closePrice < BOLL_DATA[BOLL_DATA_SIZE - 1].dn)
				{
					double max_down = 0;
					for(int i = BOLL_DATA_SIZE - 1; i >= BOLL_DATA_SIZE - m_nZhangKouCheckCycle; --i)
					{
						if(BOLL_DATA[i].dn > max_down)
							max_down = BOLL_DATA[i].dn;
					}
					if((BOLL_DATA[BOLL_DATA_SIZE - 1].dn / max_down) <= 0.095)
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
			{
				__SetBollState(eBollTrend_ShouKou);
				return;
			}
		}
	}
}

void COKExMartingaleDlg::__CheckTrend_ZhangKou()
{
	//寻找收口的同时用N个周期判断张口的成立
	if(BOLL_DATA_SIZE <= m_nZhangKouConfirmBar + m_nZhangKouDoubleConfirmCycle + 1)
	{
		if(m_bZhangKouUp)
		{
			if(BOLL_DATA[BOLL_DATA_SIZE - 1].up < BOLL_DATA[BOLL_DATA_SIZE - 2].up)
			{
				CActionLog("boll", "张口不成立");
				__SetBollState(m_eLastBollState);
			}

		}
		else
		{
			if(BOLL_DATA[BOLL_DATA_SIZE - 1].dn > BOLL_DATA[BOLL_DATA_SIZE - 2].dn)
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
		for(int i = m_nZhangKouConfirmBar; i < BOLL_DATA_SIZE; ++i)
		{
			if(m_bZhangKouUp)
			{
				if(BOLL_DATA[BOLL_DATA_SIZE - 1].up < BOLL_DATA[i].up && (BOLL_DATA_SIZE - 1 - i) > 1)
				{
					__SetBollState(eBollTrend_ShouKou);
					return;
				}
			}
			else
			{

				if(BOLL_DATA[BOLL_DATA_SIZE - 1].dn > BOLL_DATA[i].dn && (BOLL_DATA_SIZE - 1 - i) > 1)
				{
					__SetBollState(eBollTrend_ShouKou);
					return;
				}
			}
			double offset = BOLL_DATA[i].up - BOLL_DATA[i].dn;
			if(offset > maxValue)
			{
				maxValue = offset;
				maxBar = i - 1;
			}
		}
		double offset = BOLL_DATA[BOLL_DATA_SIZE - 1].up - BOLL_DATA[BOLL_DATA_SIZE - 1].dn;
		if((offset / m_nZhangKouMinValue) < 1.5)
		{
			__SetBollState(eBollTrend_ShouKou);
			return;
		}
		if((maxValue / offset) > 2.5)
		{
			double avgPrice = (KLINE_DATA[KLINE_DATA_SIZE - 1].highPrice + KLINE_DATA[KLINE_DATA_SIZE - 1].lowPrice) / 2;
			if(offset / avgPrice < 0.02)
			{
				__SetBollState(eBollTrend_ShouKou);
				return;
			}
		}
	}
	//超过25个周期直接进入收口通道状态
	if(KLINE_DATA_SIZE - 1 - m_nZhangKouConfirmBar >= 25)
	{
		__SetBollState(eBollTrend_ShouKouChannel, 1);
		return;
	}
}

void COKExMartingaleDlg::__CheckTrend_ShouKou()
{
	//用N个周期来确认收口完成
	if(BOLL_DATA_SIZE >= m_nShouKouConfirmBar + m_nShoukouDoubleConfirmCycle)
	{
		double last = 0.0;
		bool bRet = true;
		for(int i = 0; i<m_nShoukouDoubleConfirmCycle; ++i)
		{
			double offset = BOLL_DATA[BOLL_DATA_SIZE - 1 - i].up - BOLL_DATA[BOLL_DATA_SIZE - 1 - i].dn;

			double avgPrice = (KLINE_DATA[KLINE_DATA_SIZE - 1 - i].highPrice + KLINE_DATA[KLINE_DATA_SIZE - 1 - i].lowPrice) / 2;
			if(offset / avgPrice >= 0.02)
			{
				bRet = false;
				break;
			}
			if(last > 0)
			{
				if(offset >= last)
				{
					if(offset / last > 1.1)
					{
						bRet = false;
						break;
					}
				}
				else
				{
					if(last / offset > 1.1)
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
	for(int i = m_nShouKouConfirmBar; i < BOLL_DATA_SIZE; ++i)
	{
		double offset = BOLL_DATA[i].up - BOLL_DATA[i].dn;
		if(offset < minValue)
		{
			minValue = offset;
			minBar = i;
		}
	}
	double offset = BOLL_DATA[BOLL_DATA_SIZE - 1].up - BOLL_DATA[BOLL_DATA_SIZE - 1].dn;
	if(offset / minValue > 2.5)
	{
		__SetBollState(eBollTrend_ZhangKou, 0, minValue);
		return;
	}
	else if(offset / minValue > 1.5)
	{
		int check = m_nZhangKouTrendCheckCycle / 2 + 1;
		if(KLINE_DATA_SIZE >= check)
		{
			int up = 0;
			int down = 0;
			for(int i = KLINE_DATA_SIZE - 1; i >= KLINE_DATA_SIZE - check; --i)
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
			if(up == check && KLINE_DATA[KLINE_DATA_SIZE - 1].closePrice > BOLL_DATA[BOLL_DATA_SIZE - 1].up)
			{
				__SetBollState(eBollTrend_ZhangKou, 1, minValue);
				return;
			}
			else if(down == check && KLINE_DATA[KLINE_DATA_SIZE - 1].closePrice < BOLL_DATA[BOLL_DATA_SIZE - 1].dn)
			{
				__SetBollState(eBollTrend_ZhangKou, 1, minValue);
				return;
			}
		}
	}
	//超过25个周期直接进入收口通道状态
	if(KLINE_DATA_SIZE - 1 - m_nShouKouConfirmBar >= 25)
	{
		__SetBollState(eBollTrend_ShouKouChannel, 1);
		return;
	}
}


void COKExMartingaleDlg::__CheckTrend_ShouKouChannel()
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
			for(int i = BOLL_DATA_SIZE - 1; i >= 0; --i)
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
			double offset = BOLL_DATA[BOLL_DATA_SIZE - 1].up - BOLL_DATA[BOLL_DATA_SIZE - 1].dn;
			if(offset / minValue > 2.5)
			{
				__SetBollState(eBollTrend_ZhangKou, 0, minValue);
				return;
			}
			else if(offset / minValue > 1.5)
			{
				int check = m_nZhangKouTrendCheckCycle / 2 + 1;
				if(KLINE_DATA_SIZE >= check)
				{
					int up = 0;
					int down = 0;
					for(int i = KLINE_DATA_SIZE - 1; i >= KLINE_DATA_SIZE - check; --i)
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

void COKExMartingaleDlg::__SetBollState(eBollTrend state, int nParam, double dParam)
{
	m_eLastBollState = m_eBollState;
	m_eBollState = state;
	switch(m_eBollState)
	{
	case eBollTrend_ZhangKou:
		{
			m_nZhangKouConfirmBar = KLINE_DATA_SIZE - 1;
			m_nZhangKouTradeCheckBar = m_nZhangKouConfirmBar;
			m_nZhangKouMinValue = dParam;
			CString szInfo;
			szInfo.Format("张口产生<<<< 确认时间[%s] %s", CFuncCommon::FormatTimeStr(KLINE_DATA[KLINE_DATA_SIZE - 1].time).c_str(), (nParam == 0 ? "开口角判断" : "柱体穿插判断"));
			if(KLINE_DATA_SIZE >= m_nZhangKouTrendCheckCycle)
			{
				int up = 0;
				int down = 0;
				for(int i = KLINE_DATA_SIZE - 1; i >= KLINE_DATA_SIZE - m_nZhangKouTrendCheckCycle; --i)
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
			std::string strConfirmTime = CFuncCommon::FormatTimeStr(KLINE_DATA[KLINE_DATA_SIZE - 1].time);
			CActionLog("boll", "收口产生>>>> 确认时间[%s]", strConfirmTime.c_str());
			m_nShouKouConfirmBar = KLINE_DATA_SIZE - 1;
		}
		break;
	case eBollTrend_ShouKouChannel:
		{
			std::string strConfirmTime = CFuncCommon::FormatTimeStr(KLINE_DATA[KLINE_DATA_SIZE - 1].time);
			CActionLog("boll", "收口通道===== 确认时间[%s] %s", strConfirmTime.c_str(), (nParam == 0 ? "趋势判断" : "超时判断"));
			m_nShouKouChannelConfirmBar = KLINE_DATA_SIZE - 1;
		}
		break;
	default:
		break;
	}

}

void COKExMartingaleDlg::UpdateAccountInfo(SSpotAccountInfo& info)
{
	if(info.currency == m_strMoneyType)
	{
		m_moneyAccountInfo = info;
		m_moneyAccountInfo.bValid = true;
		if(m_bTest)
		{
			m_moneyAccountInfo.balance = 10000;
			m_moneyAccountInfo.available = 10000;
		}
		std::string moneyNum = CFuncCommon::Double2String(m_moneyAccountInfo.balance + DOUBLE_PRECISION, 7);
		{
			char szTemp[128];
			strcpy(szTemp, moneyNum.c_str());
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
			moneyNum = szTemp;
		}
		m_editMoney.SetWindowText(moneyNum.c_str());
		std::string strMoney = m_strMoneyType;
		strMoney += ":";
		m_staticMoney.SetWindowText(strMoney.c_str());
	}
	else if(info.currency == m_strCoinType)
	{
		m_coinAccountInfo = info;
		m_coinAccountInfo.bValid = true;

		std::string coinNum = CFuncCommon::Double2String(m_coinAccountInfo.balance + DOUBLE_PRECISION, 7);
		{
			char szTemp[128];
			strcpy(szTemp, coinNum.c_str());
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
			coinNum = szTemp;
		}
		m_editCoin.SetWindowText(coinNum.c_str());
		std::string strCoin = m_strCoinType;
		strCoin += ":";
		m_staticCoin.SetWindowText(strCoin.c_str());
	}
	_UpdateProfitShow();
}

void COKExMartingaleDlg::__CheckTrade()
{
	if(!m_bRun && !m_bTest)
		return;
	switch(m_eTradeState)
	{
	case eTradeState_WaitOpen:
		{
			if(m_bStopWhenFinish)
				break;
			//if(KLINE_DATA_SIZE < m_nBollCycle)
			//	break;
			if(!m_curTickData.bValid)
				break;
			if(!m_moneyAccountInfo.bValid)
				break;
			if(CFuncCommon::CheckEqual(m_moneyAccountInfo.available, 0.0))
				break;
			m_curOpenFinishIndex = -1;
			//张口向下或者张口向上的收口期不下单
			if(m_eBollState == eBollTrend_ZhangKou && !m_bZhangKouUp)
				break;
			if(m_eBollState == eBollTrend_ShouKou && m_eLastBollState == eBollTrend_ZhangKou && m_bZhangKouUp)
				break;
			//确认资金分为多少份
			int stepCnt = 0;
			for(int i = 0; i<m_martingaleStepCnt; ++i)
			{
				stepCnt += (int)pow(2, i);
			}
			bool bReCheckMoney = false;
ReCheckMoney:
			if(bReCheckMoney)
			{
				boost::this_thread::sleep(boost::posix_time::seconds(3));
				_CheckMoney(m_strMoneyType);
			}
			//确认每份资金金额
			double eachStepMoney = 0.0;
			if(m_fixedMoneyCnt < 0)
				eachStepMoney = m_moneyAccountInfo.available/(double)stepCnt;
			else
				eachStepMoney = m_fixedMoneyCnt/(double)stepCnt;
			//开始下单
			m_vectorTradePairs.clear();
			for(int i = 0; i<m_martingaleStepCnt; ++i)
			{
				double price = m_curTickData.buy*(1-m_martingaleMovePersent*i);
				std::string strPrice = CFuncCommon::Double2String(price+DOUBLE_PRECISION, m_nPriceDecimal);
				double money = eachStepMoney*(int)pow(2,i);
				double size = money / price;
				std::string strSize = CFuncCommon::Double2String(size+DOUBLE_PRECISION, m_nVolumeDecimal);
				bool isSizeOK = false;
				if(m_nPriceDecimal)
					isSizeOK = !(size < (1/double(10*m_nPriceDecimal)));
				else
					isSizeOK = size > 1;
				if(i==0 && !isSizeOK)
				{
					bReCheckMoney = true;
					goto ReCheckMoney;

				}
				if(isSizeOK)
				{
					if(m_bTest)
					{
						SSPotTradePairInfo info;
						info.open.orderID = CFuncCommon::GenUUID();
						info.open.price = strPrice;
						info.open.size = strSize;
						info.open.side = "buy";
						info.open.timeStamp = time(NULL);
						info.open.strTimeStamp = CFuncCommon::FormatTimeStr(m_curTickData.time);
						info.open.filledSize = "0";
						info.open.filledNotional = "0";
						info.open.status = "open";
						m_vectorTradePairs.push_back(info);
						CActionLog("trade", "[新批次 开买单][%s] order=%s, size=%s, filled_size=%s, price=%s, status=%s, side=%s", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), info.open.orderID.c_str(), info.open.size.c_str(), info.open.filledSize.c_str(), info.open.price.c_str(), info.open.status.c_str(), info.open.side.c_str());
					}
					else
					{
						BEGIN_API_CHECK
							SHttpResponse resInfo;
							OKEX_HTTP->API_SpotTrade(false, m_strInstrumentID, eTradeType_buy, strPrice, strSize, CFuncCommon::GenUUID(), &resInfo);
							Json::Value& retObj = resInfo.retObj;
							if(retObj.isObject() && retObj["result"].isBool() && retObj["result"].asBool())
							{
								std::string strOrderID = retObj["order_id"].asString();
								BEGIN_API_CHECK
									SHttpResponse _resInfo;
									OKEX_HTTP->API_SpotOrderInfo(false, m_strInstrumentID, strOrderID, &_resInfo);
									Json::Value& _retObj = _resInfo.retObj;
									if(_retObj.isObject() && _retObj["order_id"].isString())
									{
										SSPotTradePairInfo pairs;
										pairs.open.orderID = strOrderID;
										m_vectorTradePairs.push_back(pairs);

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
										CActionLog("trade", "[新批次 开买单] order=%s, size=%s, filled_size=%s, price=%s, status=%s, side=%s", info.orderID.c_str(), info.size.c_str(), info.filledSize.c_str(), info.price.c_str(), info.status.c_str(), info.side.c_str());
										API_OK
									}
									else
									{
										_checkStr = _resInfo.strRet;
										boost::this_thread::sleep(boost::posix_time::seconds(1));
									}
								API_CHECK
								END_API_CHECK
								API_OK
							}
							else
							{
								_checkStr = resInfo.strRet;
								boost::this_thread::sleep(boost::posix_time::seconds(1));
							}
						API_CHECK
						END_API_CHECK
					}
				}
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
				if(m_vectorTradePairs[i].open.orderID != "" && m_vectorTradePairs[i].open.status == "filled")
				{
					m_curOpenFinishIndex = i;
					finishPrice = stod(m_vectorTradePairs[i].open.price);
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
							if(m_vectorTradePairs[i].close.status != "filled" && m_vectorTradePairs[i].close.status != "cancelled")
							{
								double closeFinish = stod(m_vectorTradePairs[i].close.filledSize);
								if(!CFuncCommon::CheckEqual(closeFinish, 0.0))
								{
									double openFinish = stod(m_vectorTradePairs[i].open.filledSize);
									openFinish -= closeFinish;
									if(m_vectorTradePairs[i].open.closeSize == "0")
										m_vectorTradePairs[i].open.closeSize = m_vectorTradePairs[i].close.filledSize;
									else
									{
										double oldClose = stod(m_vectorTradePairs[i].open.closeSize);;
										oldClose += closeFinish;
										m_vectorTradePairs[i].open.closeSize = CFuncCommon::Double2String(oldClose+DOUBLE_PRECISION, m_nVolumeDecimal);
									}
									m_vectorTradePairs[i].open.filledSize = CFuncCommon::Double2String(openFinish+DOUBLE_PRECISION, m_nVolumeDecimal);
								}
								if(m_bTest)
								{
									m_vectorTradePairs[i].close.status = "cancelled";
									CActionLog("trade", "[新批次成交 老订单撤销成功][%s] order=%s", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), m_vectorTradePairs[i].close.orderID.c_str());
								}
								else
								{
									BEGIN_API_CHECK
										SHttpResponse resInfo;
										OKEX_HTTP->API_SpotCancelOrder(false, m_strInstrumentID, m_vectorTradePairs[i].close.orderID, &resInfo);
										Json::Value& retObj = resInfo.retObj;
										if(retObj.isObject() && retObj["result"].isBool() && retObj["result"].asBool())
										{
											BEGIN_API_CHECK
												SHttpResponse _resInfo;
												OKEX_HTTP->API_SpotOrderInfo(false, m_strInstrumentID, m_vectorTradePairs[i].close.orderID, &_resInfo);
												Json::Value& _retObj = _resInfo.retObj;
												if(_retObj.isObject() && _retObj["order_id"].isString())
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
													if(info.status == "cancelled")
													{
														CActionLog("trade", "[新批次成交 老订单撤销成功] order=%s", m_vectorTradePairs[i].close.orderID.c_str());
														API_OK
													}
													else
														boost::this_thread::sleep(boost::posix_time::seconds(1));
												}
												else
												{
													_checkStr = _resInfo.strRet;
													boost::this_thread::sleep(boost::posix_time::seconds(1));
												}
											API_CHECK
											END_API_CHECK

											API_OK
										}
										else
											_checkStr = resInfo.strRet;
									API_CHECK
									END_API_CHECK
								}

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
						if(m_bTest)
						{
							m_vectorTradePairs[i].close.orderID = CFuncCommon::GenUUID();
							m_vectorTradePairs[i].close.price = strPrice;
							m_vectorTradePairs[i].close.size = m_vectorTradePairs[i].open.filledSize;
							m_vectorTradePairs[i].close.side = "sell";
							m_vectorTradePairs[i].close.timeStamp = time(NULL);
							m_vectorTradePairs[i].close.strTimeStamp = CFuncCommon::FormatTimeStr(m_curTickData.time);
							m_vectorTradePairs[i].close.filledSize = "0";
							m_vectorTradePairs[i].close.filledNotional = "0";
							m_vectorTradePairs[i].close.status = "open";
							CActionLog("trade", "[新批次成交 开卖单][%s] order=%s, size=%s, filled_size=%s, price=%s, status=%s, side=%s", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), m_vectorTradePairs[i].close.orderID.c_str(), m_vectorTradePairs[i].close.size.c_str(), m_vectorTradePairs[i].close.filledSize.c_str(), m_vectorTradePairs[i].close.price.c_str(), m_vectorTradePairs[i].close.status.c_str(), m_vectorTradePairs[i].close.side.c_str());
						}
						else
						{
							BEGIN_API_CHECK
								SHttpResponse resInfo;
								OKEX_HTTP->API_SpotTrade(false, m_strInstrumentID, eTradeType_sell, strPrice, m_vectorTradePairs[i].open.filledSize, CFuncCommon::GenUUID(), &resInfo);
								Json::Value& retObj = resInfo.retObj;
								if(retObj.isObject() && retObj["result"].isBool() && retObj["result"].asBool())
								{
									std::string strOrderID = retObj["order_id"].asString();
									BEGIN_API_CHECK
										SHttpResponse _resInfo;
										OKEX_HTTP->API_SpotOrderInfo(false, m_strInstrumentID, strOrderID, &_resInfo);
										Json::Value& _retObj = _resInfo.retObj;
										if(_retObj.isObject() && _retObj["order_id"].isString())
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
											API_OK
										}
										else
										{
											_checkStr = _resInfo.strRet;
											boost::this_thread::sleep(boost::posix_time::seconds(1));
										}
									API_CHECK
									END_API_CHECK

									API_OK
								}
								else
									_checkStr = resInfo.strRet;
							API_CHECK
							END_API_CHECK
						}
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
								if(m_bTest)
									CActionLog("trade", "[超时未成交 撤销订单成功][%s] order=%s", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), m_vectorTradePairs[i].open.orderID.c_str());
								else
								{
									BEGIN_API_CHECK
										SHttpResponse resInfo;
										OKEX_HTTP->API_SpotCancelOrder(false, m_strInstrumentID, m_vectorTradePairs[i].open.orderID, &resInfo);
										Json::Value& retObj = resInfo.retObj;
										if(retObj.isObject() && retObj["result"].isBool() && retObj["result"].asBool())
										{
											BEGIN_API_CHECK
												SHttpResponse _resInfo;
												OKEX_HTTP->API_SpotOrderInfo(false, m_strInstrumentID, m_vectorTradePairs[i].open.orderID, &_resInfo);
												Json::Value& _retObj = _resInfo.retObj;
												if(_retObj.isObject() && _retObj["order_id"].isString())
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
													if(info.status == "cancelled")
													{
														CActionLog("trade", "[超时未成交 撤销订单成功] order=%s", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), m_vectorTradePairs[i].open.orderID.c_str());
														API_OK
													}
													else
														boost::this_thread::sleep(boost::posix_time::seconds(1));
												}
												else
												{
													_checkStr = _resInfo.strRet;
													boost::this_thread::sleep(boost::posix_time::seconds(1));
												}
											API_CHECK
											END_API_CHECK
											API_OK
										}
										else
											_checkStr = resInfo.strRet;
									API_CHECK
									END_API_CHECK
								}
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
									if(m_bTest)
										CActionLog("trade", "[最后一单 撤销订单成功[%s]] order=%s", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), pairsInfo.open.orderID.c_str());
									else
									{
										BEGIN_API_CHECK
											SHttpResponse resInfo;
											OKEX_HTTP->API_SpotCancelOrder(false, m_strInstrumentID, pairsInfo.open.orderID, &resInfo);
											Json::Value& retObj = resInfo.retObj;
											if(retObj.isObject() && retObj["result"].isBool() && retObj["result"].asBool())
											{
												CActionLog("trade", "[最后一单 撤销订单成功] order=%s", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), pairsInfo.open.orderID.c_str());
												API_OK
											}
											else
												_checkStr = resInfo.strRet;
										API_CHECK
										END_API_CHECK
									}
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
										if(m_bTest)
										{
											pairsInfo.open.status = "cancelled";
											CActionLog("trade", "[最后一单 撤销订单成功[%s]] order=%s", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), pairsInfo.open.orderID.c_str());
										}
										else
										{

											BEGIN_API_CHECK
												SHttpResponse resInfo;
												OKEX_HTTP->API_SpotCancelOrder(false, m_strInstrumentID, pairsInfo.open.orderID, &resInfo);
												Json::Value& retObj = resInfo.retObj;
												if(retObj.isObject() && retObj["result"].isBool() && retObj["result"].asBool())
												{
													BEGIN_API_CHECK
														SHttpResponse _resInfo;
														OKEX_HTTP->API_SpotOrderInfo(false, m_strInstrumentID, pairsInfo.open.orderID, &_resInfo);
														Json::Value& _retObj = _resInfo.retObj;
														if(_retObj.isObject() && _retObj["order_id"].isString())
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
															if(info.status == "cancelled")
															{
																CActionLog("trade", "[最后一单 撤销订单成功] order=%s", pairsInfo.open.orderID.c_str());
																API_OK
															}
															else
																boost::this_thread::sleep(boost::posix_time::seconds(1));
														}
														else
														{
															_checkStr = _resInfo.strRet;
															boost::this_thread::sleep(boost::posix_time::seconds(1));
														}
													API_CHECK
													END_API_CHECK
													API_OK
												}
												else
													_checkStr = resInfo.strRet;
											API_CHECK
											END_API_CHECK
										}
										//直接用卖一价挂卖单
										std::string strPrice = CFuncCommon::Double2String(m_curTickData.sell + DOUBLE_PRECISION, m_nPriceDecimal);
										if(m_bTest)
										{
											pairsInfo.close.orderID = CFuncCommon::GenUUID();
											pairsInfo.close.price = strPrice;
											pairsInfo.close.size = pairsInfo.open.filledSize;
											pairsInfo.close.side = "sell";
											pairsInfo.close.timeStamp = time(NULL);
											pairsInfo.close.strTimeStamp = CFuncCommon::FormatTimeStr(m_curTickData.time);
											pairsInfo.close.filledSize = "0";
											pairsInfo.close.filledNotional = "0";
											pairsInfo.close.status = "open";
											CActionLog("trade", "[最后一单 开卖单[%s]] order=%s, size=%s, filled_size=%s, price=%s, status=%s, side=%s", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), pairsInfo.close.orderID.c_str(), pairsInfo.close.size.c_str(), pairsInfo.close.filledSize.c_str(), pairsInfo.close.price.c_str(), pairsInfo.close.status.c_str(), pairsInfo.close.side.c_str());
										}
										else
										{
											BEGIN_API_CHECK
												SHttpResponse resInfo;
												OKEX_HTTP->API_SpotTrade(false, m_strInstrumentID, eTradeType_sell, strPrice, pairsInfo.open.filledSize, CFuncCommon::GenUUID(), &resInfo);
												Json::Value& retObj = resInfo.retObj;
												if(retObj.isObject() && retObj["result"].isBool() && retObj["result"].asBool())
												{
													std::string strOrderID = retObj["order_id"].asString();
													BEGIN_API_CHECK
														SHttpResponse _resInfo;
														OKEX_HTTP->API_SpotOrderInfo(false, m_strInstrumentID, strOrderID, &_resInfo);
														Json::Value& _retObj = _resInfo.retObj;
														if(_retObj.isObject() && _retObj["order_id"].isString())
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
															API_OK
														}
														else
														{
															_checkStr = _resInfo.strRet;
															boost::this_thread::sleep(boost::posix_time::seconds(1));
														}
													API_CHECK
													END_API_CHECK

													API_OK
												}
												else
													_checkStr = resInfo.strRet;
											API_CHECK
											END_API_CHECK
										}
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
														if(m_bTest)
														{
															m_vectorTradePairs[i].close.orderID = CFuncCommon::GenUUID();
															m_vectorTradePairs[i].close.price = strPrice;
															m_vectorTradePairs[i].close.size = m_vectorTradePairs[i].open.filledSize;
															m_vectorTradePairs[i].close.side = "sell";
															m_vectorTradePairs[i].close.timeStamp = time(NULL);
															m_vectorTradePairs[i].close.strTimeStamp = CFuncCommon::FormatTimeStr(m_curTickData.time);
															m_vectorTradePairs[i].close.filledSize = "0";
															m_vectorTradePairs[i].close.filledNotional = "0";
															m_vectorTradePairs[i].close.status = "open";
															CActionLog("trade", "[止盈 开卖单[%s]] order=%s, size=%s, filled_size=%s, price=%s, status=%s, side=%s", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), m_vectorTradePairs[i].close.orderID.c_str(), m_vectorTradePairs[i].close.size.c_str(), m_vectorTradePairs[i].close.filledSize.c_str(), m_vectorTradePairs[i].close.price.c_str(), m_vectorTradePairs[i].close.status.c_str(), m_vectorTradePairs[i].close.side.c_str());
														}
														else
														{
															BEGIN_API_CHECK
																SHttpResponse resInfo;
																OKEX_HTTP->API_SpotTrade(false, m_strInstrumentID, eTradeType_sell, strPrice, m_vectorTradePairs[i].open.filledSize, CFuncCommon::GenUUID(), &resInfo);
																Json::Value& retObj = resInfo.retObj;
																if(retObj.isObject() && retObj["result"].isBool() && retObj["result"].asBool())
																{
																	std::string strOrderID = retObj["order_id"].asString();
																	BEGIN_API_CHECK
																		SHttpResponse _resInfo;
																		OKEX_HTTP->API_SpotOrderInfo(false, m_strInstrumentID, strOrderID, &_resInfo);
																		Json::Value& _retObj = _resInfo.retObj;
																		if(_retObj.isObject() && _retObj["order_id"].isString())
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
																			API_OK
																		}
																		else
																		{
																			_checkStr = _resInfo.strRet;
																			boost::this_thread::sleep(boost::posix_time::seconds(1));
																		}
																	API_CHECK
																	END_API_CHECK

																	API_OK
																}
																else
																	_checkStr = resInfo.strRet;
															API_CHECK
															END_API_CHECK
														}
													}
												}
											}
										}
									}

								}
								else
								{
									std::string strPrice = CFuncCommon::Double2String(openPrice*(1+m_stopProfitFactor)+DOUBLE_PRECISION, m_nPriceDecimal);
									if(m_bTest)
									{
										pairsInfo.close.orderID = CFuncCommon::GenUUID();
										pairsInfo.close.price = strPrice;
										pairsInfo.close.size = pairsInfo.open.filledSize;
										pairsInfo.close.side = "sell";
										pairsInfo.close.timeStamp = time(NULL);
										pairsInfo.close.strTimeStamp = CFuncCommon::FormatTimeStr(m_curTickData.time);
										pairsInfo.close.filledSize = "0";
										pairsInfo.close.filledNotional = "0";
										pairsInfo.close.status = "open";
										CActionLog("trade", "[止盈 开卖单[%s]] order=%s, size=%s, filled_size=%s, price=%s, status=%s, side=%s", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), pairsInfo.close.orderID.c_str(), pairsInfo.close.size.c_str(), pairsInfo.close.filledSize.c_str(), pairsInfo.close.price.c_str(), pairsInfo.close.status.c_str(), pairsInfo.close.side.c_str());
									}
									else
									{
										BEGIN_API_CHECK
											SHttpResponse resInfo;
											OKEX_HTTP->API_SpotTrade(false, m_strInstrumentID, eTradeType_sell, strPrice, pairsInfo.open.filledSize, CFuncCommon::GenUUID(), &resInfo);
											Json::Value& retObj = resInfo.retObj;
											if(retObj.isObject() && retObj["result"].isBool() && retObj["result"].asBool())
											{
												std::string strOrderID = retObj["order_id"].asString();
												BEGIN_API_CHECK
													SHttpResponse _resInfo;
													OKEX_HTTP->API_SpotOrderInfo(false, m_strInstrumentID, strOrderID, &_resInfo);
													Json::Value& _retObj = _resInfo.retObj;
													if(_retObj.isObject() && _retObj["order_id"].isString())
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
														API_OK
													}
													else
													{
														_checkStr = _resInfo.strRet;
														boost::this_thread::sleep(boost::posix_time::seconds(1));
													}
												API_CHECK
												END_API_CHECK

												API_OK
											}
											else
											{
												_checkStr = resInfo.strRet;
												boost::this_thread::sleep(boost::posix_time::seconds(1));
											}
										API_CHECK
										END_API_CHECK
									}
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
											if(m_bTest)
												CActionLog("trade", "[止盈 撤销订单成功[%s]] order=%s", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), m_vectorTradePairs[i].open.orderID.c_str());
											else
											{
												BEGIN_API_CHECK
													SHttpResponse resInfo;
													OKEX_HTTP->API_SpotCancelOrder(false, m_strInstrumentID, m_vectorTradePairs[i].open.orderID, &resInfo);
													Json::Value& retObj = resInfo.retObj;
													if(retObj.isObject() && retObj["result"].isBool() && retObj["result"].asBool())
													{
														CActionLog("trade", "[止盈 撤销订单成功] order=%s", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), pairsInfo.open.orderID.c_str());
														API_OK
													}
													else
														_checkStr = resInfo.strRet;
												API_CHECK
												END_API_CHECK
											}
										}
									}
									if(m_vectorTradePairs[i].close.orderID != "")
									{
										if(m_vectorTradePairs[i].close.status != "filled")
										{
											if(m_bTest)
												CActionLog("trade", "[止盈 撤销订单成功[%s]] order=%s", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), m_vectorTradePairs[i].close.orderID.c_str());
											else
											{
												BEGIN_API_CHECK
													SHttpResponse resInfo;
													OKEX_HTTP->API_SpotCancelOrder(false, m_strInstrumentID, m_vectorTradePairs[i].close.orderID, &resInfo);
													Json::Value& retObj = resInfo.retObj;
													if(retObj.isObject() && retObj["result"].isBool() && retObj["result"].asBool())
													{
														CActionLog("trade", "[止盈 撤销订单成功] order=%s", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), pairsInfo.open.orderID.c_str());
														API_OK
													}
													else
														_checkStr = resInfo.strRet;
												API_CHECK
												END_API_CHECK
											}
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

void COKExMartingaleDlg::UpdateTradeInfo(SSPotTradeInfo& info)
{
	for(int i = 0; i<(int)m_vectorTradePairs.size(); i++)
	{
		if(m_vectorTradePairs[i].open.orderID == info.orderID && 
		   m_vectorTradePairs[i].open.status != "filled")
		{
			if((m_vectorTradePairs[i].open.status == "open" || m_vectorTradePairs[i].open.status == "") && info.status != "open" && m_curTickData.bValid)
			{
				m_vectorTradePairs[i].open.minPrice = m_curTickData.last;
				m_vectorTradePairs[i].open.maxPrice = m_curTickData.last;
			}
			m_vectorTradePairs[i].open.price = info.price;
			m_vectorTradePairs[i].open.size = info.size;
			m_vectorTradePairs[i].open.side = info.side;
			m_vectorTradePairs[i].open.strTimeStamp = info.strTimeStamp;
			m_vectorTradePairs[i].open.timeStamp = info.timeStamp;
			m_vectorTradePairs[i].open.filledSize = info.filledSize;
			m_vectorTradePairs[i].open.filledNotional = info.filledNotional;
			m_vectorTradePairs[i].open.status = info.status;
			break;
		}
		else if(m_vectorTradePairs[i].close.orderID == info.orderID &&
				m_vectorTradePairs[i].close.status != "filled")
		{
			m_vectorTradePairs[i].close.price = info.price;
			m_vectorTradePairs[i].close.size = info.size;
			m_vectorTradePairs[i].close.side = info.side;
			m_vectorTradePairs[i].close.strTimeStamp = info.strTimeStamp;
			m_vectorTradePairs[i].close.timeStamp = info.timeStamp;
			m_vectorTradePairs[i].close.filledSize = info.filledSize;
			m_vectorTradePairs[i].close.filledNotional = info.filledNotional;
			m_vectorTradePairs[i].close.status = info.status;
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
	std::string strTemp = m_config.get("spot", "instrument", "");
	if(strTemp == "BTC-USDT")
		m_combInstrumentID.SetCurSel(0);
	else if(strTemp == "ETC-USDT")
		m_combInstrumentID.SetCurSel(1);
	else if(strTemp == "EOS-USDT")
		m_combInstrumentID.SetCurSel(2);
	else if(strTemp == "XRP-USDT")
		m_combInstrumentID.SetCurSel(3);
	else if(strTemp == "ETH-USDT")
		m_combInstrumentID.SetCurSel(4);
	else if(strTemp == "TRX-USDT")
		m_combInstrumentID.SetCurSel(5);
	strTemp = m_config.get("spot", "martingaleStepCnt", "");
	m_editMartingaleStepCnt.SetWindowText(strTemp.c_str());

	strTemp = m_config.get("spot", "martingaleMovePersent", "");
	m_editMartingaleMovePersent.SetWindowText(strTemp.c_str());

	strTemp = m_config.get("spot", "fixedMoneyCnt", "");
	m_editFixedMoneyCnt.SetWindowText(strTemp.c_str());
	
	strTemp = m_config.get("spot", "stopProfitFactor", "");
	m_editStopProfitFactor.SetWindowText(strTemp.c_str());
	
	strTemp = m_config.get("spot", "stopProfitType", "");
	if(strTemp == "")
		m_btnStopProfitMove.SetCheck(1);
	else if(strTemp == "move")
		m_btnStopProfitMove.SetCheck(1);
	else if(strTemp == "fix")
		m_btnStopProfitFix.SetCheck(1);

	strTemp = m_config.get("spot", "beginMoney", "");
	m_editCost.SetWindowText(strTemp.c_str());

	
}

bool COKExMartingaleDlg::__SaveConfigCtrl()
{
	CString strInstrumentID;
	m_combInstrumentID.GetWindowText(strInstrumentID);
	if(strInstrumentID == "")
	{
		MessageBox("未选择交易对");
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
	CString strFixedMoneyCnt;
	m_editFixedMoneyCnt.GetWindowText(strFixedMoneyCnt);
	if(strFixedMoneyCnt == "")
	{
		MessageBox("未填写固定货币数量");
		return false;
	}
	CString strStopProfitFactor;
	m_editStopProfitFactor.GetWindowText(strStopProfitFactor);
	if(strStopProfitFactor == "")
	{
		MessageBox("未填写止盈系数");
		return false;
	}
	CString szCost = "";
	m_editCost.GetWindowText(szCost);
	
	
	m_strInstrumentID = strInstrumentID.GetBuffer();
	int pos = m_strInstrumentID.find_first_of("-");
	m_strCoinType = m_strInstrumentID.substr(0, pos);
	m_strMoneyType = m_strInstrumentID.substr(pos+1);
	m_martingaleStepCnt = atoi(strMartingaleStepCnt.GetBuffer());
	m_martingaleMovePersent = stod(strMartingaleMovePersent.GetBuffer());
	if(strFixedMoneyCnt == "0")
		m_fixedMoneyCnt = -1;
	else
		m_fixedMoneyCnt = stod(strFixedMoneyCnt.GetBuffer());
	m_stopProfitFactor = stod(strStopProfitFactor.GetBuffer());
	if(szCost != "")
		m_beginMoney = stod(szCost.GetBuffer());
	if(m_btnStopProfitMove.GetCheck())
		m_bStopProfitMove = true;
	else
		m_bStopProfitMove = false;

		


	m_config.set_value("spot", "instrument", m_strInstrumentID.c_str());
	m_config.set_value("spot", "martingaleStepCnt", strMartingaleStepCnt.GetBuffer());
	m_config.set_value("spot", "martingaleMovePersent", strMartingaleMovePersent.GetBuffer());
	m_config.set_value("spot", "fixedMoneyCnt", strFixedMoneyCnt.GetBuffer());
	m_config.set_value("spot", "stopProfitFactor", strStopProfitFactor.GetBuffer());
	if(m_bStopProfitMove)
		m_config.set_value("spot", "stopProfitType", "move");
	else
		m_config.set_value("spot", "stopProfitType", "fix");
	m_config.set_value("spot", "beginMoney", szCost.GetBuffer());
	m_config.save("./config.ini");
	return true;
}

void COKExMartingaleDlg::Test()
{
	m_moneyAccountInfo.bValid = true;
	m_moneyAccountInfo.available = 10000;
	m_moneyAccountInfo.balance = 10000;
	m_coinAccountInfo.bValid = true;
	m_coinAccountInfo.balance = 0;
	m_coinAccountInfo.available = 0;
	m_nPriceDecimal = 4;
	m_nVolumeDecimal = 1;
	m_bTest = true;
	m_bTestCfg = true;
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
		if(m_bTestCfg)
			TestCfg();
		else
		{
			__CheckTrade();
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


void COKExMartingaleDlg::TestCfg()
{
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

				std::string strTickChannel = "spot/ticker";

				std::string strKlineChannel = "spot/";
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
					g_pDlg->AddKlineData(data);
				}
				else if(strChannel == strTickChannel)
				{
					STickerData data;
					data.baseVolume24h = retObj["data"][0]["base_volume_24h"].asString();
					data.quoteVolume24h = retObj["data"][0]["quote_volume_24h"].asString();
					data.sell = stod(retObj["data"][0]["best_ask"].asString());
					data.buy = stod(retObj["data"][0]["best_bid"].asString());
					data.high = stod(retObj["data"][0]["high_24h"].asString());
					data.low = stod(retObj["data"][0]["low_24h"].asString());
					data.last = stod(retObj["data"][0]["last"].asString());
					data.time = CFuncCommon::ISO8601ToTime(retObj["data"][0]["timestamp"].asString());
					g_pDlg->OnRevTickerInfo(data);
				}
			}
			_UpdateTradeShow();
		}
		stream.close();
		++itB;
	}

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
	OKEX_HTTP->API_SpotAccountInfoByCurrency(false, strCurrency, &resInfo);
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
	if(m_eTradeState == eTradeState_WaitOpen)
	{
		boost::this_thread::sleep(boost::posix_time::seconds(10));
		BEGIN_API_CHECK
			if(_CheckMoney(m_strMoneyType))
				API_OK
		API_CHECK
		END_API_CHECK
	}
}