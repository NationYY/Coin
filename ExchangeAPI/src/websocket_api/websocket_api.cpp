#include "stdafx.h"
#include "websocket_api.h"

CWebsocketAPI::CWebsocketAPI() : m_pWebsocket(NULL), m_hThread(NULL), m_bConnect(false),
m_bUTF8(false), m_pExchange(NULL), m_bGZIP(false), m_bHaveSetURI(false)
{
	m_strAPIKey = m_strURI = m_strSecretKey = m_futuresKlineCheck = m_futuresTickerCheck = "";
}


CWebsocketAPI::~CWebsocketAPI()
{
	if(m_pWebsocket)
		delete m_pWebsocket;
}

void CWebsocketAPI::SetKey(std::string strAPIKey, std::string strSecretKey, std::string strPassphrase)
{
	m_strAPIKey = strAPIKey;
	m_strSecretKey = strSecretKey;
	m_strPassphrase = strPassphrase;
}

void CWebsocketAPI::SetURI(std::string strURI)
{
	m_strURI = strURI;
	if(m_strURI != "")
		m_bHaveSetURI = true;
}

bool CWebsocketAPI::Request(const char* szRequestInfo)
{
	if(!m_bConnect)
		return false;
	if(m_pWebsocket)
	{
		m_pWebsocket->request(szRequestInfo);
		return true;
	}
	return false;
}

void CWebsocketAPI::Ping()
{
	if(!m_bConnect)
		return;
	if(m_pWebsocket)
		m_pWebsocket->Ping();
}

void CWebsocketAPI::Close()
{
	if(m_pWebsocket)
	{
		m_pWebsocket->doclose();
		WaitForSingleObject(m_hThread, INFINITE);
		if(NULL != m_hThread)
			CloseHandle(m_hThread);
	}
}

unsigned __stdcall CWebsocketAPI::RunThread(LPVOID arg)
{
	if(arg != 0)
	{
		CWebsocketAPI *api = (CWebsocketAPI *)arg;

		for(int i = 0; i < MAX_RETRY_COUNT; i++)
		{
			if(api->m_pWebsocket)
				api->m_pWebsocket->doclose();

			if(api->m_pWebsocket == NULL)
				api->m_pWebsocket = new WebSocket();

			if(api->m_pWebsocket)
			{
				api->m_pWebsocket->set_websoket_api(api);
				api->m_pWebsocket->set_utf8(api->GetIsUTF8());
				api->m_pWebsocket->set_gzip(api->GetGZIP());
				api->m_pWebsocket->run(api->m_strURI);
				bool bManualClose = api->m_pWebsocket->m_manual_close;
				delete api->m_pWebsocket;
				api->m_pWebsocket = NULL;
				if(bManualClose == false)//是否为主动关闭连接，如果不是用户主动关闭，当接到断开联接回调时则自动执行重新连接机制。
					Sleep(2000);
				else
					return 0;
			}
			else
			{
				return 0;
			}
		}
	}
	//::SetEvent(Global::g_hExit);

	return 0;
}

void CWebsocketAPI::Run()
{
	unsigned int threadId = 0;
	m_hThread = (HANDLE)_beginthreadex(NULL, 0, CWebsocketAPI::RunThread, this, 0, &threadId);
}

void CWebsocketAPI::PushRet(int type, Json::Value& retObj, const char* szRet)
{
	SWebSocketResponse info;
	info.retObj = retObj;
	info.strRet = szRet;
	info.type = type;
	boost::mutex::scoped_lock sl(m_responseMutex);
	m_queueResponseInfo.push_back(info);
}

void CWebsocketAPI::Update()
{
	if(m_queueResponseInfo.size())
	{
		SWebSocketResponse responseInfo;
		{
			boost::mutex::scoped_lock sl(m_responseMutex);
			if(m_queueResponseInfo.empty())
				return;
			responseInfo = m_queueResponseInfo.front();
			m_queueResponseInfo.pop_front();
		}
		switch(responseInfo.type)
		{
		case 0:
			m_bConnect = false;
			if(m_closeFunc)
				m_closeFunc();
			break;
		case 1:
			m_bConnect = true;
			if(m_openFunc)
				m_openFunc();
			break;
		case 2:
			if(m_messageFunc)
				m_messageFunc(responseInfo.retObj, responseInfo.strRet);
			break;
		case 3:
			m_bConnect = false;
			if(m_failFunc)
				m_failFunc();
		default:
			break;
		}
	}
}