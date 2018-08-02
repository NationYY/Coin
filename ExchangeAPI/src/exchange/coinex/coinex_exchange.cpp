#include "stdafx.h"
#include "coinex_exchange.h"
#include "exchange/coinex/coinex_http_api.h"
#include "exchange/coinex/coinex_websocket_api.h"

CCoinexExchange::CCoinexExchange(std::string strAPIKey, std::string strSecretKey)
{
	m_pWebSocketAPI = new CCoinexWebSocketAPI(strAPIKey, strSecretKey);
	m_pWebSocketAPI->SetExchange(this);
	m_pHttpAPI = new CCoinexHttpAPI(strAPIKey, strSecretKey);
	m_pHttpAPI->SetExchange(this);
	m_listSupportMarket.push_back(eMarketType_ETH_BTC);
	m_listSupportMarket.push_back(eMarketType_ETH_USDT);
	m_listSupportMarket.push_back(eMarketType_BTC_USDT);
}


CCoinexExchange::~CCoinexExchange()
{
}

void CCoinexExchange::OnWebsocketResponse(const char* szExchangeName, Json::Value& retObj, const std::string& strRet)
{
	if(retObj["method"].isString() && retObj["method"].asString() == "deals.update")
	{
		if(retObj["params"].isArray() && retObj["params"][1].isArray() && retObj["params"][1][0]["price"].isString())
		{
			double price = atof(retObj["params"][1][0]["price"].asString().c_str());
			m_dataCenter.SetLatestExecutedOrderPrice(price);
			if(m_webSocketCallbakMessage)
				m_webSocketCallbakMessage(eWebsocketAPIType_LatestExecutedOrder, szExchangeName, retObj, strRet);
		}
	}
	else if(retObj["method"].isString() && retObj["method"].asString() == "depth.update")
	{
		if(!retObj["params"].isArray())
			return;
		Json::Value& price = retObj["params"][1];
		if(retObj["params"][0].asBool() == true)
		{
			m_dataCenter.ClearAllEntrustDepth();
			if(price["bids"].isArray())
			{
				for(int i=0; i<(int)price["bids"].size(); ++i)
				{
					std::string _price = price["bids"][i][0].asString();
					std::string _volume = price["bids"][i][1].asString();
					m_dataCenter.UpdateBuyEntrustDepth(_price, _volume, (int)time(NULL));
				}
			}
			if(price["asks"].isArray())
			{
				for(int i=0; i <(int)price["asks"].size(); ++i)
				{
					std::string _price = price["asks"][i][0].asString();
					std::string _volume = price["asks"][i][1].asString();
					m_dataCenter.UpdateSellEntrustDepth(_price, _volume, (int)time(NULL));
				}
			}
		}
		else
		{
			if(price["bids"].isArray())
			{
				for(int i=0; i<(int)price["bids"].size(); ++i)
				{
					std::string _price = price["bids"][i][0].asString();
					std::string _volume = price["bids"][i][1].asString();
					if(_volume == "0")
						m_dataCenter.DelBuyEntrustDepth(_price, (int)time(NULL));
					else
						m_dataCenter.UpdateBuyEntrustDepth(_price, _volume, (int)time(NULL));
				}
			}
			if(price["asks"].isArray())
			{
				for(int i=0; i<(int)price["asks"].size(); ++i)
				{
					std::string _price = price["asks"][i][0].asString();
					std::string _volume = price["asks"][i][1].asString();
					if(_volume == "0")
						m_dataCenter.DelSellEntrustDepth(_price, (int)time(NULL));
					else
						m_dataCenter.UpdateSellEntrustDepth(_price, _volume, (int)time(NULL));
				}
			}
		}
		if(m_webSocketCallbakMessage)
			m_webSocketCallbakMessage(eWebsocketAPIType_EntrustDepth, szExchangeName, retObj, strRet);
	}
}

void CCoinexExchange::OnHttpResponse(eHttpAPIType type, Json::Value& retObj, const std::string& strRet, int customData, std::string strCustomData)
{
	CExchange::OnHttpResponse(type, retObj, strRet, customData, strCustomData);
}