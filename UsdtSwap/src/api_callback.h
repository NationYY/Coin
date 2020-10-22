#pragma once
#include "exchange/okex/okex_websocket_api.h"
#include "exchange/okex/okex_http_api.h"
#include "exchange/binance/binance_http_api.h"
#include "exchange/binance/binance_websocket_api.h"
extern COkexExchange* pOkexExchange;
extern CBinanceExchange* pBinanceExchange;
extern void Pong();
extern void OnWSConnectSuccess();
extern void OnLoginSuccess();
#define OKEX_WEB_SOCKET ((COkexWebsocketAPI*)pOkexExchange->GetWebSocket())
#define OKEX_HTTP ((COkexHttpAPI*)pOkexExchange->GetHttp())

#define BINANCE_WEB_SOCKET ((CBinanceWebsocketAPI*)pBinanceExchange->GetWebSocket())
#define BINANCE_HTTP ((CBinanceHttpAPI*)pBinanceExchange->GetHttp())
void okex_websocket_callbak_open(const char* szExchangeName)
{
	LOCAL_INFO("连接成功");
	OnWSConnectSuccess();
}

void okex_websocket_callbak_close(const char* szExchangeName)
{
	LOCAL_ERROR("断开连接");
}

void okex_websocket_callbak_fail(const char* szExchangeName)
{
	LOCAL_ERROR("连接失败");
}

void okex_http_callbak_message(eHttpAPIType apiType, Json::Value& retObj, const std::string& strRet, int customData, std::string strCustomData)
{
}

void binance_http_callbak_message(eHttpAPIType apiType, Json::Value& retObj, const std::string& strRet, int customData, std::string strCustomData)
{
	switch(apiType)
	{
	case eHttpAPIType_Ping:
		{
			LOCAL_ERROR("ping: %s", strRet.c_str());
		}
		break;
	case eHttpAPIType_SetFuturesLeverage:
		{
											LOCAL_ERROR("eHttpAPIType_SetFuturesLeverage: %s", strRet.c_str());
		}
		break;
	default:
		break;
	}

}

void okex_websocket_callbak_message(eWebsocketAPIType apiType, const char* szExchangeName, Json::Value& retObj, const std::string& strRet)
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