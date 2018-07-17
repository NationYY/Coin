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
	info.apiType = eHttpAPIType_MarketList;
	info.reqType = eHttpReqType_Get;
	info.strMethod = "market/list";
	PushReqInfo(info);
}

void CCoinexHttpAPI::API_balance()
{
	SHttpReqInfo info;
	info.apiType = eHttpAPIType_Balance;
	info.reqType = eHttpReqType_Get;
	info.strMethod = "balance/info";
	info.confirmationType = eHttpConfirmationType_HeaderAuthorization;
	info.mapParams["access_id"] = SHttpParam(eHttpParamType_String, m_strAPIKey);
	char szBuffer[128];
	_snprintf(szBuffer, 128, "%lld", time(NULL)*1000);
	info.mapParams["tonce"] = SHttpParam(eHttpParamType_Int, szBuffer);
	PushReqInfo(info);
}