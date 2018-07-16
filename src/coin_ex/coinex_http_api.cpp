#include "stdafx.h"
#include "coinex_http_api.h"


CCoinexHttpAPI::CCoinexHttpAPI(string strAPIKey, string strSecretKey, string strContentType)
{
	SetKey(strAPIKey, strSecretKey);
	SetContentType(strContentType);
	SetURL("https://api.coinex.com/v1");
}


CCoinexHttpAPI::~CCoinexHttpAPI()
{
}


void CCoinexHttpAPI::API_market_list()
{
	SHttpReqInfo info;
	info.type = eHttpReqType_Get;
	info.strMethod = "market/list";
	info.strPostParams = "";
	PushReqInfo(info);
}