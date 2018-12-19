#pragma once
#include "exchange/exchange.h"
class CHuobiProExchange : public CExchange
{
public:
	CHuobiProExchange(std::string strAPIKey, std::string strSecretKey);
	virtual ~CHuobiProExchange();
	virtual const char* GetName(){
		return "HuobiPro";
	}
	virtual double GetTakerRate(){
		return 0.002;
	}
	virtual void OnHttpResponse(eHttpAPIType type, Json::Value& retObj, const std::string& strRet, int customData, std::string strCustomData);
	virtual void OnWebsocketResponse(const char* szExchangeName, Json::Value& retObj, const std::string& strRet);
	virtual const char* GetMarketString(eMarketType type, bool bHttp)
	{
		switch(type)
		{
			case eMarketType_ETH_BTC:
				return "ethbtc";
			case eMarketType_ETH_USDT:
				return "ethusdt";
			case eMarketType_BTC_USDT:
				return "btcusdt";
		}
		return NULL;
	}
};

