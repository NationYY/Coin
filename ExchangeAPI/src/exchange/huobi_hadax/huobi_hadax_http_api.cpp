#include "stdafx.h"
#include "huobi_hadax_http_api.h"
#ifdef _OPEN_HUOBI_HADAX_

CHuobiHadaxHttpAPI::CHuobiHadaxHttpAPI(std::string strAPIKey, std::string strSecretKey)
{
	SetKey(strAPIKey, strSecretKey);
	SetURL("https://api.hadax.com/v1");
}


CHuobiHadaxHttpAPI::~CHuobiHadaxHttpAPI()
{
}
#endif