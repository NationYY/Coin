#include "stdafx.h"
#include "okex_http_api.h"
#ifdef _OPEN_OKEX_
COkexHttpAPI::COkexHttpAPI(std::string strAPIKey, std::string strSecretKey, std::string strPassphrase):
m_futuresAccountInfoByCurrencyIndex(0)
{
	SetKey(strAPIKey, strSecretKey, strPassphrase);
	SetURL("https://www.okex.com");
}


COkexHttpAPI::~COkexHttpAPI()
{
}


void COkexHttpAPI::API_FuturesAccountInfoByCurrency(std::string& currency)
{
	SHttpReqInfo info;
	info.apiType = eHttpAPIType_FuturesAccountInfoByCurrency;
	info.reqType = eHttpReqType_Get;
	info.strMethod = "api/futures/v3/accounts/" + currency;
	info.confirmationType = eHttpConfirmationType_OKEx;
	info.bUTF8 = true;
	info.customData = ++m_futuresAccountInfoByCurrencyIndex;
	RequestAsync(info);
}

void COkexHttpAPI::API_FuturesTrade(eFuturesTradeType tradeType, std::string& strCoinType, std::string& strFuturesCycle, std::string& price, std::string& size, std::string& leverage)
{
	SHttpReqInfo info;
	info.apiType = eHttpAPIType_FuturesTrade;
	info.reqType = eHttpReqType_Post;
	info.strMethod = "api/futures/v3/order";
	info.confirmationType = eHttpConfirmationType_OKEx;
	info.bUTF8 = true;
	info.mapParams["client_oid"] = SHttpParam(eHttpParamType_String, CFuncCommon::ToString(CFuncCommon::GenUUID()));
	char szBuffer[128];
	_snprintf(szBuffer, 128, "%s-USD-%s", strCoinType.c_str(), strFuturesCycle.c_str());
	info.mapParams["instrument_id"] = SHttpParam(eHttpParamType_String, szBuffer);
	switch(tradeType)
	{
	case eFuturesTradeType_OpenBull:
		info.mapParams["type"] = SHttpParam(eHttpParamType_String, "1");
		break;
	case eFuturesTradeType_OpenBear:
		info.mapParams["type"] = SHttpParam(eHttpParamType_String, "2");
		break;
	case eFuturesTradeType_CloseBull:
		info.mapParams["type"] = SHttpParam(eHttpParamType_String, "3");
		break;
	case eFuturesTradeType_CloseBear:
		info.mapParams["type"] = SHttpParam(eHttpParamType_String, "4");
		break;
	default:
		break;
	}
	if(price == "-1")
		info.mapParams["type"] = SHttpParam(eHttpParamType_String, "1");
	else
	{
		info.mapParams["price"] = SHttpParam(eHttpParamType_String, price);
		info.mapParams["type"] = SHttpParam(eHttpParamType_String, "0");
	}
	info.mapParams["size"] = SHttpParam(eHttpParamType_String, size);
	info.mapParams["leverage"] = SHttpParam(eHttpParamType_String, leverage);
	RequestAsync(info);
}

void COkexHttpAPI::API_FuturesServerTime()
{
	SHttpReqInfo info;
	info.apiType = eHttpAPIType_FuturesServerTime;
	info.reqType = eHttpReqType_Get;
	info.strMethod = "api/general/v3/time";
	info.confirmationType = eHttpConfirmationType_OKEx;
	info.bUTF8 = true;
	RequestAsync(info);
}


#endif
