#include "stdafx.h"
#include "websocket_api.h"


CWebSocketAPI::CWebSocketAPI() : m_pWebsocket(NULL), m_callbakOpen(NULL), m_callbakClose(NULL),
 m_callbakMessage(NULL), m_hThread(NULL)
{
	m_strAPIKey = m_strURI = m_strSecretKey = "";
}


CWebSocketAPI::~CWebSocketAPI()
{
}

void CWebSocketAPI::SetKey(string strAPIKey, string strSecretKey)
{
	m_strAPIKey = strAPIKey;
	m_strSecretKey = strSecretKey;
}

void CWebSocketAPI::SetURI(string strURI)
{
	m_strURI = strURI;
}

void CWebSocketAPI::Request(const char* szRequestInfo)
{
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

void CWebSocketAPI::SetCallBackOpen(websocketpp_callbak_open callbak0pen)
{
	m_callbakOpen = callbak0pen;
}

void CWebSocketAPI::SetCallBackClose(websocketpp_callbak_close callbakClose)
{
	m_callbakClose = callbakClose;
}

void CWebSocketAPI::SetCallBackMessage(websocketpp_callbak_message callbakMessage)
{
	m_callbakMessage = callbakMessage;
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
				api->m_pWebsocket->callbak_open = api->m_callbakOpen;
				api->m_pWebsocket->callbak_close = api->m_callbakClose;
				api->m_pWebsocket->callbak_message = api->m_callbakMessage;
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