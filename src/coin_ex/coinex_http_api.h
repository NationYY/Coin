#pragma once
#include "http_api.h"
class CCoinexHttpAPI : public CHttpAPI
{
public:
	CCoinexHttpAPI(string strAPIKey, string strSecretKey, string strContentType);
	~CCoinexHttpAPI();
	void API_market_list();
	void API_Balance();
};

