#pragma once
#include "http_api/http_api.h"
class CExxHttpAPI: public CHttpAPI
{
public:
	CExxHttpAPI(string strAPIKey, string strSecretKey, string strContentType);
	~CExxHttpAPI();
	virtual void API_Balance();
};

