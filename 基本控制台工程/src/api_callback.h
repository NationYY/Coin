#pragma once
#include "exchange/okex/okex_websocket_api.h"
#include "exchange/okex/okex_http_api.h"
extern COkexExchange* pExchange;
extern void Pong();
extern void Listen();
#define OKEX_WEB_SOCKET ((COkexWebsocketAPI*)pExchange->GetWebSocket())
#define OKEX_HTTP ((COkexHttpAPI*)pExchange->GetHttp())

void local_websocket_callbak_open(const char* szExchangeName)
{
	LOCAL_INFO("���ӳɹ�");
	Listen();
}

void local_websocket_callbak_close(const char* szExchangeName)
{
	LOCAL_ERROR("�Ͽ�����");
}

void local_websocket_callbak_fail(const char* szExchangeName)
{
	LOCAL_ERROR("����ʧ��");
}

void local_http_callbak_message(eHttpAPIType apiType, Json::Value& retObj, const std::string& strRet, int customData, std::string strCustomData)
{
}

void local_websocket_callbak_message(eWebsocketAPIType apiType, const char* szExchangeName, Json::Value& retObj, const std::string& strRet)
{
	switch(apiType)
	{
	case eWebsocketAPIType_SpotTrade:
		{
		}
		break;
	case eWebsocketAPIType_Pong:
		{
			 Pong();
		}
		break;
	case eWebsocketAPIType_SpotKline:
		{
		}
		break;
	}
}