#pragma once
#include "exchange/okex/okex_websocket_api.h"
#include "exchange/okex/okex_http_api.h"
#include "exchange/binance/binance_http_api.h"
#include "exchange/binance/binance_websocket_api.h"
extern COkexExchange* pOkexExchange;
extern CBinanceExchange* pBinanceExchange;
extern void OKexPong();
extern void BinancePong();
extern void OnOkexWSConnectSuccess();
extern void OnOkexWSLoginSuccess();

extern void OnBinanceWSConnectSuccess();
extern void OnBinanceWSLoginSuccess();
#define OKEX_WEB_SOCKET ((COkexWebsocketAPI*)pOkexExchange->GetWebSocket())
#define OKEX_HTTP ((COkexHttpAPI*)pOkexExchange->GetHttp())

#define BINANCE_WEB_SOCKET ((CBinanceWebsocketAPI*)pBinanceExchange->GetWebSocket())
#define BINANCE_HTTP ((CBinanceHttpAPI*)pBinanceExchange->GetHttp())
void okex_websocket_callbak_open(const char* szExchangeName)
{
	LOCAL_INFO("Okex websocket���ӳɹ�");
	OnOkexWSConnectSuccess();
}

void okex_websocket_callbak_close(const char* szExchangeName)
{
	LOCAL_ERROR("Okex websocket�Ͽ�����");
}

void okex_websocket_callbak_fail(const char* szExchangeName)
{
	LOCAL_ERROR("Okex websocket����ʧ��");
}

void okex_http_callbak_message(eHttpAPIType apiType, Json::Value& retObj, const std::string& strRet, int customData, std::string strCustomData)
{
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
								   OKexPong();
	}
		break;
	case eWebsocketAPIType_SpotKline:
	{
	}
		break;
	}
}


void binance_websocket_callbak_open(const char* szExchangeName)
{
	LOCAL_INFO("Binance websocket���ӳɹ�");
	OnOkexWSConnectSuccess();
}

void binance_websocket_callbak_close(const char* szExchangeName)
{
	LOCAL_ERROR("Binance websocket�Ͽ�����");
}

void binance_websocket_callbak_fail(const char* szExchangeName)
{
	LOCAL_ERROR("Binance websocket����ʧ��");
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

void binance_websocket_callbak_message(eWebsocketAPIType apiType, const char* szExchangeName, Json::Value& retObj, const std::string& strRet)
{
	switch(apiType)
	{
	case eWebsocketAPIType_SpotTrade:
		{
		}
		break;
	case eWebsocketAPIType_Pong:
		{
			BinancePong();
		}
		break;
	case eWebsocketAPIType_SpotKline:
		{
		}
		break;
	}
}