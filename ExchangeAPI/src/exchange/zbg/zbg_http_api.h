#pragma once
#include "http_api/http_api.h"
class CZbgHttpAPI: public CHttpAPI
{
public:
	CZbgHttpAPI(std::string strAPIKey, std::string strSecretKey, std::string strContentType);
	~CZbgHttpAPI();
public:
	virtual void API_Balance();
	virtual void API_Ticker(const char* szType);
};

