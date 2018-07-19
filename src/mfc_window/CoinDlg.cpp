
// CoinDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Coin.h"
#include "CoinDlg.h"
#include "afxdialogex.h"
#include <string>
#include "exchange/coinex/coinex_web_socket_api.h"
#include "exchange/coinex/coinex_http_api.h"
#include "exchange/okex/okex_web_socket_api.h"
#include "exchange/exx/exx_web_socket_api.h"
#include "exchange/exx/exx_http_api.h"
#include <MMSystem.h>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
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


// CCoinDlg �Ի���



CCoinDlg::CCoinDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CCoinDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCoinDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CCoinDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CCoinDlg::OnBnClickedButton1)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CCoinDlg ��Ϣ�������
void com_callbak_open()
{
	AfxMessageBox("���ӳɹ�");
};
void com_callbak_close()
{
	std::cout << "�����Ѿ��Ͽ��� " << std::endl;
};
void com_callbak_message(Json::Value& retObj, const std::string& strRet)
{
	OutputDebugString(strRet.c_str());
};

void local_http_callbak_message(eHttpAPIType apiType, Json::Value& retObj, const std::string& strRet)
{
	OutputDebugString(strRet.c_str());
};



CCoinexWebSocketAPI* comapi;
CCoinexHttpAPI* pHttpAPI = NULL;
COkexWebSocketAPI* pOkexWebSocketAPI = NULL;
CExxWebSocketAPI* pExxWebSocketAPI = NULL;
CExxHttpAPI* pExxHttpAPI = NULL;
CCoinexWebSocketAPI* pCoinexWebSocketAPI = NULL;
void CALLBACK UpdateFunc(UINT wTimerID, UINT msg, DWORD dwUser, DWORD dwl, DWORD dw2)
{
	CCoinDlg* pDlg = (CCoinDlg*)dwUser;
	if(pHttpAPI)
		pHttpAPI->Update();
	if(pExxHttpAPI)
		pExxHttpAPI->Update();
	//if(pCoinexWebSocketAPI)
	//	pCoinexWebSocketAPI->Update();
	//if(pCoinexWebSocketAPI)
	//	pCoinexWebSocketAPI->Update();
}

BOOL CCoinDlg::OnInitDialog()
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

	// TODO:  �ڴ���Ӷ���ĳ�ʼ������
	//comapi = new CCoinexWebSocketAPI("4836FE0E839B4ABB9541536CAE04FE9E", "65FAB5F4DDBB4EC5ABAF4B34337027758B4430B77971C958");
	//comapi->SetCallBackOpen(com_callbak_open);
	//comapi->SetCallBackClose(com_callbak_close);
	//comapi->SetCallBackMessage(com_callbak_message);
	//comapi->Run();//�������ӷ������߳�


	pHttpAPI = new CCoinexHttpAPI("4836FE0E839B4ABB9541536CAE04FE9E", "65FAB5F4DDBB4EC5ABAF4B34337027758B4430B77971C958", "application/json");
	pHttpAPI->SetCallBackMessage(local_http_callbak_message);
	pHttpAPI->Run(1);

	pExxHttpAPI = new CExxHttpAPI("c9e68ce5-c5e5-40da-9d1d-63238d2ffc79", "9b9b4f5b65c0ea198eb6b4a550ee83a2ff9ca52a", "");
	pExxHttpAPI->SetCallBackMessage(local_http_callbak_message);
	pExxHttpAPI->Run(1);

	//pCoinexWebSocketAPI = new CCoinexWebSocketAPI("4836FE0E839B4ABB9541536CAE04FE9E", "65FAB5F4DDBB4EC5ABAF4B34337027758B4430B77971C958");
	//pCoinexWebSocketAPI->SetCallBackOpen(com_callbak_open);
	//pCoinexWebSocketAPI->SetCallBackClose(com_callbak_close);
	//pCoinexWebSocketAPI->SetCallBackMessage(com_callbak_message);
	//pCoinexWebSocketAPI->Run();

	pExxWebSocketAPI = new CExxWebSocketAPI("4836FE0E839B4ABB9541536CAE04FE9E", "65FAB5F4DDBB4EC5ABAF4B34337027758B4430B77971C958");
	pExxWebSocketAPI->SetCallBackOpen(com_callbak_open);
	pExxWebSocketAPI->SetCallBackClose(com_callbak_close);
	pExxWebSocketAPI->SetCallBackMessage(com_callbak_message);
	pExxWebSocketAPI->Run();

	TIMECAPS   tc;
	UINT wTimerRes;
	if(timeGetDevCaps(&tc, sizeof(TIMECAPS)) != TIMERR_NOERROR)//���������һ����ý�嶨ʱ��       
		return FALSE;
	wTimerRes = min(max(tc.wPeriodMin, 1), tc.wPeriodMax);
	timeBeginPeriod(wTimerRes);
	MMRESULT g_wTimerID = timeSetEvent(6, wTimerRes, (LPTIMECALLBACK)UpdateFunc, (DWORD)this, TIME_PERIODIC);
	if(g_wTimerID == 0)
		return FALSE;
	SetTimer(1, 1, NULL);
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CCoinDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CCoinDlg::OnPaint()
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
HCURSOR CCoinDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CCoinDlg::OnBnClickedButton1()
{
	//comapi->Ping();
	//comapi->Ping();
	//comapi->Ping();
	//comapi->Ping();
	//comapi->LoginIn();
	//comapi->LoginIn();
	//if(pHttpAPI)
	//	pHttpAPI->API_balance();
	//if(pCoinexWebSocketAPI)
	//	pCoinexWebSocketAPI->API_sub_spot_ticker("bch_btc");
	if(pExxHttpAPI)
		pExxHttpAPI->API_Balance();
	if(pExxWebSocketAPI)
		pExxWebSocketAPI->API_EntrustDepth("ETH_BTC", 5, true);
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
}

void CCoinDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO:  �ڴ������Ϣ�����������/�����Ĭ��ֵ
	if(pCoinexWebSocketAPI)
		pCoinexWebSocketAPI->Update();
	if(pExxWebSocketAPI)
		pExxWebSocketAPI->Update();
	if(pOkexWebSocketAPI)
		pOkexWebSocketAPI->Update();
	CDialogEx::OnTimer(nIDEvent);
}
