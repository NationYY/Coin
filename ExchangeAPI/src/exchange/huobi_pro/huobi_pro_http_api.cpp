#include "stdafx.h"
#include "huobi_pro_http_api.h"


CHuobiProHttpAPI::CHuobiProHttpAPI(std::string strAPIKey, std::string strSecretKey)
{
	SetKey(strAPIKey, strSecretKey);
	SetURL("https://api.huobi.pro/v1");
}


CHuobiProHttpAPI::~CHuobiProHttpAPI()
{
}
