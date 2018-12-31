
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
	m_tradeCharge = 0.0015;
	m_fixedMoneyCnt = -1;
	m_curOpenFinishIndex = -1;

	m_moveStopProfit = 0.005;
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
	DDX_Control(pDX, IDC_EDIT3, m_editTradeCharge);
	DDX_Control(pDX, IDC_EDIT4, m_editFixedMoneyCnt);
	DDX_Control(pDX, IDC_EDIT5, m_editMoveStopProfit);
	DDX_Control(pDX, IDC_EDIT6, m_editTotalMoney);
}

BEGIN_MESSAGE_MAP(COKExMartingaleDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &COKExMartingaleDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON2, &COKExMartingaleDlg::OnBnClickedButtonTest)
	ON_WM_TIMER()
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
		OKEX_HTTP->API_SpotInstruments(false, resInfo);
		bFound = false;
		if(resInfo.retObj.isArray())
		{
			for(int j=0; j<(int)resInfo.retObj.size(); ++i)
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
						bFound = false;
						break;
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
						bFound = false;
						break;
					}
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
	m_accountInfo.bValid = false;
	if(OKEX_WEB_SOCKET)
	{
		OKEX_WEB_SOCKET->API_SpotKlineData(true, m_strKlineCycle, m_strCoinType, m_strMoneyType);
		OKEX_WEB_SOCKET->API_SpotTickerData(true, m_strCoinType, m_strMoneyType);
		OKEX_WEB_SOCKET->API_LoginFutures(m_apiKey, m_secretKey, time(NULL));
	}
	m_bRun = true;
}


void COKExMartingaleDlg::OnBnClickedButtonTest()
{

}


void COKExMartingaleDlg::OnTimer(UINT_PTR nIDEvent)
{
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
			__CheckTrade();
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
			LOCAL_ERROR("差距%d秒", data.time - KLINE_DATA[KLINE_DATA_SIZE - 1].time);
			KLINE_DATA.clear();
			BOLL_DATA.clear();
		}
	}
	tm* pTM = localtime(&data.time);
	_snprintf(data.szTime, 20, "%d-%02d-%02d %02d:%02d:%02d", pTM->tm_year + 1900, pTM->tm_mon + 1, pTM->tm_mday, pTM->tm_hour, pTM->tm_min, pTM->tm_sec);
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
		double addValue = 0.0;
		double scaleValue = 10;
		if(m_nPriceDecimal == 1)
		{
			addValue = 0.05;
			scaleValue = 10;
		}
		else if(m_nPriceDecimal == 2)
		{
			addValue = 0.005;
			scaleValue = 100;
		}
		else if(m_nPriceDecimal == 3)
		{
			addValue = 0.0005;
			scaleValue = 1000;
		}
		else if(m_nPriceDecimal == 4)
		{
			addValue = 0.00005;
			scaleValue = 10000;
		}

		SBollInfo info;
		info.mb = ma;
		info.up = info.mb + 2 * md;
		info.dn = info.mb - 2 * md;
		info.mb = CFuncCommon::Round(int((info.mb + addValue)*scaleValue) / scaleValue + DOUBLE_PRECISION, m_nPriceDecimal);
		info.up = CFuncCommon::Round(int((info.up + addValue)*scaleValue) / scaleValue + DOUBLE_PRECISION, m_nPriceDecimal);
		info.dn = CFuncCommon::Round(int((info.dn + addValue)*scaleValue) / scaleValue + DOUBLE_PRECISION, m_nPriceDecimal);
		info.time = data.time;
		tm* pTM = localtime(&info.time);
		_snprintf(info.szTime, 20, "%d-%02d-%02d %02d:%02d:%02d", pTM->tm_year + 1900, pTM->tm_mon + 1, pTM->tm_mday, pTM->tm_hour, pTM->tm_min, pTM->tm_sec);
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
}

void COKExMartingaleDlg::Pong()
{
	m_tListenPong = 0;
}


void COKExMartingaleDlg::OnLoginSuccess()
{
	OKEX_WEB_SOCKET->API_SpotOrderInfo(true, m_strCoinType, m_strMoneyType);
	OKEX_HTTP->API_SpotAccountInfoByCurrency(m_strMoneyType);
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
		double minValue = 100.0;
		for(int i = BOLL_DATA_SIZE - 1; i >= BOLL_DATA_SIZE - m_nZhangKouCheckCycle; --i)
		{
			double offset = BOLL_DATA[i].up - BOLL_DATA[i].dn;
			if(offset < minValue)
			{
				minValue = offset;
				minBar = i - 1;
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
				LOCAL_INFO("张口不成立");
				__SetBollState(m_eLastBollState);
			}

		}
		else
		{
			if(BOLL_DATA[BOLL_DATA_SIZE - 1].dn > BOLL_DATA[BOLL_DATA_SIZE - 2].dn)
			{
				LOCAL_INFO("张口不成立");
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
	double minValue = 100.0;
	for(int i = m_nShouKouConfirmBar; i < BOLL_DATA_SIZE; ++i)
	{
		double offset = BOLL_DATA[i].up - BOLL_DATA[i].dn;
		if(offset < minValue)
		{
			minValue = offset;
			minBar = i - 1;
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
	if(BOLL_DATA_SIZE - 1 > m_nShouKouChannelConfirmBar)
	{
		int minBar = 0;
		double minValue = 100.0;
		for(int i = BOLL_DATA_SIZE - 1; i >= m_nShouKouChannelConfirmBar; --i)
		{
			double offset = BOLL_DATA[i].up - BOLL_DATA[i].dn;
			if(offset < minValue)
			{
				minValue = offset;
				minBar = i - 1;
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
			LOCAL_INFO(szInfo.GetBuffer());
		}
		break;
	case eBollTrend_ShouKou:
		{
			std::string strConfirmTime = CFuncCommon::FormatTimeStr(KLINE_DATA[KLINE_DATA_SIZE - 1].time);
			LOCAL_INFO("收口产生>>>> 确认时间[%s]", strConfirmTime.c_str());
			m_nShouKouConfirmBar = KLINE_DATA_SIZE - 1;
		}
		break;
	case eBollTrend_ShouKouChannel:
		{
			std::string strConfirmTime = CFuncCommon::FormatTimeStr(KLINE_DATA[KLINE_DATA_SIZE - 1].time);
			LOCAL_INFO("收口通道===== 确认时间[%s] %s", strConfirmTime.c_str(), (nParam == 0 ? "趋势判断" : "超时判断"));
			m_nShouKouChannelConfirmBar = KLINE_DATA_SIZE - 1;
		}
		break;
	default:
		break;
	}

}

void COKExMartingaleDlg::UpdateAccountInfo(SSpotAccountInfo& info)
{
	if(!m_accountInfo.bValid)
	{
		m_accountInfo.bValid = true;
		OKEX_WEB_SOCKET->API_SpotAccountInfoByCurrency(true, m_strMoneyType);
	}
	m_accountInfo = info;
	m_accountInfo.bValid = true;
	std::string totalMoney = CFuncCommon::Double2String(m_accountInfo.balance+DOUBLE_PRECISION, 5);
	for(int i=totalMoney.size()-1; i>=0; --i)
	{
		if(totalMoney.at(i) == '0')
			totalMoney.replace(i, i, '\0');
		else
			break;
	}
	
	m_editTotalMoney.SetWindowText(totalMoney.c_str());
}

void COKExMartingaleDlg::__CheckTrade()
{
	if(!m_bRun)
		return;
	switch(m_eTradeState)
	{
	case eTradeState_WaitOpen:
		{
			//if(KLINE_DATA_SIZE < m_nBollCycle)
			//	break;
			if(!m_curTickData.bValid)
				break;
			if(!m_accountInfo.bValid)
				break;
			m_curOpenFinishIndex = -1;
			//张口向下或者张口向上的收口期不下单
			if(m_eBollState == eBollTrend_ZhangKou && !m_bZhangKouUp)
				break;
			if(m_eBollState == eBollTrend_ShouKou && m_eLastBollState == eBollTrend_ZhangKou && m_bZhangKouUp)
				break;
			//确认资金分为多少份
			int stepCnt = 0;
			int m = 2;
			for(int i = 0; i<m_martingaleStepCnt; ++i)
			{
				stepCnt += (int)pow(m, i);
			}
			//确认每份资金金额
			m_eachStepMoney = 0.0;
			if(m_fixedMoneyCnt < 0)
				m_eachStepMoney = m_accountInfo.available/(double)stepCnt;
			else
				m_eachStepMoney = m_fixedMoneyCnt/(double)stepCnt;
			//开始下单
			m_vectorTradePairs.clear();
			for(int i = 0; i<m_martingaleStepCnt; ++i)
			{
				std::string strClinetOrderID = CFuncCommon::ToString(CFuncCommon::GenUUID());
				double price = m_curTickData.buy*(1-m_martingaleMovePersent*(i+1));
				std::string strPrice = CFuncCommon::Double2String(price+DOUBLE_PRECISION, m_nPriceDecimal);
				double money = m_eachStepMoney*pow(m,i);
				double size = money / price;
				std::string strSize = CFuncCommon::Double2String(size+DOUBLE_PRECISION, m_nVolumeDecimal);
				SSPotTradePairInfo info;
				info.open.strClientOrderID = strClinetOrderID;
				info.open.waitClientOrderIDTime = time(NULL);
				m_vectorTradePairs.push_back(info);
				OKEX_HTTP->API_SpotTrade(m_strInstrumentID, eTradeType_buy, strPrice, strSize, strClinetOrderID);
			}
			m_eTradeState = eTradeState_WaitTradeOrder;
		}
		break;
	case eTradeState_WaitTradeOrder:
		{
			for(int i = 0; i<(int)m_vectorTradePairs.size(); i++)
			{
				if(m_vectorTradePairs[i].open.orderID == "")
					return;
			}
			m_eTradeState = eTradeState_Trading;
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
					m_vectorTradePairs[0].open.bBeginMoveStopProfit = true;
				else
				{
					m_vectorTradePairs[0].open.bBeginMoveStopProfit = false;
					finishPrice *= (1+m_martingaleMovePersent + m_curOpenFinishIndex*m_tradeCharge);
					std::string strPrice = CFuncCommon::Double2String(finishPrice+DOUBLE_PRECISION, m_nPriceDecimal);
					//先把当前挂的卖单撤销了
					
					for(int i=0; i<m_curOpenFinishIndex; ++i)
					{
						if(m_vectorTradePairs[i].close.orderID != "" && m_vectorTradePairs[i].close.status != "filled")
						{
							double closeFinish = stod(m_vectorTradePairs[i].close.filledSize);
							if(!CFuncCommon::CheckEqual(closeFinish, 0.0))
							{
								double openFinish = stod(m_vectorTradePairs[i].open.filledSize);
								openFinish -= closeFinish;
								m_vectorTradePairs[i].open.filledSize = CFuncCommon::Double2String(openFinish+DOUBLE_PRECISION, m_nVolumeDecimal);
							}
							std::string strClientOrderID = "0";
							OKEX_HTTP->API_SpotCancelOrder(m_strInstrumentID, m_vectorTradePairs[i].close.orderID, strClientOrderID);
							m_vectorTradePairs[i].close.Reset();
						}
					}
					//重新挂卖单
					for(int i = 0; i <= m_curOpenFinishIndex; ++i)
					{
						std::string strClinetOrderID = CFuncCommon::ToString(CFuncCommon::GenUUID());
						OKEX_HTTP->API_SpotTrade(m_strInstrumentID, eTradeType_sell, strPrice, m_vectorTradePairs[i].open.filledSize, strClinetOrderID);
						m_vectorTradePairs[i].close.strClientOrderID = strClinetOrderID;
						m_vectorTradePairs[i].close.waitClientOrderIDTime = time(NULL);
					}
				}
			}
			if(m_vectorTradePairs.size())
			{
				bool bAllFinish = true;
				for(int i = 0; i < (int)m_vectorTradePairs.size(); i++)
				{
					if(m_vectorTradePairs[i].open.orderID != "" && m_vectorTradePairs[i].open.status != "filled")
					{
						bAllFinish = false;
						break;
					}
				}
				if(bAllFinish)
					m_eTradeState = eTradeState_WaitOpen;
				else
				{
					if(m_vectorTradePairs[0].open.bBeginMoveStopProfit)
					{
						double openPrice = stod(m_vectorTradePairs[0].open.price);
						if(m_curTickData.last > openPrice)
						{
							double up = (m_curTickData.last - openPrice) / openPrice;
							int nowStep = int(up / m_moveStopProfit);
							if(nowStep - m_vectorTradePairs[0].open.stopProfit >= 2)
								m_vectorTradePairs[0].open.stopProfit = nowStep - 1;
						}
						if(m_vectorTradePairs[0].open.stopProfit)
						{
							if(m_curTickData.last <= (openPrice*(1+m_vectorTradePairs[0].open.stopProfit*m_moveStopProfit+m_moveStopProfit/2)))
							{
								//把所有成交的open单子市价处理,其余撤单
								std::string strPrice = CFuncCommon::Double2String((m_curTickData.last*0.8)+DOUBLE_PRECISION, m_nPriceDecimal);
								for(int i=0; i<=(int)m_vectorTradePairs.size(); ++i)
								{
									if(m_vectorTradePairs[i].open.orderID != "")
									{
										//先市价处理
										if(m_vectorTradePairs[i].open.status == "part_filled" || m_vectorTradePairs[i].open.status == "filled")
										{
											std::string strClinetOrderID = CFuncCommon::ToString(CFuncCommon::GenUUID());
											OKEX_HTTP->API_SpotTrade(m_strInstrumentID, eTradeType_sell, strPrice, m_vectorTradePairs[i].open.filledSize, strClinetOrderID);
										}
										//再撤销订单
										if(m_vectorTradePairs[i].open.status != "filled")
										{
											std::string strClientOrderID = "0";
											OKEX_HTTP->API_SpotCancelOrder(m_strInstrumentID, m_vectorTradePairs[i].open.orderID, strClientOrderID);
										}
									}
								}
								m_vectorTradePairs.clear();
								m_eTradeState = eTradeState_WaitOpen;
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

void COKExMartingaleDlg::OnTradeSuccess(std::string& strClientOrderID, std::string& serverOrderID)
{
	for(int i=0; i<(int)m_vectorTradePairs.size(); i++)
	{
		if(m_vectorTradePairs[i].open.strClientOrderID == strClientOrderID)
		{
			m_vectorTradePairs[i].open.orderID = serverOrderID;
			m_vectorTradePairs[i].open.waitClientOrderIDTime = 0;
			OKEX_HTTP->API_SpotOrderInfo(m_strInstrumentID, serverOrderID);
			break;
		}
		if(m_vectorTradePairs[i].close.strClientOrderID == strClientOrderID)
		{
			m_vectorTradePairs[i].close.orderID = serverOrderID;
			m_vectorTradePairs[i].close.waitClientOrderIDTime = 0;
			OKEX_HTTP->API_SpotOrderInfo(m_strInstrumentID, serverOrderID);
			break;
		}
	}
}

void COKExMartingaleDlg::UpdateTradeInfo(SSPotTradeInfo& info)
{
	for(int i = 0; i<(int)m_vectorTradePairs.size(); i++)
	{
		if(info.side == m_vectorTradePairs[i].open.side &&
		   m_vectorTradePairs[i].open.orderID == info.orderID && 
		   m_vectorTradePairs[i].open.status != "filled")
		{
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
		else if(info.side == m_vectorTradePairs[i].close.side &&
				m_vectorTradePairs[i].close.orderID == info.orderID &&
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
}

void COKExMartingaleDlg::__InitConfigCtrl()
{
	std::string strTemp = m_config.get("futures", "instrument", "");
	if(strTemp == "BTC-USDT")
		m_combInstrumentID.SetCurSel(0);
	strTemp = m_config.get("futures", "martingaleStepCnt", "");
	m_editMartingaleStepCnt.SetWindowText(strTemp.c_str());

	strTemp = m_config.get("futures", "martingaleMovePersent", "");
	m_editMartingaleMovePersent.SetWindowText(strTemp.c_str());

	strTemp = m_config.get("futures", "tradeCharge", "");
	m_editTradeCharge.SetWindowText(strTemp.c_str());

	strTemp = m_config.get("futures", "fixedMoneyCnt", "");
	m_editFixedMoneyCnt.SetWindowText(strTemp.c_str());
	
	strTemp = m_config.get("futures", "moveStopProfit", "");
	m_editMoveStopProfit.SetWindowText(strTemp.c_str());
	
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
	CString strTradeCharge;
	m_editTradeCharge.GetWindowText(strTradeCharge);
	if(strTradeCharge == "")
	{
		MessageBox("未填写交易手续费");
		return false;
	}
	CString strFixedMoneyCnt;
	m_editFixedMoneyCnt.GetWindowText(strFixedMoneyCnt);
	if(strFixedMoneyCnt == "")
	{
		MessageBox("未填写固定货币数量");
		return false;
	}
	CString strMoveStopProfit;
	m_editMoveStopProfit.GetWindowText(strMoveStopProfit);
	if(strMoveStopProfit == "")
	{
		MessageBox("未填写移动止盈系数");
		return false;
	}
	
	
	m_strInstrumentID = strInstrumentID.GetBuffer();
	int pos = m_strInstrumentID.find_first_of("-");
	m_strCoinType = m_strInstrumentID.substr(0, pos);
	m_strMoneyType = m_strInstrumentID.substr(pos+1);
	m_martingaleStepCnt = atoi(strMartingaleStepCnt.GetBuffer());
	m_martingaleMovePersent = stod(strMartingaleMovePersent.GetBuffer());
	m_tradeCharge = stod(strTradeCharge.GetBuffer());
	if(strFixedMoneyCnt == "0")
		m_fixedMoneyCnt = -1;
	else
		m_fixedMoneyCnt = stod(strFixedMoneyCnt.GetBuffer());
	m_moveStopProfit = stod(strMoveStopProfit.GetBuffer());
	m_config.set_value("futures", "instrument", m_strInstrumentID.c_str());
	m_config.set_value("futures", "martingaleStepCnt", strMartingaleStepCnt.GetBuffer());
	m_config.set_value("futures", "martingaleMovePersent", strMartingaleMovePersent.GetBuffer());
	m_config.set_value("futures", "tradeCharge", strTradeCharge.GetBuffer());
	m_config.set_value("futures", "fixedMoneyCnt", strFixedMoneyCnt.GetBuffer());
	m_config.set_value("futures", "moveStopProfit", strMoveStopProfit.GetBuffer());
	m_config.save("./config.ini");
	return true;
}