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
//CFuncCommon::ToString(CFuncCommon::GenUUID())
void COkexHttpAPI::API_FuturesTrade(eFuturesTradeType tradeType, std::string& strCoinType, std::string& strFuturesCycle, std::string& price, std::string& size, std::string& leverage, std::string& clientOrderID)
{
	SHttpReqInfo info;
	info.apiType = eHttpAPIType_FuturesTrade;
	info.reqType = eHttpReqType_Post;
	info.strMethod = "api/futures/v3/order";
	info.confirmationType = eHttpConfirmationType_OKEx;
	info.bUTF8 = true;
	info.mapParams["client_oid"] = SHttpParam(eHttpParamType_String, clientOrderID);
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
		info.mapParams["price"] = SHttpParam(eHttpParamType_String, price);
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

void COkexHttpAPI::API_FuturesOrderInfo(std::string& strCoinType, std::string& strFuturesCycle, std::string& orderID)
{
	SHttpReqInfo info;
	info.apiType = eHttpAPIType_FuturesTradeInfo;
	info.reqType = eHttpReqType_Get;
	info.strMethod = "api/futures/v3/orders/" + strCoinType + "-USD-" + strFuturesCycle + "/" + orderID;
	info.confirmationType = eHttpConfirmationType_OKEx;
	info.bUTF8 = true;
	RequestAsync(info);
}

void COkexHttpAPI::API_FuturesCancelOrder(std::string& strCoinType, std::string& strFuturesCycle, std::string& orderID)
{
	SHttpReqInfo info;
	info.apiType = eHttpAPIType_FuturesCancelOrder;
	info.reqType = eHttpReqType_Post;
	info.strMethod = "api/futures/v3/cancel_order/" + strCoinType + "-USD-" + strFuturesCycle + "/" + orderID;
	info.confirmationType = eHttpConfirmationType_OKEx;
	info.bUTF8 = true;
	RequestAsync(info);
}

void COkexHttpAPI::API_SpotTrade(std::string& instrumentID, eTradeType tradeType, std::string price, std::string size, std::string& clientOrderID)
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
	RequestAsync(info);
}

void COkexHttpAPI::API_SpotOrderInfo(std::string& instrumentID, std::string& orderID, std::string strCustomData)
{
	SHttpReqInfo info;
	info.apiType = eHttpAPIType_SpotTradeInfo;
	info.reqType = eHttpReqType_Get;
	info.strMethod = "api/spot/v3/orders/" + orderID + "?instrument_id=" + instrumentID;
	info.confirmationType = eHttpConfirmationType_OKEx;
	info.bUTF8 = true;
	info.strCustomData = strCustomData;
	RequestAsync(info);
}

void COkexHttpAPI::API_SpotAccountInfoByCurrency(std::string& strMoneyType)
{
	SHttpReqInfo info;
	info.apiType = eHttpAPIType_SpotAccountInfoByCurrency;
	info.reqType = eHttpReqType_Get;
	info.strMethod = "api/spot/v3/accounts/" + strMoneyType;
	info.confirmationType = eHttpConfirmationType_OKEx;
	info.bUTF8 = true;
	RequestAsync(info);
}

void COkexHttpAPI::API_SpotCancelOrder(std::string& instrumentID, std::string& orderID, std::string& clientOrderID)
{
	SHttpReqInfo info;
	info.apiType = eHttpAPIType_SpotCancelOrder;
	info.reqType = eHttpReqType_Post;
	info.strMethod = "api/spot/v3/cancel_orders/" + orderID;
	info.confirmationType = eHttpConfirmationType_OKEx;
	info.bUTF8 = true;
	info.mapParams["client_oid"] = SHttpParam(eHttpParamType_String, clientOrderID);
	info.mapParams["instrument_id"] = SHttpParam(eHttpParamType_String, instrumentID);
	RequestAsync(info);
}

void COkexHttpAPI::API_FuturesInstruments(bool bSync, SHttpResponse& resInfo)
{
	SHttpReqInfo info;
	info.apiType = eHttpAPIType_FuturesInstruments;
	info.reqType = eHttpReqType_Get;
	info.strMethod = "api/futures/v3/instruments";
	info.confirmationType = eHttpConfirmationType_OKEx;
	info.bUTF8 = true;
	if(bSync)
		RequestAsync(info);
	else
		Request(info, resInfo);

}

void COkexHttpAPI::API_SpotInstruments(bool bSync, SHttpResponse& resInfo)
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
		Request(info, resInfo);
}
#endif