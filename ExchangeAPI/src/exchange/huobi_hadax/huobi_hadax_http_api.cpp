#include "stdafx.h"
#include "huobi_hadax_http_api.h"


CHuobiHadaxHttpAPI::CHuobiHadaxHttpAPI(std::string strAPIKey, std::string strSecretKey)
{
	SetKey(strAPIKey, strSecretKey);
	SetURL("https://api.hadax.com/v1");
}


CHuobiHadaxHttpAPI::~CHuobiHadaxHttpAPI()
{
}
