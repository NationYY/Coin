#include "stdafx.h"
#include "huobi_pro_exchange.h"
#include "exchange/huobi_pro/huobi_pro_http_api.h"
#include "exchange/huobi_pro/huobi_pro_websocket_api.h"
#include "common/func_common.h"
CHuobiProExchange::CHuobiProExchange(std::string strAPIKey, std::string strSecretKey)
{
	m_pWebSocketAPI = new CHuobiProWebsocketAPI(strAPIKey, strSecretKey);
	m_pWebSocketAPI->SetExchange(this);
	m_pWebSocketAPI->SetGZIP(true);
	m_pHttpAPI = new CHuobiProHttpAPI(strAPIKey, strSecretKey);
	m_pHttpAPI->SetExchange(this);
	m_listSupportMarket.push_back(eMarketType_ETH_BTC);
	m_listSupportMarket.push_back(eMarketType_ETH_USDT);
	m_listSupportMarket.push_back(eMarketType_BTC_USDT);
}


CHuobiProExchange::~CHuobiProExchange()
{
}

void CHuobiProExchange::OnWebsocketResponse(Json::Value& retObj, const std::string& strRet)
{
	if(retObj["pong"].isString() || retObj["ping"].isString())
	{
	}
	else if(retObj["ch"].isString() && retObj["ch"].asString().find("depth") != std::string::npos && retObj["tick"].isObject())
	{
		m_dataCenter.ClearAllEntrustDepth();
		Json::Value& ticks = retObj["tick"];
		if(ticks["bids"].isArray())
		{
			for(int i=0; i<ticks["bids"].size(); ++i)
			{
				double price = ticks["bids"][i][0].asDouble();
				double volume = ticks["bids"][i][1].asDouble();
				m_dataCenter.UpdateBuyEntrustDepth(CFuncCommon::Double2String(price, 6), CFuncCommon::Double2String(volume, 6), (int)time(NULL));
			}
		}
		if(ticks["asks"].isArray())
		{
			for(int i = 0; i < ticks["asks"].size(); ++i)
			{
				double price = ticks["asks"][i][0].asDouble();
				double volume = ticks["asks"][i][1].asDouble();
				m_dataCenter.UpdateSellEntrustDepth(CFuncCommon::Double2String(price, 6), CFuncCommon::Double2String(volume, 6), (int)time(NULL));
			}
		}
		if(m_webSocketCallbakMessage)
			m_webSocketCallbakMessage(eWebsocketAPIType_EntrustDepth, retObj, strRet);
	}
	
}

void CHuobiProExchange::OnHttpResponse(eHttpAPIType type, Json::Value& retObj, const std::string& strRet, int customData, std::string strCustomData)
{
	CExchange::OnHttpResponse(type, retObj, strRet, customData, strCustomData);
}