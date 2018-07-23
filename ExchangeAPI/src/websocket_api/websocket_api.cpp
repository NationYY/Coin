#include "stdafx.h"
#include "websocket_api.h"


CWebSocketAPI::CWebSocketAPI() : m_pWebsocket(NULL), m_hThread(NULL), m_bConnect(false),
 m_bUTF8(false)
{
	m_strAPIKey = m_strURI = m_strSecretKey = "";
}


CWebSocketAPI::~CWebSocketAPI()
{
}

void CWebSocketAPI::SetKey(std::string strAPIKey, std::string strSecretKey)
{
	m_strAPIKey = strAPIKey;
	m_strSecretKey = strSecretKey;
}

void CWebSocketAPI::SetURI(std::string strURI)
{
	m_strURI = strURI;
}

void CWebSocketAPI::Request(const char* szRequestInfo)
{
	if(!m_bConnect)
		return;
	if(m_pWebsocket)
		m_pWebsocket->request(szRequestInfo);
}

void CWebSocketAPI::Close()
{
	if(m_pWebsocket)
	{
		m_pWebsocket->doclose();
		WaitForSingleObject(m_hThread, INFINITE);
		if(NULL != m_hThread)
			CloseHandle(m_hThread);
	}
}

unsigned __stdcall CWebSocketAPI::RunThread(LPVOID arg)
{
	if(arg != 0)
	{
		CWebSocketAPI *api = (CWebSocketAPI *)arg;

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

void CWebSocketAPI::Run()
{
	unsigned int threadId = 0;
	m_hThread = (HANDLE)_beginthreadex(NULL, 0, CWebSocketAPI::RunThread, this, 0, &threadId);
}

void CWebSocketAPI::PushRet(int type, Json::Value& retObj, const char* szRet)
{
	SWebSocketResponse info;
	info.retObj = retObj;
	info.strRet = szRet;
	info.type = type;
	boost::mutex::scoped_lock sl(m_responseMutex);
	m_queueResponseInfo.push_back(info);
}

void CWebSocketAPI::Update()
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
			if(m_failFunc)
				m_failFunc();
		default:
			break;
		}
	}
}