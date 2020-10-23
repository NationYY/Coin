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

extern void OnBinanceMarketWSConnectSuccess();
extern void OnBinanceAccountWSConnectSuccess();
extern void OnBinanceGotListenKey(std::string key);
#define OKEX_WEB_SOCKET ((COkexWebsocketAPI*)pOkexExchange->GetMarketWebSocket())
#define OKEX_HTTP ((COkexHttpAPI*)pOkexExchange->GetHttp())

#define BINANCE_MARKET_WEB_SOCKET ((CBinanceWebsocketAPI*)pBinanceExchange->GetMarketWebSocket())
#define BINANCE_ACCOUNT_WEB_SOCKET ((CBinanceWebsocketAPI*)pBinanceExchange->GetAccountWebSocket())

#define BINANCE_HTTP ((CBinanceHttpAPI*)pBinanceExchange->GetHttp())
void okex_websocket_callbak_open(const char* szExchangeName)
{
	LOCAL_INFO("Okex websocket连接成功");
	OnOkexWSConnectSuccess();
}

void okex_websocket_callbak_close(const char* szExchangeName)
{
	LOCAL_ERROR("Okex websocket断开连接");
}

void okex_websocket_callbak_fail(const char* szExchangeName)
{
	LOCAL_ERROR("Okex websocket连接失败");
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


void binance_market_websocket_callbak_open(const char* szExchangeName)
{
	LOCAL_INFO("Binance 行情websocket连接成功");
	OnBinanceMarketWSConnectSuccess();
}

void binance_market_websocket_callbak_close(const char* szExchangeName)
{
	LOCAL_ERROR("Binance 行情websocket断开连接");
}

void binance_market_websocket_callbak_fail(const char* szExchangeName)
{
	LOCAL_ERROR("Binance 行情websocket连接失败");
}

void binance_account_websocket_callbak_open(const char* szExchangeName)
{
	LOCAL_INFO("Binance 账户websocket连接成功");
	OnBinanceAccountWSConnectSuccess();
}

void binance_account_websocket_callbak_close(const char* szExchangeName)
{
	LOCAL_ERROR("Binance 账户websocket断开连接");
}

void binance_account_websocket_callbak_fail(const char* szExchangeName)
{
	LOCAL_ERROR("Binance 账户websocket连接失败");
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
			LOCAL_INFO("eHttpAPIType_SetFuturesLeverage: %s", strRet.c_str());
		}
		break;
	case eHttpAPIType_ListenKey:
		{
			if(retObj.isObject() && retObj["listenKey"].isString())
			{
				OnBinanceGotListenKey(retObj["listenKey"].asString());
			}
			else
			{
				LOCAL_ERROR("eHttpAPIType_ListenKey: %s", strRet.c_str());
			}
		}
		break;
	default:
		LOCAL_ERROR("unkown httpret type=%d ret=%s", apiType, strRet.c_str());
		break;
	}

}

void binance_market_websocket_callbak_message(eWebsocketAPIType apiType, const char* szExchangeName, Json::Value& retObj, const std::string& strRet)
{
	switch(apiType)
	{
	case eWebsocketAPIType_Pong:
		{
			BinancePong();
		}
		break;
	default:
		LOCAL_ERROR("unkown market_websocketret type=%d ret=%s", apiType, strRet.c_str());
	}
}

void binance_account_websocket_callbak_message(eWebsocketAPIType apiType, const char* szExchangeName, Json::Value& retObj, const std::string& strRet)
{
	LOCAL_ERROR("unkown account_websocketret type=%d ret=%s", apiType, strRet.c_str());
}