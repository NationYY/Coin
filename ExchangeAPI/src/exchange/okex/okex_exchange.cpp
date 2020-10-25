#include "stdafx.h"
#include "okex_exchange.h"
#include "exchange/okex/okex_http_api.h"
#include "exchange/okex/okex_websocket_api.h"
#include "log/local_log.h"
#ifdef _OPEN_OKEX_
COkexExchange::COkexExchange(std::string strAPIKey, std::string strSecretKey, std::string strPassphrase)
{
	m_pMarketWebSocketAPI = new COkexWebsocketAPI(strAPIKey, strSecretKey, strPassphrase);
	m_pMarketWebSocketAPI->SetExchange(this);
	m_pMarketWebSocketAPI->SetGZIP(true);
	m_pHttpAPI = new COkexHttpAPI(strAPIKey, strSecretKey, strPassphrase);
	m_pHttpAPI->SetExchange(this);
	m_listSupportMarket.push_back(eMarketType_ETH_BTC);
	m_listSupportMarket.push_back(eMarketType_ETH_USDT);
	m_listSupportMarket.push_back(eMarketType_BTC_USDT);
}


COkexExchange::~COkexExchange()
{
}

void COkexExchange::OnMarketWebsocketResponse(const char* szExchangeName, Json::Value& retObj, const std::string& strRet)
{  
	if(retObj.isObject() && retObj["table"].isString() && retObj["table"].asString() == "spot/depth")
	{
		if(m_marketWebSocketCallbakMessage)
			m_marketWebSocketCallbakMessage(eWebsocketAPIType_EntrustDepth, szExchangeName, retObj, strRet);
	}
	else if(strRet == "pong")
	{
		if(m_marketWebSocketCallbakMessage)
			m_marketWebSocketCallbakMessage(eWebsocketAPIType_Pong, szExchangeName, retObj, strRet);
	}
	else if(retObj.isObject() && retObj["table"].isString() && retObj["table"].asString() == m_pMarketWebSocketAPI->m_futuresKlineCheck)
	{
		if(m_marketWebSocketCallbakMessage)
			m_marketWebSocketCallbakMessage(eWebsocketAPIType_FuturesKline, szExchangeName, retObj, strRet);
	}
	else if(retObj.isObject() && retObj["table"].isString() && retObj["table"].asString() == m_pMarketWebSocketAPI->m_futuresTickerCheck)
	{
		if(m_marketWebSocketCallbakMessage)
			m_marketWebSocketCallbakMessage(eWebsocketAPIType_FuturesTicker, szExchangeName, retObj, strRet);
	}

	else if(retObj.isObject() && retObj["table"].isString() && retObj["table"].asString() == m_pMarketWebSocketAPI->m_spotKlineCheck)
	{
		if(m_marketWebSocketCallbakMessage)
			m_marketWebSocketCallbakMessage(eWebsocketAPIType_SpotKline, szExchangeName, retObj, strRet);
	}
	else if(retObj.isObject() && retObj["table"].isString() && retObj["table"].asString() == m_pMarketWebSocketAPI->m_spotTickerCheck)
	{
		if(m_marketWebSocketCallbakMessage)
			m_marketWebSocketCallbakMessage(eWebsocketAPIType_SpotTicker, szExchangeName, retObj, strRet);
	}
	else if(retObj.isObject() && retObj["event"].isString() && retObj["event"].asString() == "login" && retObj["success"].isBool() && retObj["success"].asBool())
	{
		if(m_marketWebSocketCallbakMessage)
			m_marketWebSocketCallbakMessage(eWebsocketAPIType_Login, szExchangeName, retObj, strRet);
	}
	else if(retObj.isObject() && retObj["table"].isString() && (retObj["table"].asString() == "futures/order" || retObj["table"].asString() == "swap/order"))
	{
		if(m_marketWebSocketCallbakMessage)
			m_marketWebSocketCallbakMessage(eWebsocketAPIType_FuturesOrderInfo, szExchangeName, retObj, strRet);
	}
	else if(retObj.isObject() && retObj["table"].isString() && (retObj["table"].asString() == "futures/account" || retObj["table"].asString() == "swap/account"))
	{
		if(m_marketWebSocketCallbakMessage)
			m_marketWebSocketCallbakMessage(eWebsocketAPIType_FuturesAccountInfo, szExchangeName, retObj, strRet);
	}
	else if(retObj.isObject() && retObj["table"].isString() && retObj["table"].asString() == "spot/account")
	{
		if(m_marketWebSocketCallbakMessage)
			m_marketWebSocketCallbakMessage(eWebsocketAPIType_SpotAccountInfo, szExchangeName, retObj, strRet);
	}
	else if(retObj.isObject() && retObj["table"].isString() && retObj["table"].asString() == "spot/order")
	{
		if(m_marketWebSocketCallbakMessage)
			m_marketWebSocketCallbakMessage(eWebsocketAPIType_SpotOrderInfo, szExchangeName, retObj, strRet);
	}
	else if(retObj.isObject() && retObj["table"].isString() && (retObj["table"].asString() == "swap/depth" || retObj["table"].asString() == "futures/depth"))
	{
		if(m_marketWebSocketCallbakMessage)
			m_marketWebSocketCallbakMessage(eWebsocketAPIType_FuturesEntrustDepth, szExchangeName, retObj, strRet);
	}
	else if(retObj.isObject() && retObj["table"].isString() && (retObj["table"].asString() == "swap/position" || retObj["table"].asString() == "futures/position"))
	{
		if(m_marketWebSocketCallbakMessage)
			m_marketWebSocketCallbakMessage(eWebsocketAPIType_FuturesPositionInfo, szExchangeName, retObj, strRet);
	}
	else if(retObj.isObject() && retObj["table"].isString() && (retObj["table"].asString() == "swap/depth" || retObj["table"].asString() == "futures/depth"))
	{
		if(m_marketWebSocketCallbakMessage)
			m_marketWebSocketCallbakMessage(eWebsocketAPIType_FuturesEntrustDepth, szExchangeName, retObj, strRet);
	}
	else if(retObj.isObject() && retObj["table"].isString() && retObj["table"].asString() == "spot/trade")
	{
		if(m_marketWebSocketCallbakMessage)
			m_marketWebSocketCallbakMessage(eWebsocketAPIType_SpotTrade, szExchangeName, retObj, strRet);
	}
	else
		LOCAL_ERROR("okex unkown market_websokect %s", strRet.c_str());
}

void COkexExchange::OnHttpResponse(eHttpAPIType type, Json::Value& retObj, const std::string& strRet, int customData, std::string strCustomData)
{
	CExchange::OnHttpResponse(type, retObj, strRet, customData, strCustomData);
}
#endif