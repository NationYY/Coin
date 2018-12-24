#pragma once
#ifdef _OPEN_OKEX_
#include "exchange/exchange.h"
class COkexExchange : public CExchange
{
public:
	COkexExchange(std::string strAPIKey, std::string strSecretKey, std::string strPassphrase, bool bFutures);
	virtual ~COkexExchange();
	virtual const char* GetName(){
		return "Okex";
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
			return "eth_btc";
		case eMarketType_ETH_USDT:
			return "eth_usdt";
		case eMarketType_BTC_USDT:
			return "btc_usdt";
		}
		return NULL;
	}
};
#endif

