#include "stdafx.h"
#include "okex_http_api.h"

COkexHttpAPI::COkexHttpAPI(std::string strAPIKey, std::string strSecretKey)
{
	SetKey(strAPIKey, strSecretKey);
	SetURL("https://www.okex.com/api/v1");
}


COkexHttpAPI::~COkexHttpAPI()
{
}
