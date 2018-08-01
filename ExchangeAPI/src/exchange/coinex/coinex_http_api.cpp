#include "stdafx.h"
#include "coinex_http_api.h"
#include "exchange/exchange.h"

CCoinexHttpAPI::CCoinexHttpAPI(std::string strAPIKey, std::string strSecretKey, std::string strContentType)
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
	RequestAsync(info);
}

void CCoinexHttpAPI::API_Balance()
{
	SHttpReqInfo info;
	info.apiType = eHttpAPIType_Balance;
	info.reqType = eHttpReqType_Get;
	info.strMethod = "balance/info";
	info.confirmationType = eHttpConfirmationType_Coinex;
	info.mapParams["access_id"] = SHttpParam(eHttpParamType_String, m_strAPIKey);
	char szBuffer[128];
	_snprintf(szBuffer, 128, "%lld", time(NULL)*1000);
	info.mapParams["tonce"] = SHttpParam(eHttpParamType_Int, szBuffer);
	RequestAsync(info);
}