#pragma once
#include "http_api/http_api.h"
#ifdef _OPEN_ZBG_
class CZbgHttpAPI: public CHttpAPI
{
public:
	CZbgHttpAPI(std::string strAPIKey, std::string strSecretKey, std::string strContentType);
	~CZbgHttpAPI();
public:
	virtual void API_Balance();
	virtual void API_Ticker(eMarketType type);
	virtual void API_EntrustDepth(eMarketType type) {}
	virtual void API_Trade(eMarketType type, std::string strAmount, std::string strPrice, bool bBuy, int customData, std::string strCustomData) {}
	virtual void API_GetTradeOrderListState(eMarketType type, int page, bool bBuy) {};
	virtual void API_GetTradeOrderState(eMarketType type, std::string strID) {};
	virtual void API_CancelTrade(eMarketType type, std::string strID, std::string strCustomData) {};
};

#endif