#pragma once
#include "exchange/exchange.h"
class CBinanceExchange: public CExchange
{
public:
	CBinanceExchange(std::string strAPIKey, std::string strSecretKey);
	~CBinanceExchange();
	virtual const char* GetName(){
		return "Binance";
	}

	virtual void OnHttpResponse(eHttpAPIType type, Json::Value& retObj, const std::string& strRet, int customData, std::string strCustomData);
	virtual void OnMarketWebsocketResponse(const char* szExchangeName, Json::Value& retObj, const std::string& strRet);
};

