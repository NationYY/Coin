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

void CBWHttpAPI::API_Balance()
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

