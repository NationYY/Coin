#include "stdafx.h"
#include "coinex_exchange.h"
#include "exchange/coinex/coinex_http_api.h"
#include "exchange/coinex/coinex_websocket_api.h"

CCoinexExchange::CCoinexExchange(std::string strAPIKey, std::string strSecretKey)
{
	m_pWebSocketAPI = new CCoinexWebSocketAPI(strAPIKey, strSecretKey);
	m_pHttpAPI = new CCoinexHttpAPI(strAPIKey, strSecretKey, "");
	m_listSupportMarket.push_back(eMarketType_ETH_BTC);
	m_listSupportMarket.push_back(eMarketType_ETH_USDT);
	m_listSupportMarket.push_back(eMarketType_BTC_USDT);
}


CCoinexExchange::~CCoinexExchange()
{
}

void CCoinexExchange::OnWebsocketResponse(Json::Value& retObj, const std::string& strRet)
{
	if(retObj["method"].isString() && retObj["method"].asString() == "deals.update")
	{
		if(retObj["params"].isArray() && retObj["params"][1].isArray() && retObj["params"][1][0]["price"].isString())
		{
			double price = atof(retObj["params"][1][0]["price"].asString().c_str());
			m_dataCenter.SetLatestExecutedOrderPrice(price);
			if(m_webSocketCallbakMessage)
				m_webSocketCallbakMessage(eWebsocketAPIType_LatestExecutedOrder, retObj, strRet);
		}
	}
}

void CCoinexExchange::OnHttpResponse(eHttpAPIType type, Json::Value& retObj, const std::string& strRet, int customData, std::string strCustomData)
{
	CExchange::OnHttpResponse(type, retObj, strRet, customData, strCustomData);
}