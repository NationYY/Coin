#include "stdafx.h"
#include "bw_exchange.h"
#include "exchange/bw/bw_http_api.h"
#include "exchange/bw/bw_websocket_api.h"

CBWExchange::CBWExchange(std::string strAPIKey, std::string strSecretKey)
{
	m_pWebSocketAPI = new CBWWebsocketAPI(strAPIKey, strSecretKey);
	m_pWebSocketAPI->SetExchange(this);
	m_pHttpAPI = new CBWHttpAPI(strAPIKey, strSecretKey);
	m_pHttpAPI->SetExchange(this);
	m_listSupportMarket.push_back(eMarketType_ETH_BTC);
	m_listSupportMarket.push_back(eMarketType_ETH_USDT);
	m_listSupportMarket.push_back(eMarketType_BTC_USDT);
	m_listSupportMarket.push_back(eMarketType_BWB_USDT);
	m_listSupportMarket.push_back(eMarketType_BWB_BTC);
}


CBWExchange::~CBWExchange()
{
}

void CBWExchange::OnWebsocketResponse(const char* szExchangeName, Json::Value& retObj, const std::string& strRet)
{
	if(retObj.isArray() && retObj[0].isArray() && retObj[0][0].isString() && retObj[0][0].asString() == "AE")
	{
		m_dataCenter.ClearAllEntrustDepth();
		if(retObj[0].size() > 5)
		{
			int updateTime = 0;
			std::string marketName = "";
			if(retObj[0][3].isString())
				updateTime = atoi(retObj[0][3].asString().c_str());
			if(retObj[0][2].isString())
				marketName = retObj[0][2].asString();
			Json::Value info = retObj[0][4];
			if(info["asks"].isArray())
			{
				for(int i = 0; i < (int)info["asks"].size(); ++i)
				{
					std::string price = info["asks"][i][0].asString();
					std::string volume = info["asks"][i][1].asString();
					m_dataCenter.UpdateSellEntrustDepth(price, volume, updateTime, marketName);
				}
			}
			info = retObj[0][5];
			if(info["bids"].isArray())
			{
				for(int i = 0; i < (int)info["bids"].size(); ++i)
				{
					std::string price = info["bids"][i][0].asString();
					std::string volume = info["bids"][i][1].asString();
					m_dataCenter.UpdateBuyEntrustDepth(price, volume, updateTime, marketName);
				}
			}
		}
		if(m_webSocketCallbakMessage)
			m_webSocketCallbakMessage(eWebsocketAPIType_EntrustDepth, szExchangeName, retObj, strRet);
	}
	else if(retObj.isArray() && retObj[0].isString() && retObj[0].asString() == "E")
	{
		if(retObj.size() > 6 && retObj[4].isString() && retObj[5].isString() && retObj[6].isString())
		{
			int updateTime = 0;
			std::string marketName = "";
			if(retObj[2].isString())
				updateTime = atoi(retObj[2].asString().c_str());
			if(retObj[3].isString())
				marketName = retObj[3].asString();
			std::string type = retObj[4].asString();
			std::string price = retObj[5].asString();
			std::string volume = retObj[6].asString();
			if(type == "BID")
			{
				if(volume == "0")
					m_dataCenter.DelBuyEntrustDepth(price, updateTime, marketName);
				else
					m_dataCenter.UpdateBuyEntrustDepth(price, volume, updateTime, marketName);
			}
			else if(type == "ASK")
			{
				if(volume == "0")
					m_dataCenter.DelSellEntrustDepth(price, updateTime, marketName);
				else
					m_dataCenter.UpdateSellEntrustDepth(price, volume, updateTime, marketName);
			}
		}
		if(m_webSocketCallbakMessage)
			m_webSocketCallbakMessage(eWebsocketAPIType_EntrustDepth, szExchangeName, retObj, strRet);
	}
}

void CBWExchange::OnHttpResponse(eHttpAPIType type, Json::Value& retObj, const std::string& strRet, int customData, std::string strCustomData)
{
	CExchange::OnHttpResponse(type, retObj, strRet, customData, strCustomData);
}
