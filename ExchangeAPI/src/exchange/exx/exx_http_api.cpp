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
	info.confirmationType = eHttpConfirmationType_Exx;
	info.mapParams["accesskey"] = SHttpParam(eHttpParamType_String, m_strAPIKey);
	char szBuffer[128];
	_snprintf(szBuffer, 128, "%lld", time(NULL) * 1000);
	info.mapParams["nonce"] = SHttpParam(eHttpParamType_Int, szBuffer);
	RequestAsync(info);
}

void CExxHttpAPI::API_Ticker(eMarketType type)
{
	SHttpReqInfo info;
	info.strURL = "https://api.exxvip.com/data/v1";
	info.apiType = eHttpAPIType_Ticker;
	info.reqType = eHttpReqType_Get;
	info.strMethod = "ticker";
	switch(type)
	{
	case eMarketType_ETH_BTC:
		info.mapParams["currency"] = SHttpParam(eHttpParamType_String, "eth_btc");
		break;
	case eMarketType_ETH_USDT:
		info.mapParams["currency"] = SHttpParam(eHttpParamType_String, "eth_usdt");
		break;
	case eMarketType_BTC_USDT:
		info.mapParams["currency"] = SHttpParam(eHttpParamType_String, "btc_usdt");
		break;
	default:
		break;
	}
	RequestAsync(info);
}

