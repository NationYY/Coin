#pragma once
#include "http_api/http_api.h"
class CBinanceHttpAPI : public CHttpAPI
{
public:
	CBinanceHttpAPI(std::string strAPIKey, std::string strSecretKey);
	~CBinanceHttpAPI();

	virtual void API_Ping();
	virtual void API_FuturesSetLeverage(bool bSync, std::string& strCoinType, std::string& standardCurrency, int nLeverage, SHttpResponse* pResInfo = NULL);
};

