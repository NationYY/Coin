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
	info.strMethod = "fapi/v1/leverage";
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

void CBinanceHttpAPI::API_ListenKey(bool bSync, SHttpResponse* pResInfo)
{
	SHttpReqInfo info;
	info.apiType = eHttpAPIType_ListenKey;
	info.reqType = eHttpReqType_Post;
	info.strMethod = "fapi/v1/listenKey";
	info.confirmationType = eHttpConfirmationType_Binance;
	info.bSignature = false;
	if(bSync)
		RequestAsync(info);
	else
		Request(info, pResInfo);
	
}


void CBinanceHttpAPI::API_FuturesAccountInfo(bool bSync, SHttpResponse* pResInfo)
{
	SHttpReqInfo info;
	info.apiType = eHttpAPIType_FuturesAccountInfo;
	info.reqType = eHttpReqType_Get;
	info.strMethod = "/fapi/v2/balance";;
	info.confirmationType = eHttpConfirmationType_Binance;
	info.bSignature = true;
	time_t tNow = time(NULL);
	info.mapParams["timestamp"] = SHttpParam(eHttpParamType_Int, CFuncCommon::ToString(tNow * 1000));

	if(bSync)
		RequestAsync(info);
	else
		Request(info, pResInfo);
}

void CBinanceHttpAPI::API_FuturesTrade(bool bSync, eFuturesTradeType tradeType, std::string& strCoinType, std::string& standardCurrency, std::string& price, std::string& size, std::string& clientOrderID, SHttpResponse* pResInfo)
{
	SHttpReqInfo info;
	info.apiType = eHttpAPIType_FuturesTrade;
	info.reqType = eHttpReqType_Post;
	info.strMethod = "/fapi/v1/order";
	info.confirmationType = eHttpConfirmationType_Binance;
	info.bSignature = true;
	time_t tNow = time(NULL);
	info.mapParams["timestamp"] = SHttpParam(eHttpParamType_Int, CFuncCommon::ToString(tNow * 1000));
	std::string symbol = strCoinType + standardCurrency;
	info.mapParams["symbol"] = SHttpParam(eHttpParamType_String, symbol);
	if(tradeType == eFuturesTradeType_OpenBear)
		info.mapParams["side"] = SHttpParam(eHttpParamType_String, "SELL");
	else if(tradeType == eFuturesTradeType_OpenBull)
		info.mapParams["side"] = SHttpParam(eHttpParamType_String, "BUY");
	info.mapParams["type"] = SHttpParam(eHttpParamType_String, "LIMIT");
	info.mapParams["quantity"] = SHttpParam(eHttpParamType_String, size.c_str());
	info.mapParams["price"] = SHttpParam(eHttpParamType_String, price.c_str());
	info.mapParams["timeInForce"] = SHttpParam(eHttpParamType_String, "GTC");
	info.mapParams["newClientOrderId"] = SHttpParam(eHttpParamType_String, clientOrderID.c_str());
	if(bSync)
		RequestAsync(info);
	else
		Request(info, pResInfo);
}

void CBinanceHttpAPI::API_SwapFuturesTransferToSpot(bool bSync, std::string& standardCurrency, std::string amount, SHttpResponse* pResInfo)
{
	SHttpReqInfo info;
	info.apiType = eHttpAPIType_FuturesTransfer;
	info.reqType = eHttpReqType_Post;
	info.strMethod = "/sapi/v1/futures/transfer";
	info.confirmationType = eHttpConfirmationType_Binance;
	info.bSignature = true;
	time_t tNow = time(NULL);
	info.mapParams["timestamp"] = SHttpParam(eHttpParamType_Int, CFuncCommon::ToString(tNow * 1000));
	info.mapParams["asset"] = SHttpParam(eHttpParamType_String, standardCurrency.c_str());
	info.mapParams["amount"] = SHttpParam(eHttpParamType_String, amount.c_str());
	info.mapParams["type"] = SHttpParam(eHttpParamType_Int, "2");
	if(bSync)
		RequestAsync(info);
	else
		Request(info, pResInfo);
}

void CBinanceHttpAPI::API_FuturesOrderInfo(bool bSync, std::string& strCoinType, std::string& standardCurrency, __int64 orderID, SHttpResponse* pResInfo)
{
	SHttpReqInfo info;
	info.apiType = eHttpAPIType_FuturesTradeInfo;
	info.reqType = eHttpReqType_Get;
	info.strMethod = "/fapi/v1/order";
	info.confirmationType = eHttpConfirmationType_Binance;
	info.bSignature = true;
	time_t tNow = time(NULL);
	info.mapParams["timestamp"] = SHttpParam(eHttpParamType_Int, CFuncCommon::ToString(tNow * 1000));
	std::string symbol = strCoinType + standardCurrency;
	info.mapParams["symbol"] = SHttpParam(eHttpParamType_String, symbol);
	info.mapParams["orderId"] = SHttpParam(eHttpParamType_Int, CFuncCommon::ToString(orderID));
	if(bSync)
		RequestAsync(info);
	else
		Request(info, pResInfo);
}

void CBinanceHttpAPI::API_FuturesCancelOrder(bool bSync, std::string& strCoinType, std::string& standardCurrency, __int64 orderID, SHttpResponse* pResInfo)
{
	SHttpReqInfo info;
	info.apiType = eHttpAPIType_FuturesCancelOrder;
	info.reqType = eHttpReqType_Delete;
	info.strMethod = "/fapi/v1/order";
	info.confirmationType = eHttpConfirmationType_Binance;
	info.bSignature = true;
	time_t tNow = time(NULL);
	info.mapParams["timestamp"] = SHttpParam(eHttpParamType_Int, CFuncCommon::ToString(tNow * 1000));
	std::string symbol = strCoinType + standardCurrency;
	info.mapParams["symbol"] = SHttpParam(eHttpParamType_String, symbol);
	info.mapParams["orderId"] = SHttpParam(eHttpParamType_Int, CFuncCommon::ToString(orderID));
	if(bSync)
		RequestAsync(info);
	else
		Request(info, pResInfo);
}