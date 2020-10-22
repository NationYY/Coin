#include "stdafx.h"
#include "binance_http_api.h"


CBinanceHttpAPI::CBinanceHttpAPI(std::string strAPIKey, std::string strSecretKey)
{
	SetKey(strAPIKey, strSecretKey);
	SetURL("https://fapi.binance.com");
	
}


CBinanceHttpAPI::~CBinanceHttpAPI()
{
}

void CBinanceHttpAPI::API_Ping()
{
	SHttpReqInfo info;
	info.apiType = eHttpAPIType_Ping;
	info.reqType = eHttpReqType_Get;
	info.strMethod = "fapi/v1/ping";
	info.confirmationType = eHttpConfirmationType_Binance;
	RequestAsync(info);
}


void CBinanceHttpAPI::API_FuturesSetLeverage(bool bSync, std::string& strCoinType, std::string& standardCurrency, int nLeverage, SHttpResponse* pResInfo)
{
	SHttpReqInfo info;
	info.apiType = eHttpAPIType_SetFuturesLeverage;
	info.reqType = eHttpReqType_Post;
	info.strMethod = "/fapi/v1/leverage";
	std::string symbol = strCoinType + standardCurrency;
	info.mapParams["symbol"] = SHttpParam(eHttpParamType_String, symbol);
	info.mapParams["leverage"] = SHttpParam(eHttpParamType_Int, CFuncCommon::ToString(nLeverage));
	time_t tNow = time(NULL);
	info.mapParams["timestamp"] = SHttpParam(eHttpParamType_Int, CFuncCommon::ToString(tNow*1000));
	info.confirmationType = eHttpConfirmationType_Binance;
	info.bSignature = true;
	if(bSync)
		RequestAsync(info);
	else
		Request(info, pResInfo);
}