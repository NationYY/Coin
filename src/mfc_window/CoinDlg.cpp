
// CoinDlg.cpp : 实现文件
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
// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

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


// CCoinDlg 对话框



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


// CCoinDlg 消息处理程序
void com_callbak_open()
{
	//向服务器发送命令
	AfxMessageBox("连接成功");
	//连接成功后立即接收tick和depth数据
	//另外，把接收行情数据请求放在open回调里作用在于:
	//当意外断开，重新连接后触发本回调可自动发送接收请求。
	//所以尽量要把行情类的接收请求放在本回调里。
	//if(comapi != 0)
	//{
	//	comapi->ok_spotusd_btc_ticker();
	//}
};
void com_callbak_close()
{
	std::cout << "连接已经断开！ " << std::endl;
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

	// TODO:  在此添加额外的初始化代码
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
	//comapi->Run();//启动连接服务器线程


	pHttpAPI = new CCoinexHttpAPI("4836FE0E839B4ABB9541536CAE04FE9E", "65FAB5F4DDBB4EC5ABAF4B34337027758B4430B77971C958", "application/json");
	pHttpAPI->SetCallBackMessage(local_http_callbak_message);
	pHttpAPI->Run(1);
	//person["age"] = root;
	//root.append(person);

	//std::string json_file = writer.write(root);
	//curl_get_req("https://api.coinex.com/v1/market/list", a);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CCoinDlg::OnPaint()
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
	// TODO:  在此添加控件通知处理程序代码
}


LRESULT CCoinDlg::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO:  在此添加专用代码和/或调用基类
	if(pHttpAPI)
		pHttpAPI->Update();
	return CDialogEx::DefWindowProc(message, wParam, lParam);
}
