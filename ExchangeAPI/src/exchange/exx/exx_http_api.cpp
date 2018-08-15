#include "stdafx.h"
#include "exx_http_api.h"
#include "exchange/data_center.h"
#include "exchange/exchange.h"
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

void CExxHttpAPI::API_Trade(eMarketType type, std::string strAmount, std::string strPrice, bool bBuy, int customData, std::string strCustomData)
{
	SHttpReqInfo info;
	info.apiType = eHttpAPIType_Trade;
	info.reqType = eHttpReqType_Get;
	info.customData = customData;
	info.strCustomData = strCustomData; 
	info.strMethod = "order";
	info.confirmationType = eHttpConfirmationType_Exx;
	info.mapParams["accesskey"] = SHttpParam(eHttpParamType_String, m_strAPIKey);
	info.mapParams["amount"] = SHttpParam(eHttpParamType_String, strAmount);
	info.mapParams["currency"] = SHttpParam(eHttpParamType_String, m_pExchange->GetMarketString(type, true));
	info.bUTF8 =  true;
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
	info.mapParams["currency"] = SHttpParam(eHttpParamType_String, m_pExchange->GetMarketString(type, true));
	RequestAsync(info);
}


void CExxHttpAPI::API_EntrustDepth(eMarketType type)
{
	SHttpReqInfo info;
	info.strURL = "https://api.exxvip.com/data/v1";
	info.apiType = eHttpAPIType_EntrustDepth;
	info.reqType = eHttpReqType_Get;
	info.strMethod = "depth";
	info.mapParams["currency"] = SHttpParam(eHttpParamType_String, m_pExchange->GetMarketString(type, true));
	RequestAsync(info);
}

void CExxHttpAPI::API_GetTradeOrderListState(eMarketType type, int page, bool bBuy)
{
	SHttpReqInfo info;
	info.apiType = eHttpAPIType_TradeOrderListState;
	info.reqType = eHttpReqType_Get;
	info.strMethod = "getOpenOrders";
	info.confirmationType = eHttpConfirmationType_Exx;
	info.mapParams["accesskey"] = SHttpParam(eHttpParamType_String, m_strAPIKey);
	info.mapParams["currency"] = SHttpParam(eHttpParamType_String, m_pExchange->GetMarketString(type, true));
	char szBuffer[128];
	_snprintf(szBuffer, 128, "%lld", time(NULL) * 1000);
	info.mapParams["nonce"] = SHttpParam(eHttpParamType_Int, szBuffer);
	_snprintf(szBuffer, 128, "%d", page);
	info.mapParams["pageIndex"] = SHttpParam(eHttpParamType_String, szBuffer);
	if(bBuy)
		info.mapParams["type"] = SHttpParam(eHttpParamType_String, "buy");
	else
		info.mapParams["type"] = SHttpParam(eHttpParamType_String, "sell");
	RequestAsync(info);
}

void CExxHttpAPI::API_GetTradeOrderState(eMarketType type, std::string strID)
{
	SHttpReqInfo info;
	info.apiType = eHttpAPIType_TradeOrderState;
	info.reqType = eHttpReqType_Get;
	info.strMethod = "getOrder";
	info.confirmationType = eHttpConfirmationType_Exx;
	info.mapParams["accesskey"] = SHttpParam(eHttpParamType_String, m_strAPIKey);
	info.mapParams["currency"] = SHttpParam(eHttpParamType_String, m_pExchange->GetMarketString(type, true));
	char szBuffer[128];
	_snprintf(szBuffer, 128, "%lld", time(NULL) * 1000);
	info.mapParams["nonce"] = SHttpParam(eHttpParamType_Int, szBuffer);
	info.mapParams["id"] = SHttpParam(eHttpParamType_String, strID);
	RequestAsync(info);
}

void CExxHttpAPI::API_CancelTrade(eMarketType type, std::string strID, std::string strCustomData)
{
	SHttpReqInfo info;
	info.apiType = eHttpAPIType_CancelTrade;
	info.reqType = eHttpReqType_Get;
	info.strMethod = "cancel";
	info.strCustomData = strCustomData;
	info.confirmationType = eHttpConfirmationType_Exx;
	info.mapParams["accesskey"] = SHttpParam(eHttpParamType_String, m_strAPIKey);
	info.mapParams["currency"] = SHttpParam(eHttpParamType_String, m_pExchange->GetMarketString(type, true));
	char szBuffer[128];
	_snprintf(szBuffer, 128, "%lld", time(NULL) * 1000);
	info.mapParams["nonce"] = SHttpParam(eHttpParamType_Int, szBuffer);
	info.mapParams["id"] = SHttpParam(eHttpParamType_String, strID);
	RequestAsync(info);
}