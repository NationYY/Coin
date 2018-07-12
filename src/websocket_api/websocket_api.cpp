#include "stdafx.h"
#include "websocket_api.h"


CWebSocketAPI::CWebSocketAPI() : pWebsocket(NULL)
{
}


CWebSocketAPI::~CWebSocketAPI()
{
}

void CWebSocketAPI::SetKey(string api_key, string secret_key)
{
	m_api_key = api_key;
	m_secret_key = secret_key;
}

void CWebSocketAPI::SetUri(string uri)
{
	m_uri = uri;
}

void CWebSocketAPI::Request(const char* requestInfo)
{

	if(pWebsocket)
	{
		pWebsocket->request(requestInfo);
	}
}

void CWebSocketAPI::Close()
{
	if(pWebsocket)
	{
		pWebsocket->doclose();
		WaitForSingleObject(hThread, INFINITE);
		if(NULL != hThread)
		{
			CloseHandle(hThread);
		}
	}
}

void CWebSocketAPI::SetCallBackOpen(websocketpp_callbak_open callbak_open)
{
	m_callbak_open = callbak_open;
}

void CWebSocketAPI::SetCallBackClose(websocketpp_callbak_close callbak_close)
{
	m_callbak_close = callbak_close;
}

void CWebSocketAPI::SetCallBackMessage(websocketpp_callbak_message callbak_message)
{
	m_callbak_message = callbak_message;
}

unsigned __stdcall CWebSocketAPI::RunThread(LPVOID arg)
{
	if(arg != 0)
	{
		CWebSocketAPI *api = (CWebSocketAPI *)arg;

		for(int i = 0; i < MAX_RETRY_COUNT; i++)
		{
			if(api->pWebsocket)
			{
				api->pWebsocket->doclose();
			}

			if(api->pWebsocket == NULL)
			{
				api->pWebsocket = new WebSocket();
			}

			if(api->pWebsocket)
			{
				api->pWebsocket->callbak_open = api->m_callbak_open;
				api->pWebsocket->callbak_close = api->m_callbak_close;
				api->pWebsocket->callbak_message = api->m_callbak_message;
				api->pWebsocket->run(api->m_uri);
				bool bManualClose = api->pWebsocket->m_manual_close;
				delete api->pWebsocket;
				api->pWebsocket = NULL;
				if(bManualClose == false)//是否为主动关闭连接，如果不是用户主动关闭，当接到断开联接回调时则自动执行重新连接机制。
				{
					Sleep(2000);
				}
				else
				{
					return 0;
				}
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
	hThread = (HANDLE)_beginthreadex(NULL, 0, CWebSocketAPI::RunThread, this, 0, &threadId);
}