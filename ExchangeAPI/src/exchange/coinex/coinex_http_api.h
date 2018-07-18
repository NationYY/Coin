#pragma once
#include "http_api/http_api.h"
class CCoinexHttpAPI : public CHttpAPI
{
public:
	CCoinexHttpAPI(string strAPIKey, string strSecretKey, string strContentType);
	~CCoinexHttpAPI();
	void API_market_list();
	virtual void API_Balance();
};

