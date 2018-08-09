#include "stdafx.h"
#include "bw_http_api.h"


CBWHttpAPI::CBWHttpAPI(std::string strAPIKey, std::string strSecretKey)
{
	SetKey(strAPIKey, strSecretKey);
	SetURL("https://www.BW.com/");
}


CBWHttpAPI::~CBWHttpAPI()
{
}
