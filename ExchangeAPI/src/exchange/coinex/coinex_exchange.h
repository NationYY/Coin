#pragma once
#include "exchange/exchange.h"
class CCoinexExchange : public CExchange
{
public:
	CCoinexExchange(std::string strAPIKey, std::string strSecretKey);
	~CCoinexExchange();
	virtual const char* GetName(){
		return "Coinex";
	}
	virtual void OnHttpResponse(eHttpAPIType type, Json::Value& retObj, const std::string& strRet, int customData, std::string strCustomData);
	virtual void OnWebsocketResponse(Json::Value& retObj, const std::string& strRet);
	virtual const char* GetMarketString(eMarketType type, bool bHttp)
	{
		switch(type)
		{
			case eMarketType_ETH_BTC:
				return "ETHBTC";
			case eMarketType_ETH_USDT:
				return "ETHUSDT";
			case eMarketType_BTC_USDT:
				return "BTCUSDT";
		}
		return NULL;
	}
private:
};

