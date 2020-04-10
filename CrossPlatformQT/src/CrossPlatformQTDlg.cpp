
// CrossPlatformQTDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "CrossPlatformQT.h"
#include "CrossPlatformQTDlg.h"
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
CExchange* pExchange = NULL;
CCrossPlatformQTDlg* g_pDlg = NULL;
int LocalLogCallBackFunc(LOG_TYPE type, const char* szLog)
{
	//if(g_pDlg->m_ctrlListLog.GetCount() > 1500)
	//	g_pDlg->m_ctrlListLog.ResetContent();
	//g_pDlg->m_ctrlListLog.InsertString(0, szLog);
	//g_pDlg->SetHScroll();
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
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CCrossPlatformQTDlg �Ի���



CCrossPlatformQTDlg::CCrossPlatformQTDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CCrossPlatformQTDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	g_pDlg = this;
	m_tListenPong = 0;
	m_bRun = false;
}

void CCrossPlatformQTDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_combCoinType);
	DDX_Control(pDX, IDC_COMBO4, m_combMoneyType);
	DDX_Control(pDX, IDC_EDIT1, m_editFuturesCycle);
	DDX_Control(pDX, IDC_COMBO2, m_combLeverage);
	DDX_Control(pDX, IDC_COMBO3, m_combFuturesType);
}

BEGIN_MESSAGE_MAP(CCrossPlatformQTDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON1, &CCrossPlatformQTDlg::OnBnClickedButtonStart)
END_MESSAGE_MAP()


// CCrossPlatformQTDlg ��Ϣ�������

BOOL CCrossPlatformQTDlg::OnInitDialog()
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
	m_combCoinType.InsertString(9, "BSV");

	m_combMoneyType.InsertString(0, "USD");
	m_combMoneyType.InsertString(1, "USDT");

	m_combLeverage.InsertString(0, "5");
	m_combLeverage.InsertString(1, "10");
	m_combLeverage.InsertString(2, "15");
	m_combLeverage.InsertString(3, "20");
	m_combLeverage.InsertString(4, "25");
	m_combLeverage.InsertString(5, "50");

	m_combFuturesType.InsertString(0, "�����Լ");
	m_combFuturesType.InsertString(1, "������Լ");

	// TODO:  �ڴ���Ӷ���ĳ�ʼ������
	if(!m_config.open("./config.ini"))
		return FALSE;
	m_apiKey = m_config.get("futures", "apiKey", "");
	m_secretKey = m_config.get("futures", "secretKey", "");
	m_passphrase = m_config.get("futures", "passphrase", "");

	__InitBaseConfigCtrl();

	pExchange = new COkexExchange(m_apiKey, m_secretKey, m_passphrase, true);
	pExchange->SetHttpCallBackMessage(local_http_callbak_message);
	pExchange->SetWebSocketCallBackOpen(local_websocket_callbak_open);
	pExchange->SetWebSocketCallBackClose(local_websocket_callbak_close);
	pExchange->SetWebSocketCallBackFail(local_websocket_callbak_fail);
	pExchange->SetWebSocketCallBackMessage(local_websocket_callbak_message);
	pExchange->Run();

	SetTimer(eTimerType_APIUpdate, 1, NULL);
	SetTimer(eTimerType_Ping, 15000, NULL);

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

void CCrossPlatformQTDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CCrossPlatformQTDlg::OnPaint()
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
HCURSOR CCrossPlatformQTDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CCrossPlatformQTDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO:  �ڴ������Ϣ�����������/�����Ĭ��ֵ
	switch(nIDEvent)
	{
	case eTimerType_APIUpdate:
		{
			CLocalLogger::GetInstancePt()->SwapFront2Middle();
			if(pExchange)
				pExchange->Update();
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
			if(pExchange->GetWebSocket()->Ping())
				m_tListenPong = time(NULL);
		}
		break;
	}
	CDialogEx::OnTimer(nIDEvent);
}

void CCrossPlatformQTDlg::Pong()
{
	m_tListenPong = 0;
}

void CCrossPlatformQTDlg::OnLoginSuccess()
{

}

void CCrossPlatformQTDlg::OnBnClickedButtonStart()
{
	if(m_bRun)
		return;
	if(!__SaveBaseConfigCtrl())
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
		HTTP->API_FuturesInstruments(false, m_bSwapFutures, &resInfo);
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
		HTTP->API_FuturesSetLeverage(false, m_bSwapFutures, m_strCoinType, m_strMoneyType, strLeverage, &resInfo);
		if(m_bSwapFutures)
		{
			std::string strInstrumentID = m_strCoinType + "-" + m_strMoneyType + "-SWAP";
			if(!resInfo.retObj.isObject() || ((resInfo.retObj["instrument_id"].asString() != strInstrumentID) && (resInfo.retObj["code"].asInt() != 35017)))
			{
				MessageBox("���øܸ�ʧ��");
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
				MessageBox("���øܸ�ʧ��");
				return;
			}
		}
	}
	if(WEB_SOCKET)
	{
		WEB_SOCKET->API_FuturesTickerData(true, m_bSwapFutures, m_strCoinType, m_strMoneyType, m_strFuturesCycle);
		WEB_SOCKET->API_LoginFutures(m_apiKey, m_secretKey, time(NULL));
		WEB_SOCKET->API_FuturesEntrustDepth(true, m_bSwapFutures, m_strCoinType, m_strMoneyType, m_strFuturesCycle);
	}
	CString strTitle;
	strTitle.Format("%s-%s", (m_bSwapFutures ? "������Լ" : "�����Լ"), m_strCoinType.c_str());
	CWnd *m_pMainWnd;
	m_pMainWnd = AfxGetMainWnd();
	m_pMainWnd->SetWindowText(strTitle);
	m_bRun = true;
}

bool CCrossPlatformQTDlg::__SaveBaseConfigCtrl()
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
	CString strLeverage;
	m_combLeverage.GetWindowText(strLeverage);
	if(strLeverage == "")
	{
		MessageBox("δѡ��ܸ�");
		return false;
	}
	CString strMoneyType;
	m_combMoneyType.GetWindowText(strMoneyType);
	if(strMoneyType == "")
	{
		MessageBox("δѡ���Լ����");
		return false;
	}
	m_strCoinType = strCoinType.GetBuffer();
	m_strMoneyType = strMoneyType.GetBuffer();
	m_strFuturesCycle = strFuturesCycle.GetBuffer();
	m_strLeverage = strLeverage.GetBuffer();
	m_nLeverage = stoi(m_strLeverage);
	if(strFuturesType == "������Լ")
		m_bSwapFutures = true;
	else
		m_bSwapFutures = false;
	m_config.set_value("futures", "coinType", m_strCoinType.c_str());
	m_config.set_value("futures", "moneyType", m_strMoneyType.c_str());
	m_config.set_value("futures", "futuresCycle", m_strFuturesCycle.c_str());
	m_config.set_value("futures", "leverage", m_strLeverage.c_str());
	m_config.set_value("futures", "futuresType", strFuturesType.GetBuffer());
	m_config.save("./config.ini");
	return true;
}

void CCrossPlatformQTDlg::__InitBaseConfigCtrl()
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
	else if(strTemp == "BSV")
		m_combCoinType.SetCurSel(9);

	strTemp = m_config.get("futures", "moneyType", "");
	if(strTemp == "USD")
		m_combMoneyType.SetCurSel(0);
	else if(strTemp == "USDT")
		m_combMoneyType.SetCurSel(1);
	strTemp = m_config.get("futures", "futuresCycle", "");
	m_editFuturesCycle.SetWindowText(strTemp.c_str());
	strTemp = m_config.get("futures", "leverage", "");
	if(strTemp == "5")
		m_combLeverage.SetCurSel(0);
	else if(strTemp == "10")
		m_combLeverage.SetCurSel(1);
	else if(strTemp == "15")
		m_combLeverage.SetCurSel(2);
	else if(strTemp == "20")
		m_combLeverage.SetCurSel(3);
	else if(strTemp == "25")
		m_combLeverage.SetCurSel(4);
	else if(strTemp == "50")
		m_combLeverage.SetCurSel(5);

	strTemp = m_config.get("futures", "futuresType", "");
	if(strTemp == "�����Լ")
		m_combFuturesType.SetCurSel(0);
	else if(strTemp == "������Լ")
		m_combFuturesType.SetCurSel(1);
}