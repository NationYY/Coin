#include "stdafx.h"
#include "zbg_http_api.h"


CZbgHttpAPI::CZbgHttpAPI(std::string strAPIKey, std::string strSecretKey, std::string strContentType)
{
	SetKey(strAPIKey, strSecretKey);
	SetContentType(strContentType);
	SetURL("https://www.zbg.com");
}


CZbgHttpAPI::~CZbgHttpAPI()
{
}

void CZbgHttpAPI::API_Balance()
{
	SHttpReqInfo info;
	info.apiType = eHttpAPIType_Balance;
	info.reqType = eHttpReqType_Post;
	info.strMethod = "exchange/fund/controller/website/fundcontroller/findbypage";
	info.mapParams["pageSize"] = SHttpParam(eHttpParamType_Int, "100");
	info.mapParams["pageNum"] = SHttpParam(eHttpParamType_Int, "1");
	info.confirmationType = eHttpConfirmationType_Zbg;
	info.bUTF8 = true;
	RequestAsync(info);
}

void CZbgHttpAPI::API_Ticker(eMarketType type)
{

}