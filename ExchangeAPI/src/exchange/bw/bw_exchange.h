#pragma once
#include "exchange/exchange.h"
#define BWB_ID 89
#define BTC_ID 2
#define USDT_ID 11
#define ETH_ID 5
#define QC_ID 23
#ifdef _OPEN_BW_
class CBWExchange : public CExchange
{
public:
	CBWExchange(std::string strAPIKey, std::string strSecretKey);
	virtual ~CBWExchange();
	virtual const char* GetName(){
		return "BW";
	}
	virtual double GetTakerRate(){
		return 0.001;
	}
	virtual void OnHttpResponse(eHttpAPIType type, Json::Value& retObj, const std::string& strRet, int customData, std::string strCustomData);
	virtual void OnWebsocketResponse(const char* szExchangeName, Json::Value& retObj, const std::string& strRet);

	virtual const char* GetMarketString(eMarketType type, bool bHttp)
	{
		switch(type)
		{
		case eMarketType_ETH_BTC:
			return "ETH_BTC";
		case eMarketType_ETH_USDT:
			return "ETH_USDT";
		case eMarketType_BTC_USDT:
			return "BTC_USDT";
		case eMarketType_BWB_USDT:
			return "BWB_USDT";
		case eMarketType_BWB_BTC:
			return "BWB_BTC";
		case eMarketType_BWB_ETH:
			return "BWB_ETH";
		case eMarketType_BWB_QC:
			return "BWB_QC";
		case eMarketType_USDT_QC:
			return "USDT_QC";
		}
		return NULL;
	}
	virtual int GetMarketID(eMarketType type)
	{
		switch(type)
		{
		case eMarketType_ETH_BTC:
			return 305;
		case eMarketType_ETH_USDT:
			return 280;
		case eMarketType_BTC_USDT:
			return 281;
		case eMarketType_BWB_USDT:
			return 302;
		case eMarketType_BWB_BTC:
			return 301;
		case eMarketType_BWB_ETH:
			return 314;
		case eMarketType_BWB_QC:
			return 316;
		case eMarketType_USDT_QC:
			return 3195;
		}
		return 0;
	}
private:
	void Parse_Balance(Json::Value& retObj, const std::string& strRet);
	void Parse_TradeOrderState(Json::Value& retObj, const std::string& strRet);
	void Parse_Trade(Json::Value& retObj, const std::string& strRet, std::string strCustomData);
	void Parse_CancelTrade(Json::Value& retObj, const std::string& strRet, std::string strCustomData);

};


#endif