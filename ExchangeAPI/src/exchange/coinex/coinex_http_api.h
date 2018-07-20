#pragma once
#include "http_api/http_api.h"
class CCoinexHttpAPI : public CHttpAPI
{
public:
	CCoinexHttpAPI(std::string strAPIKey, std::string strSecretKey, std::string strContentType);
	~CCoinexHttpAPI();
	void API_market_list();
	virtual void API_Balance();
};

