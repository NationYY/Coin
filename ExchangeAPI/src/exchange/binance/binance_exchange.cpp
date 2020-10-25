#include "stdafx.h"
#include "binance_exchange.h"
#include "exchange/binance/binance_http_api.h"
#include "exchange/binance/binance_websocket_api.h"
#include "log/local_log.h"

CBinanceExchange::CBinanceExchange(std::string strAPIKey, std::string strSecretKey)
{
	m_pMarketWebSocketAPI = new CBinanceWebsocketAPI(strAPIKey, strSecretKey);
	m_pMarketWebSocketAPI->SetExchange(this);

	m_pAccountWebSocketAPI = new CBinanceWebsocketAPI(strAPIKey, strSecretKey);
	m_pAccountWebSocketAPI->SetExchange(this);

	m_pHttpAPI = new CBinanceHttpAPI(strAPIKey, strSecretKey);
	m_pHttpAPI->SetExchange(this);

	m_pHttpTradeAPI = new CBinanceHttpAPI(strAPIKey, strSecretKey);
	m_pHttpTradeAPI->SetExchange(this);
	m_pHttpTradeAPI->SetURL("https://api.binance.com");
}


CBinanceExchange::~CBinanceExchange()
{
}

void CBinanceExchange::OnHttpResponse(eHttpAPIType type, Json::Value& retObj, const std::string& strRet, int customData, std::string strCustomData)
{
	CExchange::OnHttpResponse(type, retObj, strRet, customData, strCustomData);
}

void CBinanceExchange::OnMarketWebsocketResponse(const char* szExchangeName, Json::Value& retObj, const std::string& strRet)
{
	if(strRet == "ping")
	{
		if(m_marketWebSocketCallbakMessage)
			m_marketWebSocketCallbakMessage(eWebsocketAPIType_Pong, szExchangeName, retObj, strRet);
	}
	else if(retObj.isObject() && retObj["e"].isString() && retObj["e"].asString() == "aggTrade")
	{
		if(m_marketWebSocketCallbakMessage)
			m_marketWebSocketCallbakMessage(eWebsocketAPIType_FuturesTicker, szExchangeName, retObj, strRet);
	}
	else if(retObj.isObject() && retObj["e"].isString() && retObj["e"].asString() == "depthUpdate")
	{
		if(m_marketWebSocketCallbakMessage)
			m_marketWebSocketCallbakMessage(eWebsocketAPIType_FuturesEntrustDepth, szExchangeName, retObj, strRet);
	}
	else
		LOCAL_ERROR("binance unkown market_websokect %s", strRet.c_str());
}

void CBinanceExchange::OnAccountWebsocketResponse(const char* szExchangeName, Json::Value& retObj, const std::string& strRet)
{
	if(retObj.isObject() && retObj["e"].isString() && retObj["e"].asString() == "ORDER_TRADE_UPDATE")
	{
		if(m_accountWebSocketCallbakMessage)
			m_accountWebSocketCallbakMessage(eWebsocketAPIType_FuturesOrderInfo, szExchangeName, retObj, strRet);
	}
	else if(retObj.isObject() && retObj["e"].isString() && retObj["e"].asString() == "ACCOUNT_UPDATE")
	{
	}
	else
		LOCAL_ERROR("binance unkown account_websokect %s", strRet.c_str());
}