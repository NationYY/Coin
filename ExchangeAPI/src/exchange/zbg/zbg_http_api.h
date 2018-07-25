#pragma once
#include "http_api/http_api.h"
class CZbgHttpAPI: public CHttpAPI
{
public:
	CZbgHttpAPI(std::string strAPIKey, std::string strSecretKey, std::string strContentType);
	~CZbgHttpAPI();
public:
	virtual void API_Balance();
	virtual void API_Ticker(eMarketType type);
	virtual void API_EntrustDepth(eMarketType type) {}
	virtual void API_Trade(eMarketType type, std::string strAmount, std::string strPrice, bool bBuy) {}
};

