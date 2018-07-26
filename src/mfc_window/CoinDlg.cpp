
// CoinDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Coin.h"
#include "CoinDlg.h"
#include "afxdialogex.h"
#include <string>
#include "exchange/zbg/zbg_exchange.h"
#include "exchange/exx/exx_exchange.h"
#include <MMSystem.h>
#include "common/func_common.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���
CCoinDlg* g_pCoinDlg = NULL;
const int CHECK_STATE_RATE = 3;
int g_trade_pair_index = 1;
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
	, m_tradeFrequency(0)
	, m_webSokectFailTimes(0)
	, m_bIsRun(false)
	, m_strTradeVolume(_T(""))
	, m_tradePremiumPrice(0)
	, m_tradePriceDecimal(0)
{
	g_pCoinDlg = this;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCoinDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_listBalance);
	DDX_Control(pDX, IDC_LIST2, m_listCtrlEntrustDepth);
	DDX_Control(pDX, IDC_COMBO1, m_cbMarketType);
	DDX_Text(pDX, IDC_EDIT2, m_tradeFrequency);
	DDX_Control(pDX, IDC_STATIC_UPDATE_TIME, m_staticUpdateTime);
	DDX_Control(pDX, IDC_RADIO1, m_btnHightSpeed);
	DDX_Control(pDX, IDC_RADIO2, m_btnNormalSpeed);
	DDX_Text(pDX, IDC_EDIT1, m_strTradeVolume);
	DDX_Control(pDX, IDC_LIST3, m_listFinishOrder);
	DDX_Text(pDX, IDC_EDIT3, m_tradePremiumPrice);
	DDX_Text(pDX, IDC_EDIT4, m_tradePriceDecimal);
}

BEGIN_MESSAGE_MAP(CCoinDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CCoinDlg::OnBnClickedButtonBegin)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_RADIO1, &CCoinDlg::OnBnClickedRadioHightSpeed)
	ON_BN_CLICKED(IDC_RADIO2, &CCoinDlg::OnBnClickedRadioNormalSpeed)
	ON_BN_CLICKED(IDC_BUTTON2, &CCoinDlg::OnBnClickedButtonStop)
END_MESSAGE_MAP()


// CCoinDlg ��Ϣ�������
void com_callbak_open()
{
	g_pCoinDlg->m_btnHightSpeed.SetWindowTextA("��������(����)");
};
void com_callbak_close()
{
	g_pCoinDlg->m_btnHightSpeed.SetWindowTextA("��������(�����ж�, ������)");
};
void com_callbak_Fail()
{
	g_pCoinDlg->m_webSokectFailTimes++;
	CString temp;
	temp.Format("��������(����ʧ��%d, �ٴγ�����)", g_pCoinDlg->m_webSokectFailTimes);
	g_pCoinDlg->m_btnHightSpeed.SetWindowTextA(temp.GetBuffer());
}

void local_websocket_callbak_message(eWebsocketAPIType apiType, Json::Value& retObj, const std::string& strRet)
{
	switch(apiType)
	{
	case eWebsocketAPIType_EntrustDepth:
		g_pCoinDlg->UpdateEntrustDepth();
		break;
	}
	OutputDebugString(strRet.c_str());
	OutputDebugString("\n");
};

void local_http_callbak_message(eHttpAPIType apiType, Json::Value& retObj, const std::string& strRet, int customData)
{
	switch(apiType)
	{
	case eHttpAPIType_Balance:
		g_pCoinDlg->UpdateBalance();
		g_pCoinDlg->SetTimer(eTimerType_Balance, 5000, NULL);
		return;
		break;
	case eHttpAPIType_Ticker:
		break;
	case eHttpAPIType_EntrustDepth:
		g_pCoinDlg->UpdateEntrustDepth();
		return;
		break;
	case eHttpAPIType_Trade:
		break;
	case eHttpAPIType_Max:
		break;
	case eHttpAPIType_TradeOrderState:
		g_pCoinDlg->UpdateFinishOrder();
		break;
	default:
		break;
	}
	OutputDebugString(strRet.c_str());
	OutputDebugString("\n");
};

CExchange* pExchange = NULL;
/*
void CALLBACK UpdateFunc(UINT wTimerID, UINT msg, DWORD dwUser, DWORD dwl, DWORD dw2)
{
	CCoinDlg* pDlg = (CCoinDlg*)dwUser;
}*/

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
	if(!m_config.open("./config.ini"))
		return FALSE;
	const char* id = m_config.get("exx", "id", "");
	const char* key = m_config.get("exx", "key", "");
	// TODO:  �ڴ���Ӷ���ĳ�ʼ������
	pExchange = new CExxExchange(id, key);
	pExchange->SetHttpCallBackMessage(local_http_callbak_message);
	pExchange->SetWebSocketCallBackOpen(com_callbak_open);
	pExchange->SetWebSocketCallBackClose(com_callbak_close);
	pExchange->SetWebSocketCallBackFail(com_callbak_Fail);
	pExchange->SetWebSocketCallBackMessage(local_websocket_callbak_message);
	pExchange->Run();

	/*TIMECAPS   tc;
	UINT wTimerRes;
	if(timeGetDevCaps(&tc, sizeof(TIMECAPS)) != TIMERR_NOERROR)//���������һ����ý�嶨ʱ��       
		return FALSE;
	wTimerRes = min(max(tc.wPeriodMin, 1), tc.wPeriodMax);
	timeBeginPeriod(wTimerRes);
	MMRESULT g_wTimerID = timeSetEvent(6, wTimerRes, (LPTIMECALLBACK)UpdateFunc, (DWORD)this, TIME_PERIODIC);
	if(g_wTimerID == 0)
		return FALSE;*/
	
	m_btnHightSpeed.SetCheck(1);
	m_listBalance.InsertColumn(0, "����", LVCFMT_CENTER, 40);
	m_listBalance.InsertColumn(1, "���", LVCFMT_CENTER, 100);
	m_listBalance.InsertColumn(2, "����", LVCFMT_CENTER, 100);
	m_listBalance.InsertColumn(3, "����", LVCFMT_CENTER, 100);
	m_listBalance.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	
	m_listCtrlEntrustDepth.InsertColumn(0, "", LVCFMT_CENTER, 30);
	m_listCtrlEntrustDepth.InsertColumn(1, "��", LVCFMT_CENTER, 100);
	m_listCtrlEntrustDepth.InsertColumn(2, "��", LVCFMT_CENTER, 100);
	m_listCtrlEntrustDepth.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

	
	m_listFinishOrder.InsertColumn(0, "ʱ��", LVCFMT_CENTER, 100);
	m_listFinishOrder.InsertColumn(1, "��", LVCFMT_CENTER, 100);
	m_listFinishOrder.InsertColumn(2, "��", LVCFMT_CENTER, 100);
	m_listFinishOrder.InsertColumn(3, "����", LVCFMT_CENTER, 50);
	m_listFinishOrder.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	const std::list<eMarketType>& listMarket = pExchange->GetSupportMarket();
	std::list<eMarketType>::const_iterator itB = listMarket.begin();
	std::list<eMarketType>::const_iterator itE = listMarket.end();
	int index = 0;
	while(itB != itE)
	{
		switch(*itB)
		{
		case eMarketType_ETH_BTC:
			m_cbMarketType.InsertString(index, "ETH/BTC");
			break;
		case eMarketType_ETH_USDT:
			m_cbMarketType.InsertString(index, "ETH/USDT");
			break;
		case eMarketType_BTC_USDT:
			m_cbMarketType.InsertString(index, "BTC/USDT");
			break;
		default:
			break;
		}
		m_cbMarketType.SetItemData(index, *itB);
		++itB;
	}
	

	SetTimer(eTimerType_APIUpdate, 1, NULL);
	SetTimer(eTimerType_Balance, 5000, NULL);
	
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



void CCoinDlg::OnBnClickedButtonBegin()
{
	if(m_bIsRun)
		return;
	UpdateData(TRUE);
	int sel = m_cbMarketType.GetCurSel();
	if(CB_ERR == sel)
	{
		AfxMessageBox("��ѡ���ױҶ�");
		return;
	}
	DWORD type = m_cbMarketType.GetItemData(sel);
	if(type < 0 || type >= eMarketType_Max)
	{
		AfxMessageBox("���ױҶԴ���");
		return;
	}
	if(m_strTradeVolume == "")
	{
		AfxMessageBox("����д���ʽ�����");
		return;
	}
	if(m_tradeFrequency == 0)
	{
		AfxMessageBox("����д����Ƶ��");
		return;
	}
	if(m_tradePremiumPrice == 0)
	{
		AfxMessageBox("����д�µ����");
		return;
	}
	if(m_tradePriceDecimal == 0)
	{
		AfxMessageBox("����д�۸�С��λ����");
		return;
	}
	if(m_btnHightSpeed.GetCheck() && !pExchange->GetWebSocket()->IsConnect())
	{
		AfxMessageBox("��ȴ��뽻������������������!");
		return;
	}
	if(m_btnHightSpeed.GetCheck())
	{
		if(pExchange->GetWebSocket())
			pExchange->GetWebSocket()->API_EntrustDepth((eMarketType)type, 5, true);
	}
	else
	{
		SetTimer(eTimerType_EntrustDepth, 500, NULL);
	}
	SetTimer(eTimerType_Trade, m_tradeFrequency, NULL);
	SetTimer(eTimerType_TradeOrderState, 1000, NULL);
	m_bIsRun = true;
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
}

void CCoinDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO:  �ڴ������Ϣ�����������/�����Ĭ��ֵ
	switch(nIDEvent)
	{
	case eTimerType_APIUpdate:
		{
			pExchange->Update();
		}
		break;
	case eTimerType_Balance:
		{
			if(pExchange->GetHttp())
				pExchange->GetHttp()->API_Balance();
		}
		break;
	case eTimerType_EntrustDepth:
		{
			int sel = m_cbMarketType.GetCurSel();
			DWORD type = m_cbMarketType.GetItemData(sel);
			if(pExchange->GetHttp())
				pExchange->GetHttp()->API_EntrustDepth((eMarketType)type);
		}
		break;
	case eTimerType_Trade:
		{
			int sel = m_cbMarketType.GetCurSel();
			DWORD type = m_cbMarketType.GetItemData(sel);
			CDataCenter* pDataCenter = pExchange->GetDataCenter();
			if(pDataCenter && pDataCenter->m_mapBuyEntrustDepth.size() && pDataCenter->m_mapSellEntrustDepth.size())
			{
				double buyPrice, sellPrice;
				sellPrice = atof(pDataCenter->m_mapSellEntrustDepth.begin()->first.c_str());
				std::map<std::string, std::string>::iterator it = pDataCenter->m_mapBuyEntrustDepth.end();
				it--;
				buyPrice = atof(it->first.c_str());
				if(buyPrice < sellPrice)
				{
					double offset = sellPrice - buyPrice;
					int rate = (int)(offset / m_tradePremiumPrice);
					rate /= 2;
					rate = max(rate, 1);
					buyPrice += (m_tradePremiumPrice*rate);
					CString szPrice = CFuncCommon::Double2String(buyPrice, m_tradePriceDecimal).c_str();
					if(pExchange->GetTradeHttp())
						pExchange->GetTradeHttp()->API_Trade((eMarketType)type, m_strTradeVolume.GetBuffer(), szPrice.GetBuffer(), true, g_trade_pair_index);
					if(pExchange->GetTradeHttp())
						pExchange->GetTradeHttp()->API_Trade((eMarketType)type, m_strTradeVolume.GetBuffer(), szPrice.GetBuffer(), false, g_trade_pair_index);
					g_trade_pair_index++;
				}
				KillTimer(eTimerType_Trade);
			}
		}
		break;
	case eTimerType_TradeOrderState:
		{
			int sel = m_cbMarketType.GetCurSel();
			DWORD type = m_cbMarketType.GetItemData(sel);
			time_t tNow = time(NULL);
			CDataCenter* pDataCenter = pExchange->GetDataCenter();
			std::map<std::string, CDataCenter::SOrderInfo>::iterator itB = pDataCenter->m_mapTradeOrderID.begin();
			std::map<std::string, CDataCenter::SOrderInfo>::iterator itE = pDataCenter->m_mapTradeOrderID.end();
			int sendCnt = 0;
			bool bFound = false;
			while(itB != itE)
			{
				if(itB->second.checkIndex != pDataCenter->m_orderCheckIndex)
				{
					if(itB->second.serverCreatDate != 0 || (tNow - itB->second.addTime > 2))
					{
						bFound = true;
						itB->second.checkIndex = pDataCenter->m_orderCheckIndex;
						if(pExchange->GetHttp())
							pExchange->GetHttp()->API_GetTradeOrderState((eMarketType)type, itB->first);
						if(++sendCnt > CHECK_STATE_RATE)
							break;;
					}
				}
				++itB;
			}
			if(!bFound)
				pDataCenter->m_orderCheckIndex++;
		}
		break;
	default:
		break;
	}
	
	CDialogEx::OnTimer(nIDEvent);
}

void CCoinDlg::UpdateBalance()
{
	m_listBalance.DeleteAllItems();
	CDataCenter* pDataCenter = pExchange->GetDataCenter();
	std::map<std::string, SBalanceInfo>::iterator itB = pDataCenter->m_mapBalanceInfo.begin();
	std::map<std::string, SBalanceInfo>::iterator itE = pDataCenter->m_mapBalanceInfo.end();
	int index = 0;
	CString szFormat;
	while(itB != itE)
	{
		m_listBalance.InsertItem(index, "");
		szFormat.Format("%s", itB->first.c_str());
		m_listBalance.SetItemText(index, 0, szFormat);
		szFormat.Format("%lf", itB->second.total);
		m_listBalance.SetItemText(index, 1, szFormat);
		szFormat.Format("%lf", itB->second.balance);
		m_listBalance.SetItemText(index, 2, szFormat);
		szFormat.Format("%lf", itB->second.freeze);
		m_listBalance.SetItemText(index, 3, szFormat);
		index++;
		++itB;
	}
}

void CCoinDlg::UpdateEntrustDepth()
{
	const int showLines = 5;
	m_listCtrlEntrustDepth.DeleteAllItems();
	CDataCenter* pDataCenter = pExchange->GetDataCenter();
	int sellLine = min(pDataCenter->m_mapSellEntrustDepth.size(), showLines);
	for(int i = 0; i < sellLine; ++i)
	{
		m_listCtrlEntrustDepth.InsertItem(i, "");
	}
	std::map<std::string, std::string>::iterator itB = pDataCenter->m_mapSellEntrustDepth.begin();
	std::map<std::string, std::string>::iterator itE = pDataCenter->m_mapSellEntrustDepth.end();
	CString szFormat;
	int count = 0;
	while(itB != itE)
	{
		szFormat.Format("%d", count + 1);
		m_listCtrlEntrustDepth.SetItemText(sellLine - 1 - count, 0, szFormat);
		szFormat.Format("%s", itB->first.c_str());
		m_listCtrlEntrustDepth.SetItemText(sellLine - 1 - count, 1, szFormat);
		szFormat.Format("%s", itB->second.c_str());
		m_listCtrlEntrustDepth.SetItemText(sellLine - 1 - count, 2, szFormat);
		if(++count >= sellLine)
			break;
		++itB;
	}
	m_listCtrlEntrustDepth.InsertItem(sellLine, "");
	m_listCtrlEntrustDepth.SetItemText(sellLine, 0, "---");
	m_listCtrlEntrustDepth.SetItemText(sellLine, 1, "-------------");
	m_listCtrlEntrustDepth.SetItemText(sellLine, 2, "-------------");
	int buyLine = min(pDataCenter->m_mapBuyEntrustDepth.size(), showLines);
	for(int i = 0; i < sellLine; ++i)
	{
		m_listCtrlEntrustDepth.InsertItem(sellLine+1+i, "");
	}
	if(buyLine > 0)
	{
		std::map<std::string, std::string>::iterator itB = pDataCenter->m_mapBuyEntrustDepth.begin();
		std::map<std::string, std::string>::iterator itE = pDataCenter->m_mapBuyEntrustDepth.end();
		CString szFormat;
		int count = 0;
		itE--;
		while(itB != itE)
		{
			szFormat.Format("%d", count + 1);
			m_listCtrlEntrustDepth.SetItemText(sellLine+1+count, 0, szFormat);
			szFormat.Format("%s", itE->first.c_str());
			m_listCtrlEntrustDepth.SetItemText(sellLine+1+count, 1, szFormat);
			szFormat.Format("%s", itE->second.c_str());
			m_listCtrlEntrustDepth.SetItemText(sellLine+1+count, 2, szFormat);
			if(++count >= buyLine)
				break;
			itE--;
		}
		if(count < buyLine)
		{
			szFormat.Format("%d", count + 1);
			m_listCtrlEntrustDepth.SetItemText(sellLine + 1 + count, 0, szFormat);
			szFormat.Format("%s", itB->first.c_str());
			m_listCtrlEntrustDepth.SetItemText(sellLine + 1 + count, 1, szFormat);
			szFormat.Format("%s", itB->second.c_str());
			m_listCtrlEntrustDepth.SetItemText(sellLine + 1 + count, 2, szFormat);
		}
	}
	tm* pTM = localtime(&pDataCenter->m_updateEntrustDepthTime);
	szFormat.Format("����ʱ��: %d:%d:%d", pTM->tm_hour, pTM->tm_min, pTM->tm_sec);
	m_staticUpdateTime.SetWindowTextA(szFormat.GetBuffer());
}


void CCoinDlg::OnBnClickedRadioHightSpeed()
{
	m_btnNormalSpeed.SetCheck(0);
}


void CCoinDlg::OnBnClickedRadioNormalSpeed()
{
	m_btnHightSpeed.SetCheck(0);
}


void CCoinDlg::OnBnClickedButtonStop()
{
	if(m_bIsRun)
	{
		int sel = m_cbMarketType.GetCurSel();
		DWORD type = m_cbMarketType.GetItemData(sel);
		if(m_btnHightSpeed.GetCheck())
		{
			if(pExchange->GetWebSocket())
				pExchange->GetWebSocket()->API_EntrustDepth((eMarketType)type, 5, false);
		}
		else
		{
			g_pCoinDlg->KillTimer(eTimerType_EntrustDepth);
		}
		g_pCoinDlg->KillTimer(eTimerType_Trade);
		g_pCoinDlg->KillTimer(eTimerType_TradeOrderState);
		m_bIsRun = false;
	}
}

void CCoinDlg::UpdateFinishOrder()
{
	CDataCenter* pDataCenter = pExchange->GetDataCenter();
	if(pDataCenter->m_bJustUpdateFinishOrder)
	{
		if(m_listCtrlEntrustDepth.GetItemCount() > 500)
			m_listCtrlEntrustDepth.DeleteAllItems();
		pDataCenter->m_bJustUpdateFinishOrder = false;
		std::list<CDataCenter::SFinishOrderInfo>::iterator itB = pDataCenter->m_listAllFinishOrder.begin();
		std::list<CDataCenter::SFinishOrderInfo>::iterator itE = pDataCenter->m_listAllFinishOrder.end();
		CString szTemp;
		while(itB != itE)
		{
			m_listFinishOrder.InsertItem(0, "");
			time_t tTime = itB->time/1000;
			tm* pTM = localtime(&tTime);
			szTemp.Format("%02d-%02d %02d:%02d:%02d", pTM->tm_mon+1, pTM->tm_mday, pTM->tm_hour, pTM->tm_min, pTM->tm_sec);
			m_listCtrlEntrustDepth.SetItemText(0, 0, szTemp.GetBuffer());
			szTemp.Format("%lf", itB->price);
			m_listCtrlEntrustDepth.SetItemText(0, 1, szTemp.GetBuffer());
			szTemp.Format("%lf", itB->amount);
			m_listCtrlEntrustDepth.SetItemText(0, 2, szTemp.GetBuffer());
			m_listCtrlEntrustDepth.SetItemText(0, 3, itB->type.c_str());
			++itB;
		}
		pDataCenter->m_listAllFinishOrder.clear();
	}
}