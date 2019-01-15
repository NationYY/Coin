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


void COkexHttpAPI::API_FuturesAccountInfoByCurrency(bool bSwap, std::string& currency)
{
	SHttpReqInfo info;
	info.apiType = eHttpAPIType_FuturesAccountInfoByCurrency;
	info.reqType = eHttpReqType_Get;
	if(bSwap)
		info.strMethod = "api/swap/v3/" + currency + "-USD-SWAP/accounts";
	else
		info.strMethod = "api/futures/v3/accounts/" + currency;
	info.confirmationType = eHttpConfirmationType_OKEx;
	info.bUTF8 = true;
	info.customData = ++m_futuresAccountInfoByCurrencyIndex;
	RequestAsync(info);
}

void COkexHttpAPI::API_FuturesTrade(bool bSwap, eFuturesTradeType tradeType, std::string& strCoinType, std::string& strFuturesCycle, std::string& price, std::string& size, std::string& leverage, std::string& clientOrderID)
{
	SHttpReqInfo info;
	info.apiType = eHttpAPIType_FuturesTrade;
	info.reqType = eHttpReqType_Post;
	info.confirmationType = eHttpConfirmationType_OKEx;
	info.bUTF8 = true;
	if(bSwap)
		info.strMethod = "api/swap/v3/order";
	else
		info.strMethod = "api/futures/v3/order";
	
	info.mapParams["client_oid"] = SHttpParam(eHttpParamType_String, clientOrderID);
	char szBuffer[128];
	if(bSwap)
		_snprintf(szBuffer, 128, "%s-USD-SWAP", strCoinType.c_str());
	else
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
		info.mapParams["match_price"] = SHttpParam(eHttpParamType_String, "1");
	else
		info.mapParams["price"] = SHttpParam(eHttpParamType_String, price);
	info.mapParams["size"] = SHttpParam(eHttpParamType_String, size);
	if(!bSwap)
		info.mapParams["leverage"] = SHttpParam(eHttpParamType_String, leverage);
	RequestAsync(info);
}

void COkexHttpAPI::API_FuturesSetLeverage(bool bSync, bool bSwap, std::string& strCoinType, std::string& strLeverage, SHttpResponse* pResInfo)
{
	SHttpReqInfo info;
	info.apiType = eHttpAPIType_SetFuturesLeverage;
	info.reqType = eHttpReqType_Post;
	if(bSwap)
	{
		info.strMethod = "api/swap/v3/accounts/" + strCoinType + "-USD-SWAP/leverage";
		info.mapParams["leverage"] = SHttpParam(eHttpParamType_String, strLeverage);
		info.mapParams["side"] = SHttpParam(eHttpParamType_String, "3");
	}
	else
	{
		info.strMethod = "api/futures/v3/accounts/" + strCoinType + "/leverage";
		info.mapParams["leverage"] = SHttpParam(eHttpParamType_String, strLeverage);
	}
	info.confirmationType = eHttpConfirmationType_OKEx;
	info.bUTF8 = true;
	if(bSync)
		RequestAsync(info);
	else
		Request(info, pResInfo);
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

void COkexHttpAPI::API_FuturesOrderInfo(bool bSync, bool bSwap, std::string& strCoinType, std::string& strFuturesCycle, std::string& orderID, SHttpResponse* pResInfo)
{
	SHttpReqInfo info;
	info.apiType = eHttpAPIType_FuturesTradeInfo;
	info.reqType = eHttpReqType_Get;
	if(bSwap)
		info.strMethod = "api/swap/v3/orders/" + strCoinType + "-USD-SWAP/" + orderID;
	else
		info.strMethod = "api/futures/v3/orders/" + strCoinType + "-USD-" + strFuturesCycle + "/" + orderID;
	info.confirmationType = eHttpConfirmationType_OKEx;
	info.bUTF8 = true;
	if(bSync)
		RequestAsync(info);
	else
		Request(info, pResInfo);
}

void COkexHttpAPI::API_FuturesCancelOrder(bool bSwap, std::string& strCoinType, std::string& strFuturesCycle, std::string& orderID)
{
	SHttpReqInfo info;
	info.apiType = eHttpAPIType_FuturesCancelOrder;
	info.reqType = eHttpReqType_Post;
	if(bSwap)
		info.strMethod = "api/swap/v3/cancel_order/" + strCoinType + "-USD-SWAP/" + orderID;
	else
		info.strMethod = "api/futures/v3/cancel_order/" + strCoinType + "-USD-" + strFuturesCycle + "/" + orderID;
	info.confirmationType = eHttpConfirmationType_OKEx;
	info.bUTF8 = true;
	RequestAsync(info);
}

void COkexHttpAPI::API_SpotTrade(bool bSync, std::string& instrumentID, eTradeType tradeType, std::string price, std::string size, const char* clientOrderID, SHttpResponse* pResInfo)
{
	SHttpReqInfo info;
	info.apiType = eHttpAPIType_SpotTrade;
	info.reqType = eHttpReqType_Post;
	info.strMethod = "api/spot/v3/orders";
	info.confirmationType = eHttpConfirmationType_OKEx;
	info.bUTF8 = true;
	info.mapParams["client_oid"] = SHttpParam(eHttpParamType_String, clientOrderID);
	info.mapParams["type"] = SHttpParam(eHttpParamType_String, "limit");
	if(tradeType == eTradeType_buy)
		info.mapParams["side"] = SHttpParam(eHttpParamType_String, "buy");
	else if(tradeType == eTradeType_sell)
		info.mapParams["side"] = SHttpParam(eHttpParamType_String, "sell");
	info.mapParams["instrument_id"] = SHttpParam(eHttpParamType_String, instrumentID);
	info.mapParams["margin_trading"] = SHttpParam(eHttpParamType_String, "1");
	info.mapParams["price"] = SHttpParam(eHttpParamType_String, price);
	info.mapParams["size"] = SHttpParam(eHttpParamType_String, size);
	if(bSync)
		RequestAsync(info);
	else
		Request(info, pResInfo);
}

void COkexHttpAPI::API_SpotOrderInfo(bool bSync, std::string& instrumentID, std::string& orderID, SHttpResponse* pResInfo)
{
	SHttpReqInfo info;
	info.apiType = eHttpAPIType_SpotTradeInfo;
	info.reqType = eHttpReqType_Get;
	info.strMethod = "api/spot/v3/orders/" + orderID + "?instrument_id=" + instrumentID;
	info.confirmationType = eHttpConfirmationType_OKEx;
	info.bUTF8 = true;
	if(bSync)
		RequestAsync(info);
	else
		Request(info, pResInfo);
}

void COkexHttpAPI::API_SpotAccountInfoByCurrency(bool bSync, std::string& strMoneyType, SHttpResponse* pResInfo)
{
	SHttpReqInfo info;
	info.apiType = eHttpAPIType_SpotAccountInfoByCurrency;
	info.reqType = eHttpReqType_Get;
	info.strMethod = "api/spot/v3/accounts/" + strMoneyType;
	info.confirmationType = eHttpConfirmationType_OKEx;
	info.bUTF8 = true;
	if(bSync)
		RequestAsync(info);
	else
		Request(info, pResInfo);
}

void COkexHttpAPI::API_SpotCancelOrder(bool bSync, std::string& instrumentID, std::string& orderID, SHttpResponse* pResInfo)
{
	SHttpReqInfo info;
	info.apiType = eHttpAPIType_SpotCancelOrder;
	info.reqType = eHttpReqType_Post;
	info.strMethod = "api/spot/v3/cancel_orders/" + orderID;
	info.confirmationType = eHttpConfirmationType_OKEx;
	info.bUTF8 = true;
	info.mapParams["client_oid"] = SHttpParam(eHttpParamType_String, "0");
	info.mapParams["instrument_id"] = SHttpParam(eHttpParamType_String, instrumentID);
	if(bSync)
		RequestAsync(info);
	else
		Request(info, pResInfo);
}

void COkexHttpAPI::API_FuturesInstruments(bool bSync, bool bSwap, SHttpResponse* pResInfo)
{
	SHttpReqInfo info;
	info.apiType = eHttpAPIType_FuturesInstruments;
	info.reqType = eHttpReqType_Get;
	if(bSwap)
		info.strMethod = "api/swap/v3/instruments";
	else
		info.strMethod = "api/futures/v3/instruments";
	info.confirmationType = eHttpConfirmationType_OKEx;
	info.bUTF8 = true;
	if(bSync)
		RequestAsync(info);
	else
		Request(info, pResInfo);
}

void COkexHttpAPI::API_GetFuturesSomeKline(bool bSync, bool bSwap, std::string& strCoinType, std::string& strFuturesCycle, std::string& strKlineCycle, std::string& strFrom, std::string& strTo, SHttpResponse* pResInfo)
{
	SHttpReqInfo info;
	info.apiType = eHttpAPIType_GetFuturesSomeKline;
	info.reqType = eHttpReqType_Get;
	if(bSwap)
		info.strMethod = "api/swap/v3/instruments/" + strCoinType + "-USD-SWAP/candles";
	else
		info.strMethod = "api/futures/v3/instruments/" + strCoinType + "-USD-" + strFuturesCycle + "/candles";
	info.mapParams["start"] = SHttpParam(eHttpParamType_String, strFrom);
	info.mapParams["end"] = SHttpParam(eHttpParamType_String, strTo);
	info.mapParams["granularity"] = SHttpParam(eHttpParamType_String, strKlineCycle);
	info.confirmationType = eHttpConfirmationType_OKEx;
	info.bUTF8 = true;
	if(bSync)
		RequestAsync(info);
	else
		Request(info, pResInfo);
	
}

void COkexHttpAPI::API_SpotInstruments(bool bSync, SHttpResponse* pResInfo)
{
	SHttpReqInfo info;
	info.apiType = eHttpAPIType_SpotInstruments;
	info.reqType = eHttpReqType_Get;
	info.strMethod = "api/spot/v3/instruments";
	info.confirmationType = eHttpConfirmationType_OKEx;
	info.bUTF8 = true;
	if(bSync)
		RequestAsync(info);
	else
		Request(info, pResInfo);
}
#endif