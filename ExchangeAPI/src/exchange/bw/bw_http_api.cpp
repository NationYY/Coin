#include "stdafx.h"
#include "bw_http_api.h"
#include "exchange/exchange.h"
#ifdef _OPEN_BW_
CBWHttpAPI::CBWHttpAPI(std::string strAPIKey, std::string strSecretKey)
{
	SetKey(strAPIKey, strSecretKey);
	SetURL("https://www.BW6.com");
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

void CBWHttpAPI::API_Trade(eMarketType type, std::string strAmount, std::string strPrice, bool bBuy, int customData, std::string strCustomData)
{
	SHttpReqInfo info;
	info.apiType = eHttpAPIType_Trade;
	info.reqType = eHttpReqType_Post;
	info.strMethod = "exchange/entrust/controller/website/EntrustController/addEntrust";
	info.mapParams["amount"] = SHttpParam(eHttpParamType_Int, strAmount.c_str());
	info.mapParams["type"] = SHttpParam(eHttpParamType_Int, (bBuy ? "1" : "0"));
	info.mapParams["rangeType"] = SHttpParam(eHttpParamType_Int, "0");
	info.mapParams["marketId"] = SHttpParam(eHttpParamType_String, CFuncCommon::ToString(m_pExchange->GetMarketID(type)));
	info.mapParams["price"] = SHttpParam(eHttpParamType_Int, strPrice.c_str());
	info.confirmationType = eHttpConfirmationType_Zbg;
	info.customData = customData;
	info.strCustomData = strCustomData;
	info.bUTF8 = true;
	RequestAsync(info);

}

void CBWHttpAPI::API_CancelTrade(eMarketType type, std::string strID, std::string strCustomData)
{
	SHttpReqInfo info;
	info.apiType = eHttpAPIType_CancelTrade;
	info.reqType = eHttpReqType_Post;
	info.strMethod = "exchange/entrust/controller/website/EntrustController/cancelEntrust";
	info.mapParams["marketId"] = SHttpParam(eHttpParamType_String, CFuncCommon::ToString(m_pExchange->GetMarketID(type)));
	info.mapParams["entrustId"] = SHttpParam(eHttpParamType_String, strID.c_str());
	info.confirmationType = eHttpConfirmationType_Zbg;
	info.strCustomData = strCustomData;
	info.bUTF8 = true;
	RequestAsync(info);

}

void CBWHttpAPI::API_GetTradeOrderState(eMarketType type, std::string strID)
{
	SHttpReqInfo info;
	info.apiType = eHttpAPIType_TradeOrderState;
	info.reqType = eHttpReqType_Get;
	info.strMethod = "exchange/entrust/controller/website/EntrustController/getEntrustById";
	info.mapParams["marketId"] = SHttpParam(eHttpParamType_String, CFuncCommon::ToString(m_pExchange->GetMarketID(type)));
	info.mapParams["entrustId"] = SHttpParam(eHttpParamType_String, strID.c_str());
	info.confirmationType = eHttpConfirmationType_Zbg;
	info.bUTF8 = true;
	RequestAsync(info);
}
#endif