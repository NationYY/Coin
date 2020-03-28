#include "stdafx.h"
#include "okex_http_api.h"
#ifdef _OPEN_OKEX_
COkexHttpAPI::COkexHttpAPI(std::string strAPIKey, std::string strSecretKey, std::string strPassphrase):
m_futuresAccountInfoByCurrencyIndex(0), m_SpotAccountInfoByCurrencyIndex(0), m_marginAccountInfoByCurrency(0)
{
	SetKey(strAPIKey, strSecretKey, strPassphrase);
	SetURL("https://www.okex.com");
}


COkexHttpAPI::~COkexHttpAPI()
{
}


void COkexHttpAPI::API_FuturesAccountInfoByCurrency(bool bSync, bool bSwap, std::string& currency, std::string& standardCurrency, SHttpResponse* pResInfo)
{
	SHttpReqInfo info;
	info.apiType = eHttpAPIType_FuturesAccountInfoByCurrency;
	info.reqType = eHttpReqType_Get;
	if(bSwap)
		info.strMethod = "api/swap/v3/" + currency + "-"+standardCurrency+"-SWAP/accounts";
	else
		info.strMethod = "api/futures/v3/accounts/" + currency + "-" + standardCurrency;
	info.confirmationType = eHttpConfirmationType_OKEx;
	info.bUTF8 = true;
	info.customData = ++m_futuresAccountInfoByCurrencyIndex;
	if(bSync)
		RequestAsync(info);
	else
		Request(info, pResInfo);
}

void COkexHttpAPI::API_FuturesTrade(bool bSync, bool bSwap, eFuturesTradeType tradeType, std::string& strCoinType, std::string& standardCurrency, std::string& strFuturesCycle, std::string& price, std::string& size, std::string& leverage, std::string& clientOrderID, SHttpResponse* pResInfo)
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
		_snprintf(szBuffer, 128, "%s-%s-SWAP", strCoinType.c_str(), standardCurrency.c_str());
	else
		_snprintf(szBuffer, 128, "%s-%s-%s", strCoinType.c_str(), standardCurrency.c_str(), strFuturesCycle.c_str());
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
	if(bSync)
		RequestAsync(info);
	else
		Request(info, pResInfo);
}

void COkexHttpAPI::API_FuturesSetLeverage(bool bSync, bool bSwap, std::string& strCoinType, std::string& standardCurrency, std::string& strLeverage, SHttpResponse* pResInfo)
{
	SHttpReqInfo info;
	info.apiType = eHttpAPIType_SetFuturesLeverage;
	info.reqType = eHttpReqType_Post;
	if(bSwap)
	{
		info.strMethod = "api/swap/v3/accounts/" + strCoinType + "-" + standardCurrency + "-SWAP/leverage";
		info.mapParams["leverage"] = SHttpParam(eHttpParamType_String, strLeverage);
		info.mapParams["side"] = SHttpParam(eHttpParamType_String, "3");
	}
	else
	{
		info.strMethod = "api/futures/v3/accounts/" + strCoinType + "-" + standardCurrency + "/leverage";
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

void COkexHttpAPI::API_FuturesOrderInfo(bool bSync, bool bSwap, std::string& strCoinType, std::string& standardCurrency, std::string& strFuturesCycle, std::string& orderID, SHttpResponse* pResInfo)
{
	SHttpReqInfo info;
	info.apiType = eHttpAPIType_FuturesTradeInfo;
	info.reqType = eHttpReqType_Get;
	if(bSwap)
		info.strMethod = "api/swap/v3/orders/" + strCoinType + "-" + standardCurrency + "-SWAP/" + orderID;
	else
		info.strMethod = "api/futures/v3/orders/" + strCoinType + "-" + standardCurrency + "-" + strFuturesCycle + "/" + orderID;
	info.confirmationType = eHttpConfirmationType_OKEx;
	info.bUTF8 = true;
	if(bSync)
		RequestAsync(info);
	else
		Request(info, pResInfo);
}

void COkexHttpAPI::API_FuturesPositionInfo(bool bSync, bool bSwap, std::string& strCoinType, std::string& standardCurrency, std::string& strFuturesCycle, SHttpResponse* pResInfo)
{
	SHttpReqInfo info;
	info.apiType = eHttpAPIType_FuturesPositionInfo;
	info.reqType = eHttpReqType_Get;
	if(bSwap)
		info.strMethod = "api/swap/v3/" + strCoinType + "-" + standardCurrency + "-SWAP/position";
	else
		info.strMethod = "api/futures/v3/" + strCoinType + "-" + standardCurrency + "-" + strFuturesCycle + "/position";
	info.confirmationType = eHttpConfirmationType_OKEx;
	info.bUTF8 = true;
	if(bSync)
		RequestAsync(info);
	else
		Request(info, pResInfo);
}

void COkexHttpAPI::API_FuturesCancelOrder(bool bSync, bool bSwap, std::string& strCoinType, std::string& standardCurrency, std::string& strFuturesCycle, std::string& orderID, SHttpResponse* pResInfo)
{
	SHttpReqInfo info;
	info.apiType = eHttpAPIType_FuturesCancelOrder;
	info.reqType = eHttpReqType_Post;
	if(bSwap)
		info.strMethod = "api/swap/v3/cancel_order/" + strCoinType + "-" + standardCurrency + "-SWAP/" + orderID;
	else
		info.strMethod = "api/futures/v3/cancel_order/" + strCoinType + "-" + standardCurrency + "-" + strFuturesCycle + "/" + orderID;
	info.confirmationType = eHttpConfirmationType_OKEx;
	info.bUTF8 = true;
	if(bSync)
		RequestAsync(info);
	else
		Request(info, pResInfo);
}

void COkexHttpAPI::API_SpotTrade(bool bSync, bool bMargin, std::string& instrumentID, std::string& tradeType, std::string& price, std::string& size, std::string& clientOrderID, SHttpResponse* pResInfo)
{
	SHttpReqInfo info;
	info.apiType = eHttpAPIType_SpotTrade;
	info.reqType = eHttpReqType_Post;
	if(bMargin)
		info.strMethod = "api/margin/v3/orders";
	else
		info.strMethod = "api/spot/v3/orders";
	info.confirmationType = eHttpConfirmationType_OKEx;
	info.bUTF8 = true;
	info.mapParams["client_oid"] = SHttpParam(eHttpParamType_String, clientOrderID);
	if(price == "-1")
		info.mapParams["type"] = SHttpParam(eHttpParamType_String, "market");
	else
		info.mapParams["type"] = SHttpParam(eHttpParamType_String, "limit");
	info.mapParams["side"] = SHttpParam(eHttpParamType_String, tradeType);
	info.mapParams["instrument_id"] = SHttpParam(eHttpParamType_String, instrumentID);
	if(bMargin)
		info.mapParams["margin_trading"] = SHttpParam(eHttpParamType_String, "2");
	else
		info.mapParams["margin_trading"] = SHttpParam(eHttpParamType_String, "1");
	info.mapParams["price"] = SHttpParam(eHttpParamType_String, price);
	info.mapParams["size"] = SHttpParam(eHttpParamType_String, size);
	if(bSync)
		RequestAsync(info);
	else
		Request(info, pResInfo);
}

void COkexHttpAPI::API_SpotOrderInfo(bool bSync, bool bMargin, std::string& instrumentID, std::string& orderID, SHttpResponse* pResInfo)
{
	SHttpReqInfo info;
	info.apiType = eHttpAPIType_SpotTradeInfo;
	info.reqType = eHttpReqType_Get;
	if(bMargin)
		info.strMethod = "api/margin/v3/orders/" + orderID + "?instrument_id=" + instrumentID;
	else
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
	info.customData = ++m_SpotAccountInfoByCurrencyIndex;
	if(bSync)
		RequestAsync(info);
	else
		Request(info, pResInfo);
}

void COkexHttpAPI::API_MarginAccountInfoByCurrency(bool bSync, std::string& strCoinType, std::string strMoneyType, SHttpResponse* pResInfo)
{
	SHttpReqInfo info;
	info.apiType = eHttpAPIType_MarginAccountInfoByCurrency;
	info.reqType = eHttpReqType_Get;
	info.strMethod = "api/margin/v3/accounts/" + strCoinType + "-" + strMoneyType;
	info.confirmationType = eHttpConfirmationType_OKEx;
	info.bUTF8 = true;
	info.customData = ++m_marginAccountInfoByCurrency;
	if(bSync)
		RequestAsync(info);
	else
		Request(info, pResInfo);
}

void COkexHttpAPI::API_SpotCancelOrder(bool bSync, bool bMargin, std::string& instrumentID, std::string& orderID, SHttpResponse* pResInfo)
{
	SHttpReqInfo info;
	info.apiType = eHttpAPIType_SpotCancelOrder;
	info.reqType = eHttpReqType_Post;
	if(bMargin)
		info.strMethod = "api/margin/v3/cancel_orders/" + orderID;
	else
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

void COkexHttpAPI::API_GetFuturesSomeKline(bool bSync, bool bSwap, std::string& strCoinType, std::string& standardCurrency, std::string& strFuturesCycle, std::string& strKlineCycle, std::string& strFrom, std::string& strTo, SHttpResponse* pResInfo)
{
	SHttpReqInfo info;
	info.apiType = eHttpAPIType_GetFuturesSomeKline;
	info.reqType = eHttpReqType_Get;
	if(bSwap)
		info.strMethod = "api/swap/v3/instruments/" + strCoinType + "-" + standardCurrency + "-SWAP/candles";
	else
		info.strMethod = "api/futures/v3/instruments/" + strCoinType + "-" + standardCurrency + "-" + strFuturesCycle + "/candles";
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

void COkexHttpAPI::API_GetSpotSomeKline(bool bSync, std::string& instrumentid, std::string& strKlineCycle, std::string& strFrom, std::string& strTo, SHttpResponse* pResInfo)
{
	SHttpReqInfo info;
	info.apiType = eHttpAPIType_GetSpotSomeKline;
	info.reqType = eHttpReqType_Get;
	info.strMethod = "api/spot/v3/instruments/" + instrumentid + "/candles";
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