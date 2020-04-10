#pragma once
#include "CrossPlatformQTDlg.h"
extern CCrossPlatformQTDlg* g_pDlg;
extern CExchange* pExchange;
#define WEB_SOCKET pExchange->GetWebSocket()
#define HTTP pExchange->GetHttp()

void local_http_callbak_message(eHttpAPIType apiType, Json::Value& retObj, const std::string& strRet, int customData, std::string strCustomData)
{
}

void local_websocket_callbak_open(const char* szExchangeName)
{
	LOCAL_INFO("连接成功");
	g_pDlg->m_tListenPong = 0;
}

void local_websocket_callbak_close(const char* szExchangeName)
{
	LOCAL_ERROR("断开连接");
	g_pDlg->m_tListenPong = 0;
}

void local_websocket_callbak_fail(const char* szExchangeName)
{
	LOCAL_ERROR("连接失败");
	g_pDlg->m_tListenPong = 0;
}

void local_websocket_callbak_message(eWebsocketAPIType apiType, const char* szExchangeName, Json::Value& retObj, const std::string& strRet)
{
	switch(apiType)
	{
	case eWebsocketAPIType_Pong:
		{
			g_pDlg->Pong();
		}
		break;
	case eWebsocketAPIType_Login:
		{
			g_pDlg->OnLoginSuccess();
		}
		break;
	}
}