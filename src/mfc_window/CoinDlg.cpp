
// CoinDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Coin.h"
#include "CoinDlg.h"
#include "afxdialogex.h"
#include <string>
#include "coinex_web_socket_api.h"
#include "coinex_http_api.h"

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
END_MESSAGE_MAP()


// CCoinDlg ��Ϣ�������
void com_callbak_open()
{
	//���������������
	AfxMessageBox("���ӳɹ�");
	//���ӳɹ�����������tick��depth����
	//���⣬�ѽ������������������open�ص�����������:
	//������Ͽ����������Ӻ󴥷����ص����Զ����ͽ�������
	//���Ծ���Ҫ��������Ľ���������ڱ��ص��
	//if(comapi != 0)
	//{
	//	comapi->ok_spotusd_btc_ticker();
	//}
};
void com_callbak_close()
{
	std::cout << "�����Ѿ��Ͽ��� " << std::endl;
};
void com_callbak_message(const char *message)
{
	OutputDebugString(message);
};

void local_http_callbak_message(eHttpAPIType apiType, Json::Value& retObj, const std::string& strRet)
{
	OutputDebugString(strRet.c_str());
};

CCoinexWebSocketAPI* comapi;
CCoinexHttpAPI* pHttpAPI = NULL;
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
	/*char szBuff[256];
	//_snprintf(szBuff, 256, "access_id=2066693ABBAC402A8A0D36E1E96CF326&tonce=%lld&secret_key=DB593CF018164E64BE27C37406722B16C3B4B5D1DC4FBCB9", time(NULL));
	_snprintf(szBuff, 256, "access_id=4DA36FFC61334695A66F8D29020EB589&tonce=1513746038205&secret_key=B51068CF10B34E7789C374AB932696A05E0A629BE7BFC62F");
	clib::string out;
	clib::math::md5(szBuff, strlen(szBuff), out);
	_strupr((char*)out.c_str());
	
	Json::FastWriter writer;
	Json::Value person;
	Json::Value root;
	root["method"] = "server.sign";
	Json::Value params;
	params["access_id"] = "4DA36FFC61334695A66F8D29020EB589";
	params["tonce"] = 1513746038205;
	params["authorization"] = out.c_str();
	root["params"] = params;
	root["id"] = 15;
	std::string json_file = writer.write(root);
	std::string aa;
	curl_post_req("https://api.coinex.com/v1", json_file, aa);
	*/

	//comapi = new CCoinexWebSocketAPI("4836FE0E839B4ABB9541536CAE04FE9E", "65FAB5F4DDBB4EC5ABAF4B34337027758B4430B77971C958");
	//comapi->SetCallBackOpen(com_callbak_open);
	//comapi->SetCallBackClose(com_callbak_close);
	//comapi->SetCallBackMessage(com_callbak_message);
	//comapi->Run();//�������ӷ������߳�


	pHttpAPI = new CCoinexHttpAPI("4836FE0E839B4ABB9541536CAE04FE9E", "65FAB5F4DDBB4EC5ABAF4B34337027758B4430B77971C958", "application/json");
	pHttpAPI->SetCallBackMessage(local_http_callbak_message);
	pHttpAPI->Run(1);
	//person["age"] = root;
	//root.append(person);

	//std::string json_file = writer.write(root);
	//curl_get_req("https://api.coinex.com/v1/market/list", a);
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
	if(pHttpAPI)
		pHttpAPI->API_Balance();
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
}


LRESULT CCoinDlg::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO:  �ڴ����ר�ô����/����û���
	if(pHttpAPI)
		pHttpAPI->Update();
	return CDialogEx::DefWindowProc(message, wParam, lParam);
}
