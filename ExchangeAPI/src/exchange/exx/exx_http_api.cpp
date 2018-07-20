#include "stdafx.h"
#include "exx_http_api.h"
#include "exchange/data_center.h"

CExxHttpAPI::CExxHttpAPI(std::string strAPIKey, std::string strSecretKey, std::string strContentType)
{
	SetKey(strAPIKey, strSecretKey);
	SetContentType(strContentType);
	SetURL("https://trade.exxvip.com/api");
}


CExxHttpAPI::~CExxHttpAPI()
{
}

void CExxHttpAPI::API_Balance()
{
	SHttpReqInfo info;
	info.apiType = eHttpAPIType_Balance;
	info.reqType = eHttpReqType_Get;
	info.strMethod = "getBalance";
	info.confirmationType = eHttpConfirmationType_Signature_HmacSHA512;
	info.mapParams["accesskey"] = SHttpParam(eHttpParamType_String, m_strAPIKey);
	char szBuffer[128];
	_snprintf(szBuffer, 128, "%lld", time(NULL) * 1000);
	info.mapParams["nonce"] = SHttpParam(eHttpParamType_Int, szBuffer);
	PushReqInfo(info);
}

void CExxHttpAPI::API_Ticker(const char* szType)
{
	SHttpReqInfo info;
	info.strURL = "https://api.exxvip.com/data/v1";
	info.apiType = eHttpAPIType_Ticker;
	info.reqType = eHttpReqType_Get;
	info.strMethod = "ticker";
	info.mapParams["currency"] = SHttpParam(eHttpParamType_String, szType);
	PushReqInfo(info);
}

