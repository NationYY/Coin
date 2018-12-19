
// okex_futures_dlg.cpp : 实现文件
//

#include "stdafx.h"
#include "okex_futures.h"
#include "okex_futures_dlg.h"
#include "afxdialogex.h"
#include <clib/lib/file/file_util.h>
#include "log/local_log.h"
#include "exchange/okex/okex_exchange.h"
#include "common/func_common.h"
#include "test_kline_data.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define BOLL_DATA m_vecBollData
#define BOLL_DATA_SIZE ((int)m_vecBollData.size())
#define REAL_BOLL_DATA_SIZE ((int)m_vecBollData.size() - m_nBollCycle -1)
#define KLINE_DATA m_vecKlineData
#define KLINE_DATA_SIZE ((int)m_vecKlineData.size())
// 用于应用程序“关于”菜单项的 CAboutDlg 对话框
CExchange* pExchange = NULL;
COKExFuturesDlg* g_pOKExFuturesDlg = NULL;
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

}

void local_websocket_callbak_open(const char* szExchangeName)
{
	//AfxMessageBox("连接成功");
	LOCAL_ERROR("连接成功");
	if(g_pOKExFuturesDlg->m_bRun)
	{
		g_pOKExFuturesDlg->m_bRun = true;
		g_pOKExFuturesDlg->OnBnClickedButtonStart();
	}
}

void local_websocket_callbak_close(const char* szExchangeName)
{
	LOCAL_ERROR("断开连接");
}

void local_websocket_callbak_fail(const char* szExchangeName)
{
	LOCAL_ERROR("连接失败");
}

std::string lastKlineTime = "";
std::string lastKlineRetStr = "";
Json::Value lastKlineJson;
void local_websocket_callbak_message(eWebsocketAPIType apiType, const char* szExchangeName, Json::Value& retObj, const std::string& strRet)
{
	switch(apiType)
	{
	case eWebsocketAPIType_FuturesKline:
		{
			std::string strTime = retObj[0]["data"][0][0].asString();
			if(lastKlineTime == "")
			{
				LOCAL_INFO(strRet.c_str());
			}
			else
			{
				if(lastKlineTime != strTime)
				{
					LOCAL_INFO(lastKlineRetStr.c_str());
					int volume = stoi(retObj[0]["data"][0][5].asString());
					double volumeByCurrency = stod(retObj[0]["data"][0][6].asString());
					if(volume == 0 && CFuncCommon::CheckEqual(volumeByCurrency, 0.0))
					{
						char* szEnd = NULL;
						SKlineData data;
						data.time = _strtoi64(lastKlineJson[0]["data"][0][0].asString().c_str(), &szEnd, 10);
						data.openPrice = stod(lastKlineJson[0]["data"][0][1].asString());
						data.highPrice = stod(lastKlineJson[0]["data"][0][2].asString());
						data.lowPrice = stod(lastKlineJson[0]["data"][0][3].asString());
						data.closePrice = stod(lastKlineJson[0]["data"][0][4].asString());
						data.volume = stoi(lastKlineJson[0]["data"][0][5].asString());
						data.volumeByCurrency = stod(lastKlineJson[0]["data"][0][6].asString());
						g_pOKExFuturesDlg->AddKlineData(data);
					}
				}
			}
			lastKlineTime = strTime;
			lastKlineRetStr = strRet;
			lastKlineJson = retObj;
		}
		break;
	default:
		break;
	}
}

COKExFuturesDlg::COKExFuturesDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(COKExFuturesDlg::IDD, pParent)
{
	g_pOKExFuturesDlg = this;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_nBollCycle = 20;
	m_nPriceDecimal = 3;
	m_nZhangKouCheckCycle = 20;
	m_nShouKouCheckCycle = 20;
	m_nZhangKouTrendCheckCycle = 5;
	m_bRun = false;
}

void COKExFuturesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(COKExFuturesDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON1, &COKExFuturesDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON2, &COKExFuturesDlg::OnBnClickedButtonTest)
END_MESSAGE_MAP()


// COKExFuturesDlg 消息处理程序

BOOL COKExFuturesDlg::OnInitDialog()
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
	const char* id = m_config.get("futures", "apiKey", "");
	const char* key = m_config.get("futures", "secretKey", "");

	pExchange = new COkexExchange(id, key, true);
	pExchange->SetHttpCallBackMessage(local_http_callbak_message);
	pExchange->SetWebSocketCallBackOpen(local_websocket_callbak_open);
	pExchange->SetWebSocketCallBackClose(local_websocket_callbak_close);
	pExchange->SetWebSocketCallBackFail(local_websocket_callbak_fail);
	pExchange->SetWebSocketCallBackMessage(local_websocket_callbak_message);
	pExchange->Run();




	SetTimer(eTimerType_APIUpdate, 1, NULL);
	SetTimer(eTimerType_Ping, 5000, NULL);

	clib::string log_path = "log/";
	bool bRet = clib::file_util::mkfiledir(log_path.c_str(), true);

	LocalLogger& localLogger = LocalLogger::GetInstance();
	localLogger.SetBatchMode(true);
	localLogger.SetLogPath(log_path.c_str());
	localLogger.Start();

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
			LocalLogger::GetInstancePt()->SwapFront2Middle();
			if(pExchange)
				pExchange->Update();
		}
		break;
	case eTimerType_Ping:
		{
			if(pExchange)
				pExchange->GetWebSocket()->Ping();
		}
		break;
	}
	CDialogEx::OnTimer(nIDEvent);
}


void COKExFuturesDlg::OnBnClickedButtonStart()
{
	if(m_bRun)
		return;
	LOCAL_INFO("");
	std::string strKlineType = "3min";
	std::string strCoinType = "etc";
	std::string strFuturesCycle = "next_week";
	if(pExchange->GetWebSocket())
		pExchange->GetWebSocket()->API_FuturesKlineData(true, strKlineType, strCoinType, strFuturesCycle);
	m_bRun = true;
}

void COKExFuturesDlg::AddKlineData(SKlineData& data)
{
	if(KLINE_DATA_SIZE)
	{
		if(data.time - KLINE_DATA[KLINE_DATA_SIZE-1].time != 180000)
		{
			KLINE_DATA.clear();
			BOLL_DATA.clear();
		}
	}
	KLINE_DATA.push_back(data);
	if((int)KLINE_DATA_SIZE >= m_nBollCycle)
	{
		double totalClosePrice = 0.0;
		for(int i = (int)KLINE_DATA_SIZE-1; i>=(int)KLINE_DATA_SIZE-m_nBollCycle; --i)
		{
			totalClosePrice += KLINE_DATA[i].closePrice;
		}
		double ma = totalClosePrice/m_nBollCycle;
		double totalDifClosePriceSQ = 0.0;
		for(int i = (int)KLINE_DATA_SIZE-1; i>=(int)KLINE_DATA_SIZE-m_nBollCycle; --i)
		{
			totalDifClosePriceSQ += ((KLINE_DATA[i].closePrice - ma)*(KLINE_DATA[i].closePrice - ma));
		}
		double md = sqrt(totalDifClosePriceSQ/m_nBollCycle);
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
		info.up = info.mb + 2*md;
		info.dn = info.mb - 2*md;
		info.mb = CFuncCommon::Round(int((info.mb+addValue)*scaleValue)/scaleValue+0.00000001, m_nPriceDecimal);
		info.up = CFuncCommon::Round(int((info.up+addValue)*scaleValue)/scaleValue+0.00000001, m_nPriceDecimal);
		info.dn = CFuncCommon::Round(int((info.dn+addValue)*scaleValue)/scaleValue+0.00000001, m_nPriceDecimal);
		info.time = data.time;
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
	int nSize = sizeof(g_test_kline)/sizeof(std::string);
	char* szEnd = NULL;
	for(int i = 0; i<nSize; ++i)
	{
		Json::Value retObj;
		Json::Reader reader;
		reader.parse(g_test_kline[i].c_str(), retObj);
		SKlineData data;
		data.time = _strtoi64(retObj[0]["data"][0][0].asString().c_str(), &szEnd, 10);
		data.openPrice = CFuncCommon::Round(stod(retObj[0]["data"][0][1].asString())+0.00000001, m_nPriceDecimal);
		data.highPrice = CFuncCommon::Round(stod(retObj[0]["data"][0][2].asString())+0.00000001, m_nPriceDecimal);
		data.lowPrice = CFuncCommon::Round(stod(retObj[0]["data"][0][3].asString())+0.00000001, m_nPriceDecimal);
		data.closePrice = CFuncCommon::Round(stod(retObj[0]["data"][0][4].asString())+0.00000001, m_nPriceDecimal);
		data.volume = stoi(retObj[0]["data"][0][5].asString());
		data.volumeByCurrency = CFuncCommon::Round(stod(retObj[0]["data"][0][6].asString())+0.00000001, m_nPriceDecimal);
		g_pOKExFuturesDlg->AddKlineData(data);
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
	if(KLINE_DATA[KLINE_DATA_SIZE-1].time == 1545093000000)
		int a = 3;
	if(REAL_BOLL_DATA_SIZE >= m_nZhangKouCheckCycle)//判断张口
	{
		int minBar = 0;
		double minValue = 100.0;
		for(int i = BOLL_DATA_SIZE-1; i>=(int)BOLL_DATA_SIZE-m_nZhangKouCheckCycle; --i)
		{
			double offset = BOLL_DATA[i].up - BOLL_DATA[i].dn;
			if(offset < minValue)
			{
				minValue = offset;
				minBar = i-1;
			}
		}
		double offset = BOLL_DATA[BOLL_DATA_SIZE-1].up - BOLL_DATA[BOLL_DATA_SIZE-1].dn;
		if(offset / minValue > 2.5)
		{
			__SetBollState(eBollTrend_ZhangKou, 0);
			return;
		}
		else if(offset / minValue > 1.5)
		{
			int check = m_nZhangKouTrendCheckCycle/2 + 1;
			if(KLINE_DATA_SIZE >= check)
			{
				int up = 0;
				int down = 0;
				for(int i = KLINE_DATA_SIZE-1; i>=(int)KLINE_DATA_SIZE-check; --i)
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
				if(up == check || down == check)
				{
					__SetBollState(eBollTrend_ZhangKou, 1);
					return;
				}
			}
		}
	}
	if(REAL_BOLL_DATA_SIZE >= m_nShouKouCheckCycle)//判断收口
	{
		int maxBar = 0;
		double maxValue = 0.0;
		for(int i = BOLL_DATA_SIZE-1; i>=(int)BOLL_DATA_SIZE-m_nShouKouCheckCycle; --i)
		{
			double offset = BOLL_DATA[i].up - BOLL_DATA[i].dn;
			if(offset > maxValue)
			{
				maxValue = offset;
				maxBar = i-1;
			}
		}
		double offset = BOLL_DATA[BOLL_DATA_SIZE-1].up - BOLL_DATA[BOLL_DATA_SIZE-1].dn;
		if(maxValue / offset > 2)
		{
			double avgPrice = (KLINE_DATA[KLINE_DATA_SIZE-1].highPrice + KLINE_DATA[KLINE_DATA_SIZE-1].lowPrice) / 2;
			if(offset/avgPrice < 0.013)
			{
				__SetBollState(eBollTrend_ShouKou);
				return;
			}
		}
	}
}

void COKExFuturesDlg::__CheckTrend_ZhangKou()
{
	if(KLINE_DATA_SIZE - m_nZhangKouConfirmBar >= 20)
		__SetBollState(eBollTrend_Normal);
}

void COKExFuturesDlg::__CheckTrend_ShouKou()
{
	if(KLINE_DATA_SIZE - m_nShouKouConfirmBar >= 20)
		__SetBollState(eBollTrend_Normal);

}
void COKExFuturesDlg::__SetBollState(eBollTrend state, int param)
{
	m_eBollState = state;
	switch(m_eBollState)
	{
	case eBollTrend_ZhangKou:
		{
			m_nZhangKouConfirmBar = KLINE_DATA_SIZE-1;
			CString szInfo;
			szInfo.Format("张口<<<< 确认时间[%s] %s", CFuncCommon::FormatTimeStr(KLINE_DATA[KLINE_DATA_SIZE-1].time/1000).c_str(), (param==0 ? "开口角判断" : "柱体穿插判断"));
			if(KLINE_DATA_SIZE >= m_nZhangKouTrendCheckCycle)
			{
				int up = 0;
				int down = 0;
				for(int i = KLINE_DATA_SIZE-1; i>=(int)KLINE_DATA_SIZE-m_nZhangKouTrendCheckCycle; --i)
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
					_szInfo.Format(" 趋势[涨 %d:%d]", up, down);
				else
					_szInfo.Format(" 趋势[跌 %d:%d]", up, down);
				szInfo.Append(_szInfo);
			}
			LOCAL_INFO(szInfo.GetBuffer());
		}
		break;
	case eBollTrend_ShouKou:
		{
			std::string strConfirmTime = CFuncCommon::FormatTimeStr(KLINE_DATA[KLINE_DATA_SIZE-1].time/1000);
			LOCAL_INFO("收口>>>> 确认时间[%s]", strConfirmTime.c_str());
			m_nShouKouConfirmBar = KLINE_DATA_SIZE-1;
		}
		break;
	default:
		break;
	}

}

void COKExFuturesDlg::OnBnClickedButtonTest()
{
	Test();
}