#pragma once
#include "exchange/exchange.h"
class CExxExchange : public CExchange
{
public:
	CExxExchange(std::string strAPIKey, std::string strSecretKey);
	~CExxExchange();
	virtual const char* GetName(){
		return "EXX";
	}
	virtual void OnHttpResponse(eHttpAPIType type, Json::Value& retObj, const std::string& strRet, int customData, std::string strCustomData);
	virtual void OnWebsocketResponse(const char* szExchangeName, Json::Value& retObj, const std::string& strRet);
	virtual const char* GetMarketString(eMarketType type, bool bHttp){
		if(bHttp)
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
		}
		else
		{
			switch(type)
			{
			case eMarketType_ETH_BTC:
				return "ETH_BTC";
			case eMarketType_ETH_USDT:
				return "ETH_USDT";
			case eMarketType_BTC_USDT:
				return "BTC_USDT";
			}
		}
		return NULL;
	}
private:
	void Parse_Balance(Json::Value& retObj, const std::string& strRet);
	void Parse_Ticker(Json::Value& retObj, const std::string& strRet);
	void Parse_EntrustDepth(Json::Value& retObj, const std::string& strRet);
	void Parse_Trade(Json::Value& retObj, const std::string& strRet, std::string strCustomData);
	void Parse_TradeOrderListState(Json::Value& retObj, const std::string& strRet) {}
	void Parse_TradeOrderState(Json::Value& retObj, const std::string& strRet);
	void Parse_CancelTrade(Json::Value& retObj, const std::string& strRet, std::string strCustomData);
};

