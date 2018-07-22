#pragma once
#include "http_api/http_api.h"
class CExxHttpAPI: public CHttpAPI
{
public:
	CExxHttpAPI(std::string strAPIKey, std::string strSecretKey, std::string strContentType);
	~CExxHttpAPI();
	virtual void API_Balance();
	virtual void API_Ticker(const char* szType);
};

