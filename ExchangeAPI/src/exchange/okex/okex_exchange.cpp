#include "stdafx.h"
#include "okex_exchange.h"
#include "exchange/okex/okex_http_api.h"
#include "exchange/okex/okex_websocket_api.h"

COkexExchange::COkexExchange(std::string strAPIKey, std::string strSecretKey)
{
	m_pWebSocketAPI = new COkexWebsocketAPI(strAPIKey, strSecretKey);
	m_pWebSocketAPI->SetExchange(this);
	m_pHttpAPI = new COkexHttpAPI(strAPIKey, strSecretKey);
	m_pHttpAPI->SetExchange(this);
	m_listSupportMarket.push_back(eMarketType_ETH_BTC);
	m_listSupportMarket.push_back(eMarketType_ETH_USDT);
	m_listSupportMarket.push_back(eMarketType_BTC_USDT);
}


COkexExchange::~COkexExchange()
{
}

void COkexExchange::OnWebsocketResponse(const char* szExchangeName, Json::Value& retObj, const std::string& strRet)
{
	if(retObj.isArray() && retObj[0].isObject() && retObj[0]["channel"].isString() && retObj[0]["channel"].asString().find("ok_sub_spot") != std::string::npos && retObj[0]["channel"].asString().find("depth") != std::string::npos)
	{
		Json::Value& data = retObj[0]["data"];
		if(data["bids"].isArray())
		{
			for(int i = 0; i<(int)data["bids"].size(); ++i)
			{
				std::string price = data["bids"][i][0].asString();
				std::string volume = data["bids"][i][1].asString();
				if(volume == "0")
					m_dataCenter.DelBuyEntrustDepth(price, (int)time(NULL));
				else
					m_dataCenter.UpdateBuyEntrustDepth(price, volume, (int)time(NULL));
			}
		}
		if(data["asks"].isArray())
		{
			for(int i = 0; i<(int)data["asks"].size(); ++i)
			{
				std::string price = data["asks"][i][0].asString();
				std::string volume = data["asks"][i][1].asString();
				if(volume == "0")
					m_dataCenter.DelSellEntrustDepth(price, (int)time(NULL));
				else
					m_dataCenter.UpdateSellEntrustDepth(price, volume, (int)time(NULL));
			}
		}
		if(m_webSocketCallbakMessage)
			m_webSocketCallbakMessage(eWebsocketAPIType_EntrustDepth, szExchangeName, retObj, strRet);
	}
}

void COkexExchange::OnHttpResponse(eHttpAPIType type, Json::Value& retObj, const std::string& strRet, int customData, std::string strCustomData)
{
	CExchange::OnHttpResponse(type, retObj, strRet, customData, strCustomData);
}