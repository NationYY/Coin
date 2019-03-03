
// ManualOKExFuturesDlg.cpp : ʵ���ļ�
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
// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

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
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CManualOKExFuturesDlg �Ի���



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
	DDX_Control(pDX, IDC_STATIC_PROFIT, m_staticProfit);
	DDX_Control(pDX, IDC_STATIC_TODAY_PROFIT, m_staticTodayProfit);
	DDX_Control(pDX, IDC_STATIC_ACCOUNT, m_staticAccountInfo);
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


// CManualOKExFuturesDlg ��Ϣ��������

BOOL CManualOKExFuturesDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	// ��������...���˵������ӵ�ϵͳ�˵��С�

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

	m_listCtrlOrderOpen.InsertColumn(0, "�۸�", LVCFMT_CENTER, 90);
	m_listCtrlOrderOpen.InsertColumn(1, "����", LVCFMT_CENTER, 45);
	m_listCtrlOrderOpen.InsertColumn(2, "�ɽ���", LVCFMT_CENTER, 70);
	m_listCtrlOrderOpen.InsertColumn(3, "״̬", LVCFMT_CENTER, 45);
	m_listCtrlOrderOpen.InsertColumn(4, "�ο�����", LVCFMT_CENTER, 120);
	m_listCtrlOrderOpen.InsertColumn(5, "�µ�ʱ��", LVCFMT_CENTER, 115);
	m_listCtrlOrderOpen.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);


	m_listCtrlOrderClose.InsertColumn(0, "�۸�", LVCFMT_CENTER, 85);
	m_listCtrlOrderClose.InsertColumn(1, "�ɽ���", LVCFMT_CENTER, 70);
	m_listCtrlOrderClose.InsertColumn(2, "״̬", LVCFMT_CENTER, 45);
	m_listCtrlOrderClose.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

	m_listCtrlDepth.InsertColumn(0, "", LVCFMT_CENTER, 30);
	m_listCtrlDepth.InsertColumn(1, "��", LVCFMT_CENTER, 70);
	m_listCtrlDepth.InsertColumn(2, "��", LVCFMT_CENTER, 80);
	m_listCtrlDepth.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	// TODO:  �ڴ����Ӷ���ĳ�ʼ������
	if(!m_config.open("./config.ini"))
		return FALSE;
	m_apiKey = m_config.get("futures", "apiKey", "");
	m_secretKey = m_config.get("futures", "secretKey", "");
	m_passphrase = m_config.get("futures", "passphrase", "");
	m_nLastUpdateDay = m_config.get_int("futures", "lastUpdateDay", -1);
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
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի���������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CManualOKExFuturesDlg::OnPaint()
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
		CDialog::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
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
	strTemp = m_config.get("futures", "leverage", "");
	if(strTemp == "10")
		m_combLeverage.SetCurSel(0);
	else if(strTemp == "20")
		m_combLeverage.SetCurSel(1);
	strTemp = m_config.get("futures", "futuresType", "");
	if(strTemp == "�����Լ")
		m_combFuturesType.SetCurSel(0);
	else if(strTemp == "������Լ")
		m_combFuturesType.SetCurSel(1);
	strTemp = m_config.get("futures", "beginMoney", "");
	m_editCapital.SetWindowText(strTemp.c_str());
	strTemp = m_config.get("futures", "todayBeginMoney", "");
	m_editCapitalToday.SetWindowText(strTemp.c_str());
}

void CManualOKExFuturesDlg::__InitTradeConfigCtrl()
{
	std::string strTemp = m_config.get("futures", "futuresTradeSize", "");
	m_editFuturesTradeSize.SetWindowText(strTemp.c_str());
}

bool CManualOKExFuturesDlg::__SaveBaseConfigCtrl()
{
	CString strFuturesType;
	m_combFuturesType.GetWindowText(strFuturesType);
	if(strFuturesType == "")
	{
		MessageBox("δѡ���Լ����");
		return false;
	}
	CString strCoinType;
	m_combCoinType.GetWindowText(strCoinType);
	if(strCoinType == "")
	{
		MessageBox("δѡ���Լ����");
		return false;
	}
	CString strFuturesCycle;
	m_editFuturesCycle.GetWindowText(strFuturesCycle);
	if(strFuturesCycle == "")
	{
		MessageBox("δ��д��Լ������");
		return false;
	}
	CString strFuturesTradeSize;
	m_editFuturesTradeSize.GetWindowText(strFuturesTradeSize);
	if(strFuturesTradeSize == "")
	{
		MessageBox("δ��д�µ�����");
		return false;
	}
	CString strLeverage;
	m_combLeverage.GetWindowText(strLeverage);
	if(strLeverage == "")
	{
		MessageBox("δѡ��ܸ�");
		return false;
	}

	CString szCost = "";
	m_editCapital.GetWindowText(szCost);

	CString szTodayCost = "";
	m_editCapitalToday.GetWindowText(szTodayCost);

	m_strCoinType = strCoinType.GetBuffer();
	m_strFuturesCycle = strFuturesCycle.GetBuffer();
	m_strLeverage = strLeverage.GetBuffer();
	m_nLeverage = stoi(m_strLeverage);
	if(strFuturesType == "������Լ")
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

bool CManualOKExFuturesDlg::__SaveTradeConfigCtrl()
{
	CString strFuturesTradeSize;
	m_editFuturesTradeSize.GetWindowText(strFuturesTradeSize);
	if(strFuturesTradeSize == "")
	{
		MessageBox("δ��д�µ�����");
		return false;
	}
	m_strFuturesTradeSize = strFuturesTradeSize.GetBuffer();
	m_config.set_value("futures", "futuresTradeSize", m_strFuturesTradeSize.c_str());
	m_config.save("./config.ini");
	return true;
}

void CManualOKExFuturesDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO:  �ڴ�������Ϣ������������/�����Ĭ��ֵ
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
				LOCAL_ERROR("ping��ʱ����");
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
	if(!__SaveBaseConfigCtrl())
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
		MessageBox("δ�õ����׶���ϸ��Ϣ");
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
				MessageBox("���øܸ�ʧ��");
				return;
			}
		}
		else
		{
			if(!resInfo.retObj.isObject() || (resInfo.retObj["result"].asString() != "true"))
			{
				MessageBox("���øܸ�ʧ��");
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
	strTitle.Format("%s-%s", (m_bSwapFutures ? "������Լ" : "�����Լ"), m_strCoinType.c_str());
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
			CActionLog("trade", "http�µ��ɹ� client_order=%s, order=%s", itB->open.strClientOrderID.c_str(), itB->open.orderID.c_str());
			OKEX_HTTP->API_FuturesOrderInfo(true, m_bSwapFutures, m_strCoinType, m_strFuturesCycle, serverOrderID);
			bUpdate = true;
			break;
		}
		if(itB->close.strClientOrderID == clientOrderID)
		{
			itB->close.orderID = serverOrderID;
			itB->close.waitClientOrderIDTime = 0;
			CActionLog("trade", "http�µ��ɹ� client_order=%s, order=%s", itB->close.strClientOrderID.c_str(), itB->close.orderID.c_str());
			OKEX_HTTP->API_FuturesOrderInfo(true, m_bSwapFutures, m_strCoinType, m_strFuturesCycle, serverOrderID);
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
			m_vecTradePairInfo.erase(itB);
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
		time_t tNow = time(NULL);
		tm* pTM = localtime(&tNow);
		if(pTM->tm_yday != m_nLastUpdateDay)
		{
			double equity = stod(m_accountInfo.equity);
			if(m_nLastUpdateDay != -1)
				CFixActionLog("profit", "��������%.2f%%", (equity - m_todayBeginMoney) / m_todayBeginMoney * 100);

			m_todayBeginMoney = equity;
			m_nLastUpdateDay = pTM->tm_yday;
			m_editCapitalToday.SetWindowText(m_accountInfo.equity.c_str());
			m_config.set_value("futures", "lastUpdateDay", CFuncCommon::ToString(m_nLastUpdateDay));
			m_config.set_value("futures", "todayBeginMoney", m_accountInfo.equity.c_str());
			m_config.save("./config.ini");
		}
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
		if(!CFuncCommon::CheckEqual(m_todayBeginMoney, 0.0))
		{
			double equity = stod(m_accountInfo.equity);
			CString strTemp;
			if(equity >= m_todayBeginMoney)
				strTemp.Format("%.4f(%.2f%%)", equity - m_todayBeginMoney, (equity - m_todayBeginMoney) / m_todayBeginMoney * 100);
			else
				strTemp.Format("-%.4f(-%.2f%%)", m_todayBeginMoney - equity, (m_todayBeginMoney - equity) / m_todayBeginMoney * 100);
			m_staticTodayProfit.SetWindowText(strTemp);
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
		if(info.open.orderID != "")
		{
			if(CFuncCommon::CheckEqual(info.open.priceAvg, 0.0))
				szFormat = CFuncCommon::Double2String(info.open.price+DOUBLE_PRECISION, m_nPriceDecimal).c_str();
			else
				szFormat = CFuncCommon::Double2String(info.open.priceAvg+DOUBLE_PRECISION, m_nPriceDecimal).c_str();
			m_listCtrlOrderOpen.SetItemText(i, 0, szFormat);
			switch(info.open.tradeType)
			{
			case eFuturesTradeType_OpenBull:
				m_listCtrlOrderOpen.SetItemText(i, 1, "����");
				break;
			case eFuturesTradeType_OpenBear:
				m_listCtrlOrderOpen.SetItemText(i, 1, "����");
				break;
			default:
				break;
			}
			szFormat.Format("%s/%s", info.open.filledQTY.c_str(), info.open.size.c_str());
			m_listCtrlOrderOpen.SetItemText(i, 2, szFormat);
			if(info.open.status == "-1")
				m_listCtrlOrderOpen.SetItemText(i, 3, "cancelled");
			else if(info.open.status == "0")
				m_listCtrlOrderOpen.SetItemText(i, 3, "open");
			else if(info.open.status == "1")
				m_listCtrlOrderOpen.SetItemText(i, 3, "part_filled");
			else if(info.open.status == "2")
				m_listCtrlOrderOpen.SetItemText(i, 3, "filled");
			if(info.open.filledQTY != "0")
			{
				int count = stoi(info.open.filledQTY);
				int sizePrice = 10;
				if(m_strCoinType == "BTC")
					sizePrice = 100;
				double baozhengjin = (sizePrice*count/info.open.priceAvg)/m_nLeverage;
				if(info.open.tradeType == eFuturesTradeType_OpenBull)
				{
					double calcuPrice = m_curTickData.last;
					if(calcuPrice >= info.open.priceAvg)
					{
						double profitPersent = (calcuPrice-info.open.priceAvg)/info.open.priceAvg;
						double profit = profitPersent*m_nLeverage*baozhengjin;
						szFormat.Format("%s(%s%%)", CFuncCommon::Double2String(profit+DOUBLE_PRECISION, 5).c_str(), CFuncCommon::Double2String(profitPersent*100+DOUBLE_PRECISION, 2).c_str());
						m_listCtrlOrderOpen.SetItemText(i, 4, szFormat.GetBuffer());
					}
					else
					{
						double profitPersent = (info.open.priceAvg-calcuPrice)/info.open.priceAvg;
						double profit = profitPersent*m_nLeverage*baozhengjin;
						szFormat.Format("-%s(-%s%%)", CFuncCommon::Double2String(profit+DOUBLE_PRECISION, 5).c_str(), CFuncCommon::Double2String(profitPersent*100+DOUBLE_PRECISION, 2).c_str());
						m_listCtrlOrderOpen.SetItemText(i, 4, szFormat.GetBuffer());
					}

				}
				else if(info.open.tradeType == eFuturesTradeType_OpenBear)
				{
					double calcuPrice = m_curTickData.last;
					if(calcuPrice <= info.open.priceAvg)
					{
						double profitPersent = (info.open.priceAvg-calcuPrice)/info.open.priceAvg;
						double profit = profitPersent*m_nLeverage*baozhengjin;
						szFormat.Format("%s(%s%%)", CFuncCommon::Double2String(profit+DOUBLE_PRECISION, 5).c_str(), CFuncCommon::Double2String(profitPersent*100+DOUBLE_PRECISION, 2).c_str());
						m_listCtrlOrderOpen.SetItemText(i, 4, szFormat.GetBuffer());
					}
					else
					{
						double profitPersent = (calcuPrice-info.open.priceAvg)/info.open.priceAvg;
						double profit = profitPersent*m_nLeverage*baozhengjin;
						szFormat.Format("-%s(-%s%%)", CFuncCommon::Double2String(profit+DOUBLE_PRECISION, 5).c_str(), CFuncCommon::Double2String(profitPersent*100+DOUBLE_PRECISION, 2).c_str());
						m_listCtrlOrderOpen.SetItemText(i, 4, szFormat.GetBuffer());
					}
				}
			}
			tm _tm;
			localtime_s(&_tm, ((const time_t*)&(info.open.timeStamp)));
			szFormat.Format("%02d-%02d %02d:%02d:%02d", _tm.tm_mon+1, _tm.tm_mday, _tm.tm_hour, _tm.tm_min, _tm.tm_sec);
			m_listCtrlOrderOpen.SetItemText(i, 5, szFormat.GetBuffer());
		}
		else
		{
			m_listCtrlOrderOpen.SetItemText(i, 0, "");
			m_listCtrlOrderOpen.SetItemText(i, 1, "");
			m_listCtrlOrderOpen.SetItemText(i, 2, "");
			m_listCtrlOrderOpen.SetItemText(i, 3, "");
			m_listCtrlOrderOpen.SetItemText(i, 4, "");
			m_listCtrlOrderOpen.SetItemText(i, 5, "");

			m_listCtrlOrderClose.SetItemText(i, 0, "");
			m_listCtrlOrderClose.SetItemText(i, 1, "");
			m_listCtrlOrderClose.SetItemText(i, 2, "");
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

			m_listCtrlOrderClose.SetItemText(i, 0, "");
			m_listCtrlOrderClose.SetItemText(i, 1, "");
			m_listCtrlOrderClose.SetItemText(i, 2, "");
		}
	}
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
		MessageBox("δ��������");
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
	std::string strClientOrderID = CFuncCommon::GenUUID();
	OKEX_HTTP->API_FuturesTrade(m_bSwapFutures, type, m_strCoinType, m_strFuturesCycle, price, m_strFuturesTradeSize, m_strLeverage, strClientOrderID);
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
	pInfo->open.strClientOrderID = strClientOrderID;
	pInfo->open.waitClientOrderIDTime = time(NULL);
	pInfo->open.tradeType = type;
	CActionLog("trade", "��%s��%s��, price=%s, client_oid=%s", ((type == eFuturesTradeType_OpenBull) ? "��" : "��"), m_strFuturesTradeSize.c_str(), price.c_str(), strClientOrderID.c_str());
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
			stream << itB->open.orderID << "	" << itB->open.strClientOrderID;
		if(itB->close.orderID != "")
			stream  << "	" << itB->close.orderID << "	" << itB->close.strClientOrderID;
		else
			stream << "	0	0";
		stream << std::endl;
		++itB;
	}
	stream.close();
}