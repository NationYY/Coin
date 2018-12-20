
// okex_futures_dlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "okex_futures.h"
#include "okex_futures_dlg.h"
#include "afxdialogex.h"
#include <clib/lib/file/file_util.h>
#include "log/local_log.h"
#include "log/local_action_log.h"
#include "exchange/okex/okex_exchange.h"
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
// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���
CExchange* pExchange = NULL;
COKExFuturesDlg* g_pOKExFuturesDlg = NULL;
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
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


// COKExFuturesDlg �Ի���
void local_http_callbak_message(eHttpAPIType apiType, Json::Value& retObj, const std::string& strRet, int customData, std::string strCustomData)
{

}

void local_websocket_callbak_open(const char* szExchangeName)
{
	//AfxMessageBox("���ӳɹ�");
	LOCAL_INFO("���ӳɹ�");
	g_pOKExFuturesDlg->m_tListenPong = 0;
	if(g_pOKExFuturesDlg->m_bRun)
	{
		g_pOKExFuturesDlg->m_bRun = false;
		g_pOKExFuturesDlg->OnBnClickedButtonStart();
	}
}

void local_websocket_callbak_close(const char* szExchangeName)
{
	LOCAL_ERROR("�Ͽ�����");
	g_pOKExFuturesDlg->m_tListenPong = 0;
}

void local_websocket_callbak_fail(const char* szExchangeName)
{
	LOCAL_ERROR("����ʧ��");
	g_pOKExFuturesDlg->m_tListenPong = 0;
}

__int64 lastKlineTime = 0;
std::string lastKlineRetStr = "";
Json::Value lastKlineJson;
void local_websocket_callbak_message(eWebsocketAPIType apiType, const char* szExchangeName, Json::Value& retObj, const std::string& strRet)
{
	switch(apiType)
	{
	case eWebsocketAPIType_FuturesKline:
		{
			char* szEnd = NULL;
			__int64 curTime = _strtoi64(retObj[0]["data"][0][0].asString().c_str(), &szEnd, 10); 
			if(curTime >= lastKlineTime)
			{
				int volume = stoi(retObj[0]["data"][0][5].asString());
				double volumeByCurrency = stod(retObj[0]["data"][0][6].asString());
				if(volume == 0 && CFuncCommon::CheckEqual(volumeByCurrency, 0.0))
				{
					CActionLog("market", "%s", lastKlineRetStr.c_str());
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
				lastKlineTime = curTime;
				lastKlineRetStr = strRet;
				lastKlineJson = retObj;
			}
		}
		break;
	case eWebsocketAPIType_FuturesTicker:
		{
			CActionLog("market", "%s", strRet.c_str());
		}
		break;
	case eWebsocketAPIType_Pong:
		{
			g_pOKExFuturesDlg->Pong();
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
	m_eBollState = eBollTrend_Normal;
	m_eLastBollState = eBollTrend_Normal;
	m_nZhangKouDoubleConfirmCycle = 2;
	m_nShoukouDoubleConfirmCycle = 3;
	m_tListenPong = 0;
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
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// COKExFuturesDlg ��Ϣ�������

BOOL COKExFuturesDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	if(!m_config.open("./config.ini"))
		return FALSE;
	m_accessKey = m_config.get("futures", "apiKey", "");
	m_secretKey = m_config.get("futures", "secretKey", "");

	pExchange = new COkexExchange(m_accessKey, m_secretKey, true);
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

	CLocalActionLog::GetInstancePt()->set_log_path(log_path.c_str());
	CLocalActionLog::GetInstancePt()->start();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ������

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void COKExFuturesDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR COKExFuturesDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void COKExFuturesDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO:  �ڴ������Ϣ�����������/�����Ĭ��ֵ
	switch(nIDEvent)
	{
	case eTimerType_APIUpdate:
		{
			CLocalLogger::GetInstancePt()->SwapFront2Middle();
			if(pExchange)
				pExchange->Update();
			if(m_tListenPong && time(NULL) - m_tListenPong > 15)
			{
				m_tListenPong = 0;
				delete pExchange;
				pExchange = new COkexExchange(m_accessKey, m_secretKey, true);
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
			if(pExchange && pExchange->GetWebSocket()->Ping())
				m_tListenPong = time(NULL);
		}
		break;
	}
	CDialogEx::OnTimer(nIDEvent);
}


void COKExFuturesDlg::OnBnClickedButtonStart()
{
	if(m_bRun)
		return;
	std::string strKlineType = "3min";
	std::string strCoinType = "etc";
	std::string strFuturesCycle = "next_week";
	if(pExchange->GetWebSocket())
	{
		pExchange->GetWebSocket()->API_FuturesKlineData(true, strKlineType, strCoinType, strFuturesCycle);
		pExchange->GetWebSocket()->API_FuturesTickerData(true, strCoinType, strFuturesCycle);
		
	}
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
		info.mb = CFuncCommon::Round(int((info.mb+addValue)*scaleValue)/scaleValue+DOUBLE_PRECISION, m_nPriceDecimal);
		info.up = CFuncCommon::Round(int((info.up+addValue)*scaleValue)/scaleValue+DOUBLE_PRECISION, m_nPriceDecimal);
		info.dn = CFuncCommon::Round(int((info.dn+addValue)*scaleValue)/scaleValue+DOUBLE_PRECISION, m_nPriceDecimal);
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
		data.openPrice = CFuncCommon::Round(stod(retObj[0]["data"][0][1].asString())+DOUBLE_PRECISION, m_nPriceDecimal);
		data.highPrice = CFuncCommon::Round(stod(retObj[0]["data"][0][2].asString())+DOUBLE_PRECISION, m_nPriceDecimal);
		data.lowPrice = CFuncCommon::Round(stod(retObj[0]["data"][0][3].asString())+DOUBLE_PRECISION, m_nPriceDecimal);
		data.closePrice = CFuncCommon::Round(stod(retObj[0]["data"][0][4].asString())+DOUBLE_PRECISION, m_nPriceDecimal);
		data.volume = stoi(retObj[0]["data"][0][5].asString());
		data.volumeByCurrency = CFuncCommon::Round(stod(retObj[0]["data"][0][6].asString())+DOUBLE_PRECISION, m_nPriceDecimal);
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
	if(KLINE_DATA[KLINE_DATA_SIZE-1].time == 1545093000000)
		int a = 3;
	if(REAL_BOLL_DATA_SIZE >= m_nZhangKouCheckCycle)//�ж��ſ�
	{
		int minBar = 0;
		double minValue = 100.0;
		for(int i = BOLL_DATA_SIZE-1; i>=BOLL_DATA_SIZE-m_nZhangKouCheckCycle; --i)
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
	if(REAL_BOLL_DATA_SIZE >= m_nShouKouCheckCycle)//�ж��տ�
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
	//Ѱ���տڵ�ͬʱ��N�������ж��ſڵĳ���
	if(BOLL_DATA_SIZE <= m_nZhangKouConfirmBar+m_nZhangKouDoubleConfirmCycle+1)
	{
		if(m_bZhangKouUp)
		{
			if(BOLL_DATA[BOLL_DATA_SIZE-1].up < BOLL_DATA[BOLL_DATA_SIZE-2].up)
			{
				LOCAL_INFO("�ſڲ�����");
				__SetBollState(m_eLastBollState);
			}

		}
		else
		{
			if(BOLL_DATA[BOLL_DATA_SIZE-1].dn > BOLL_DATA[BOLL_DATA_SIZE-2].dn)
			{
				LOCAL_INFO("�ſڲ�����");
				__SetBollState(m_eLastBollState);
			}
		}
	}
	else
	{
		//Ѱ���տ�,��ȷ���ſڵ����ӿ�ʼ
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
	//����25������ֱ�ӽ����տ�ͨ��״̬
	if(KLINE_DATA_SIZE-1-m_nZhangKouConfirmBar >= 25)
	{
		__SetBollState(eBollTrend_ShouKouChannel, 1);
		return;
	}
}

void COKExFuturesDlg::__CheckTrend_ShouKou()
{
	//��N��������ȷ���տ����
	if(BOLL_DATA_SIZE >= m_nShouKouConfirmBar+m_nShoukouDoubleConfirmCycle)
	{
		double last = 0.0;
		bool bRet = true;
		for(int i = 0; i<m_nShoukouDoubleConfirmCycle; ++i)
		{
			double offset = BOLL_DATA[BOLL_DATA_SIZE-1-i].up - BOLL_DATA[BOLL_DATA_SIZE-1-i].dn;

			double avgPrice = (KLINE_DATA[KLINE_DATA_SIZE-1].highPrice + KLINE_DATA[KLINE_DATA_SIZE-1].lowPrice) / 2;
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
	//Ѱ���ſ�,��ȷ���տڵ����ӿ�ʼ
	int minBar = 0;
	double minValue = 100.0;
	for(int i = m_nShouKouConfirmBar; i<BOLL_DATA_SIZE; ++i)
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
	//����25������ֱ�ӽ����տ�ͨ��״̬
	if(KLINE_DATA_SIZE-1-m_nShouKouConfirmBar >= 25)
	{
		__SetBollState(eBollTrend_ShouKouChannel, 1);
		return;
	}
}


void COKExFuturesDlg::__CheckTrend_ShouKouChannel()
{
	//Ѱ���ſ�,��ȷ���տ�ͨ�������ӿ�ʼ
	int minBar = 0;
	double minValue = 100.0;
	for(int i=m_nShouKouChannelConfirmBar; i<BOLL_DATA_SIZE; ++i)
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
			m_nZhangKouMinValue = dParam;
			CString szInfo;
			szInfo.Format("�ſڲ���<<<< ȷ��ʱ��[%s] %s", CFuncCommon::FormatTimeStr(KLINE_DATA[KLINE_DATA_SIZE-1].time/1000).c_str(), (nParam==0 ? "���ڽ��ж�" : "���崩���ж�"));
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
					_szInfo.Format(" ����[�� %d:%d]", up, down);
				}
				else
				{
					m_bZhangKouUp = false;
					_szInfo.Format(" ����[�� %d:%d]", up, down);
				}
				szInfo.Append(_szInfo);
			}
			LOCAL_INFO(szInfo.GetBuffer());
		}
		break;
	case eBollTrend_ShouKou:
		{
			std::string strConfirmTime = CFuncCommon::FormatTimeStr(KLINE_DATA[KLINE_DATA_SIZE-1].time/1000);
			LOCAL_INFO("�տڲ���>>>> ȷ��ʱ��[%s]", strConfirmTime.c_str());
			m_nShouKouConfirmBar = KLINE_DATA_SIZE-1;
		}
		break;
	case eBollTrend_ShouKouChannel:
		{
			std::string strConfirmTime = CFuncCommon::FormatTimeStr(KLINE_DATA[KLINE_DATA_SIZE-1].time/1000);
			LOCAL_INFO("�տ�ͨ��===== ȷ��ʱ��[%s] %s", strConfirmTime.c_str(), (nParam==0 ? "�����ж�" : "��ʱ�ж�"));
			m_nShouKouChannelConfirmBar = KLINE_DATA_SIZE-1;
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

void COKExFuturesDlg::OnDestroy()
{
	delete pExchange;
	CLocalLogger::ReleaseInstance();
	CLocalActionLog::ReleaseInstance();
	CDialogEx::OnDestroy();

	// TODO:  �ڴ˴������Ϣ����������
}

void COKExFuturesDlg::Pong()
{
	m_tListenPong = 0;
}