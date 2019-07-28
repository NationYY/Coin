
// OKExMartingaleDlg.cpp : ʵ���ļ�
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
						

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���
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


// COKExMartingaleDlg �Ի���






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
	m_strKlineCycle = "candle180s";
	m_nKlineCycle = 180;
}

void COKExMartingaleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_ctrlListLog);
	DDX_Control(pDX, IDC_COMBO1, m_combCoinType);
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
	DDX_Control(pDX, IDC_EDIT3, m_editFuturesCycle);
	DDX_Control(pDX, IDC_COMBO2, m_combLeverage);
	DDX_Control(pDX, IDC_COMBO3, m_combFuturesType);
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


// COKExMartingaleDlg ��Ϣ�������

BOOL COKExMartingaleDlg::OnInitDialog()
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

	m_combFuturesType.InsertString(0, "�����Լ");
	m_combFuturesType.InsertString(1, "������Լ");

	m_listCtrlOpen.InsertColumn(0, "�۸�", LVCFMT_CENTER, 90);
	m_listCtrlOpen.InsertColumn(1, "�ɽ���", LVCFMT_CENTER, 100);
	m_listCtrlOpen.InsertColumn(2, "�۳���", LVCFMT_CENTER, 60);
	m_listCtrlOpen.InsertColumn(3, "״̬", LVCFMT_CENTER, 75);
	m_listCtrlOpen.InsertColumn(4, "�ο�����", LVCFMT_CENTER, 100);
	m_listCtrlOpen.InsertColumn(5, "��ͼ�", LVCFMT_CENTER, 90);
	m_listCtrlOpen.InsertColumn(6, "��߼�", LVCFMT_CENTER, 90);
	m_listCtrlOpen.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

	m_listCtrlClose.InsertColumn(0, "�۸�", LVCFMT_CENTER, 70);
	m_listCtrlClose.InsertColumn(1, "�ɽ���", LVCFMT_CENTER, 100);
	m_listCtrlClose.InsertColumn(2, "״̬", LVCFMT_CENTER, 70);
	m_listCtrlClose.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

	if(!m_config.open("./config.ini"))
		return FALSE;
	m_apiKey = m_config.get("spot", "apiKey", "");
	m_secretKey = m_config.get("spot", "secretKey", "");
	m_passphrase = m_config.get("spot", "passphrase", "");
	__InitConfigCtrl();
	m_logicThread = boost::thread(boost::bind(&COKExMartingaleDlg::_LogicThread, this));
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void COKExMartingaleDlg::OnPaint()
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
		MessageBox("δ�õ����׶���ϸ��Ϣ");
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
				MessageBox("���øܸ�ʧ��");
				return;
			}
		}
		else
		{
			if (!resInfo.retObj.isObject() || (resInfo.retObj["result"].asString() != "true"))
			{
				MessageBox("���øܸ�ʧ��");
				return;
			}
		}
	}
	m_coinAccountInfo.bValid = false;
	m_moneyAccountInfo.bValid = false;
	{
		if(!_CheckMoney(m_strCoinType))
		{
			std::string msg = "δ��ѯ��������Ϣ[" + m_strCoinType + "]";
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
	strTitle.Format("%s-%s", (m_bSwapFutures ? "������Լ" : "�����Լ"), m_strCoinType.c_str());
	CWnd *m_pMainWnd;
	m_pMainWnd = AfxGetMainWnd();
	m_pMainWnd->SetWindowText(strTitle);
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
			CActionLog("kline", "���%d��", data.time - KLINE_DATA[KLINE_DATA_SIZE - 1].time);
			KLINE_DATA.clear();
		}
	}
	tm _tm;
	localtime_s(&_tm, &data.time);
	_snprintf(data.szTime, 20, "%d-%02d-%02d %02d:%02d:%02d", _tm.tm_year + 1900, _tm.tm_mon + 1, _tm.tm_mday, _tm.tm_hour, _tm.tm_min, _tm.tm_sec);
	KLINE_DATA.push_back(data);
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
			if(m_vectorTradePairs[i].open.orderID != "" && m_vectorTradePairs[i].open.status != "filled" && m_vectorTradePairs[i].open.status != "cancelled")
			{
				BEGIN_API_CHECK;
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
						CActionLog("trade", "[���� ��������] order=%s, size=%s, filled_size=%s, price=%s, status=%s, side=%s", info.orderID.c_str(), info.size.c_str(), info.filledSize.c_str(), info.price.c_str(), info.status.c_str(), info.side.c_str());
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
			if(m_vectorTradePairs[i].close.orderID != "" && m_vectorTradePairs[i].close.status != "filled" && m_vectorTradePairs[i].close.status != "cancelled")
			{
				BEGIN_API_CHECK;
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
						CActionLog("trade", "[���� ��������] order=%s, size=%s, filled_size=%s, price=%s, status=%s, side=%s", info.orderID.c_str(), info.size.c_str(), info.filledSize.c_str(), info.price.c_str(), info.status.c_str(), info.side.c_str());
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
	OKEX_WEB_SOCKET->API_SpotOrderInfo(true, m_strInstrumentID);
	OKEX_WEB_SOCKET->API_SpotAccountInfoByCurrency(true, m_strCoinType);
	OKEX_WEB_SOCKET->API_SpotAccountInfoByCurrency(true, m_strMoneyType);
}

void COKExMartingaleDlg::UpdateAccountInfo(SSpotAccountInfo& info)
{
	if(info.currency == m_strMoneyType)
	{
		m_moneyAccountInfo = info;
		m_moneyAccountInfo.bValid = true;
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
	if(!m_bRun)
		return;
	std::string strTradeType = "";
	std::string clientOrderID = "";
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
			//�ſ����»����ſ����ϵ��տ��ڲ��µ�
			if(m_eBollState == eBollTrend_ZhangKou && !m_bZhangKouUp)
				break;
			if(m_eBollState == eBollTrend_ShouKou && m_eLastBollState == eBollTrend_ZhangKou && m_bZhangKouUp)
				break;
			//ȷ���ʽ��Ϊ���ٷ�
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
			//ȷ��ÿ���ʽ���
			double eachStepMoney = 0.0;
			if(m_fixedMoneyCnt < 0)
				eachStepMoney = m_moneyAccountInfo.available/(double)stepCnt;
			else
				eachStepMoney = m_fixedMoneyCnt/(double)stepCnt;
			//��ʼ�µ�
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
					
						BEGIN_API_CHECK;
							SHttpResponse resInfo;
							strTradeType = "buy";
							clientOrderID = CFuncCommon::GenUUID();
							OKEX_HTTP->API_SpotTrade(false, m_strInstrumentID, strTradeType, strPrice, strSize, clientOrderID, &resInfo);
							Json::Value& retObj = resInfo.retObj;
							if(retObj.isObject() && retObj["result"].isBool() && retObj["result"].asBool())
							{
								std::string strOrderID = retObj["order_id"].asString();
								BEGIN_API_CHECK;
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
										CActionLog("trade", "[������ ����] order=%s, size=%s, filled_size=%s, price=%s, status=%s, side=%s", info.orderID.c_str(), info.size.c_str(), info.filledSize.c_str(), info.price.c_str(), info.status.c_str(), info.side.c_str());
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
					//�Ȱѵ�ǰ�ҵ�����������
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
											CActionLog("trade", "[�����γɽ� �϶��������ɹ�] order=%s", m_vectorTradePairs[i].close.orderID.c_str());
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
					//���¹�����
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
								CActionLog("trade", "[�����γɽ� ������] order=%s, size=%s, filled_size=%s, price=%s, status=%s, side=%s", info.orderID.c_str(), info.size.c_str(), info.filledSize.c_str(), info.price.c_str(), info.status.c_str(), info.side.c_str());
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
				//��ʱ��δ�ɽ��ͳ�������
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
											CActionLog("trade", "[��ʱδ�ɽ� ���������ɹ�] order=%s", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), m_vectorTradePairs[i].open.orderID.c_str());
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
							CActionLog("trade", "��ʱδ�ɽ�");
							m_vectorTradePairs.clear();
							_SetTradeState(eTradeState_WaitOpen);
						}
						else	
							m_vectorTradePairs[0].open.bBeginStopProfit = true;
					}
				}
				else
				{
					//������״̬,�������е��Ӷ��ɽ�,��ʣ���һ��δ�ɽ���ɽ���
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
						//�������εĶ����������
						if(m_curOpenFinishIndex == m_vectorTradePairs.size()-1)
						{
							_SetTradeState(eTradeState_WaitOpen);
							CActionLog("finish_trade", "[�ɹ�����] ������");
							CActionLog("trade", "[�ɹ�����] ������");
							LOCAL_INFO("�ɹ�����һ������");
							m_nFinishTimes++;
							m_staticFinishTimes.SetWindowText(CFuncCommon::ToString(m_nFinishTimes));
						}
						else
						{
							//ֻ��ǰһ���������ɽ����˲��ж���һ�������
							if(m_vectorTradePairs[m_curOpenFinishIndex].close.orderID != "" && m_vectorTradePairs[m_curOpenFinishIndex].close.status == "filled")
							{
								SSPotTradePairInfo& pairsInfo = m_vectorTradePairs[m_curOpenFinishIndex + 1];
								//��һ��δ�ɽ�
								if(pairsInfo.open.status == "open")
								{
									BEGIN_API_CHECK;
									SHttpResponse resInfo;
									OKEX_HTTP->API_SpotCancelOrder(false, m_strInstrumentID, pairsInfo.open.orderID, &resInfo);
									Json::Value& retObj = resInfo.retObj;
									if (retObj.isObject() && retObj["result"].isBool() && retObj["result"].asBool())
									{
										CActionLog("trade", "[���һ�� ���������ɹ�] order=%s", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), pairsInfo.open.orderID.c_str());
										API_OK;
									}
									else
										_checkStr = resInfo.strRet;
									API_CHECK;
									END_API_CHECK;
									_SetTradeState(eTradeState_WaitOpen);
									CActionLog("finish_trade", "[�ɹ�����] ������");
									CActionLog("trade", "[�ɹ�����] ������");
									LOCAL_INFO("�ɹ�����һ������");
									m_nFinishTimes++;
									m_staticFinishTimes.SetWindowText(CFuncCommon::ToString(m_nFinishTimes));
								}
								else if(pairsInfo.open.status == "cancelled" || pairsInfo.open.status == "part_filled")
								{
									//���δ�ɽ��ȳ���
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
													CActionLog("trade", "[���һ�� ���������ɹ�] order=%s", pairsInfo.open.orderID.c_str());
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
										//ֱ������һ�۹�����
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
												CActionLog("trade", "[���һ�� ������] order=%s, size=%s, filled_size=%s, price=%s, status=%s, side=%s", info.orderID.c_str(), info.size.c_str(), info.filledSize.c_str(), info.price.c_str(), info.status.c_str(), info.side.c_str());
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
											CActionLog("finish_trade", "[�ɹ�����] ������");
											CActionLog("trade", "[�ɹ�����] ������");
											LOCAL_INFO("�ɹ�����һ������");
											m_nFinishTimes++;
											m_staticFinishTimes.SetWindowText(CFuncCommon::ToString(m_nFinishTimes));
										}
										else
										{
											if(pairsInfo.close.status == "cancelled" || pairsInfo.close.status == "filled")
											{
												_SetTradeState(eTradeState_WaitOpen);
												CActionLog("finish_trade", "[�ɹ�����] ������");
												CActionLog("trade", "[�ɹ�����] ������");
												LOCAL_INFO("�ɹ�����һ������");
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
											//�����гɽ���open�����м۴���,���೷��
											std::string strPrice = CFuncCommon::Double2String(m_curTickData.sell+DOUBLE_PRECISION, m_nPriceDecimal);
											for(int i = 0; i<(int)m_vectorTradePairs.size(); ++i)
											{
												if(m_vectorTradePairs[i].open.orderID != "")
												{
													//���м۴���
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
																CActionLog("trade", "[ֹӯ ������] order=%s, size=%s, filled_size=%s, price=%s, status=%s, side=%s", info.orderID.c_str(), info.size.c_str(), info.filledSize.c_str(), info.price.c_str(), info.status.c_str(), info.side.c_str());
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
											CActionLog("trade", "[ֹӯ ������] order=%s, size=%s, filled_size=%s, price=%s, status=%s, side=%s", info.orderID.c_str(), info.size.c_str(), info.filledSize.c_str(), info.price.c_str(), info.status.c_str(), info.side.c_str());
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
						//���ͷ��ֹӯ���
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
								CActionLog("finish_trade", "[ֹӯ�ɹ�] ������");
								CActionLog("trade", "[ֹӯ�ɹ�] ������");
								LOCAL_INFO("�ɹ�ֹӯһ������");
								//������δ�ɽ��ĵ��ӳ���
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
												CActionLog("trade", "[ֹӯ ���������ɹ�] order=%s", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), pairsInfo.open.orderID.c_str());
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
												CActionLog("trade", "[ֹӯ ���������ɹ�] order=%s", CFuncCommon::FormatTimeStr(m_curTickData.time).c_str(), pairsInfo.open.orderID.c_str());
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
	if (strTemp == "�����Լ")
		m_combFuturesType.SetCurSel(0);
	else if (strTemp == "������Լ")
		m_combFuturesType.SetCurSel(1);

	strTemp = m_config.get("futures", "martingaleStepCnt", "");
	m_editMartingaleStepCnt.SetWindowText(strTemp.c_str());

	strTemp = m_config.get("futures", "martingaleMovePersent", "");
	m_editMartingaleMovePersent.SetWindowText(strTemp.c_str());

	strTemp = m_config.get("futures", "fixedMoneyCnt", "");
	m_editFixedMoneyCnt.SetWindowText(strTemp.c_str());
	
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
		MessageBox("δѡ���Լ����");
		return false;
	}
	CString strCoinType;
	m_combCoinType.GetWindowText(strCoinType);
	if (strCoinType == "")
	{
		MessageBox("δѡ���Լ����");
		return false;
	}
	CString strFuturesCycle;
	m_editFuturesCycle.GetWindowText(strFuturesCycle);
	if (strFuturesCycle == "")
	{
		MessageBox("δ��д��Լ������");
		return false;
	}
	CString strLeverage;
	m_combLeverage.GetWindowText(strLeverage);
	if (strLeverage == "")
	{
		MessageBox("δѡ��ܸ�");
		return false;
	}
	CString strMartingaleStepCnt;
	m_editMartingaleStepCnt.GetWindowText(strMartingaleStepCnt);
	if(strMartingaleStepCnt == "")
	{
		MessageBox("δ��д��������״���");
		return false;
	}
	CString strMartingaleMovePersent;
	m_editMartingaleMovePersent.GetWindowText(strMartingaleMovePersent);
	if(strMartingaleMovePersent == "")
	{
		MessageBox("δ��д���������");
		return false;
	}
	CString strFixedMoneyCnt;
	m_editFixedMoneyCnt.GetWindowText(strFixedMoneyCnt);
	if(strFixedMoneyCnt == "")
	{
		MessageBox("δ��д�̶���������");
		return false;
	}
	CString strStopProfitFactor;
	m_editStopProfitFactor.GetWindowText(strStopProfitFactor);
	if(strStopProfitFactor == "")
	{
		MessageBox("δ��дֹӯϵ��");
		return false;
	}
	CString szCost = "";
	m_editCost.GetWindowText(szCost);
	
	m_strCoinType = strCoinType.GetBuffer();
	m_strFuturesCycle = strFuturesCycle.GetBuffer();
	m_strLeverage = strLeverage.GetBuffer();
	m_nLeverage = stoi(m_strLeverage);
	if (strFuturesType == "������Լ")
		m_bSwapFutures = true;
	else
		m_bSwapFutures = false;

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

		

	m_config.set_value("futures", "coinType", m_strCoinType.c_str());
	m_config.set_value("futures", "futuresCycle", m_strFuturesCycle.c_str());
	m_config.set_value("futures", "leverage", m_strLeverage.c_str());
	m_config.set_value("futures", "futuresType", strFuturesType.GetBuffer());
	m_config.set_value("futures", "martingaleStepCnt", strMartingaleStepCnt.GetBuffer());
	m_config.set_value("futures", "martingaleMovePersent", strMartingaleMovePersent.GetBuffer());
	m_config.set_value("futures", "fixedMoneyCnt", strFixedMoneyCnt.GetBuffer());
	m_config.set_value("futures", "stopProfitFactor", strStopProfitFactor.GetBuffer());
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
	if(m_eTradeState == eTradeState_WaitOpen)
	{
		boost::this_thread::sleep(boost::posix_time::seconds(10));
		BEGIN_API_CHECK;
			if(_CheckMoney(m_strMoneyType))
				API_OK;
		API_CHECK;
		END_API_CHECK;
	}
}