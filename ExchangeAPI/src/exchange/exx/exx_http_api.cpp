#include "stdafx.h"
#include "exx_http_api.h"
#include "exchange/data_center.h"

CExxHttpAPI::CExxHttpAPI(std::string strAPIKey, std::string strSecretKey, std::string strContentType)
{
	SetKey(strAPIKey, strSecretKey);
	SetContentType(strContentType);
	SetURL("https://trade.exxvip.com/api");
	SetUTF8(true);
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

void CExxHttpAPI::API_Trade(eMarketType type, std::string strAmount, std::string strPrice, bool bBuy)
{
	SHttpReqInfo info;
	info.apiType = eHttpAPIType_Trade;
	info.reqType = eHttpReqType_Get;
	info.strMethod = "order";
	info.confirmationType = eHttpConfirmationType_Exx;
	info.mapParams["accesskey"] = SHttpParam(eHttpParamType_String, m_strAPIKey);
	info.mapParams["amount"] = SHttpParam(eHttpParamType_String, strAmount);
	info.mapParams["currency"] = SHttpParam(eHttpParamType_String, GetMarketString(type));
	char szBuffer[128];
	_snprintf(szBuffer, 128, "%lld", time(NULL) * 1000);
	info.mapParams["nonce"] = SHttpParam(eHttpParamType_Int, szBuffer);
	info.mapParams["price"] = SHttpParam(eHttpParamType_String, strPrice);
	if(bBuy)
		info.mapParams["type"] = SHttpParam(eHttpParamType_String, "buy");
	else
		info.mapParams["type"] = SHttpParam(eHttpParamType_String, "sell");
	RequestAsync(info);
}

void CExxHttpAPI::API_Ticker(eMarketType type)
{
	SHttpReqInfo info;
	info.strURL = "https://api.exxvip.com/data/v1";
	info.apiType = eHttpAPIType_Ticker;
	info.reqType = eHttpReqType_Get;
	info.strMethod = "ticker";
	info.mapParams["currency"] = SHttpParam(eHttpParamType_String, GetMarketString(type));
	RequestAsync(info);
}


void CExxHttpAPI::API_EntrustDepth(eMarketType type)
{
	SHttpReqInfo info;
	info.strURL = "https://api.exxvip.com/data/v1";
	info.apiType = eHttpAPIType_EntrustDepth;
	info.reqType = eHttpReqType_Get;
	info.strMethod = "depth";
	info.mapParams["currency"] = SHttpParam(eHttpParamType_String, GetMarketString(type));
	RequestAsync(info);
}

const char* CExxHttpAPI::GetMarketString(eMarketType type)
{
	switch(type)
	{
	case eMarketType_ETH_BTC:
		return "eth_btc";
	case eMarketType_ETH_USDT:
		return "eth_usdt";
	case eMarketType_BTC_USDT:
		return "btc_usdt";
	}
	return NULL;
}