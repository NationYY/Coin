#include "stdafx.h"
#include "zbg_exchange.h"
#include "exchange/zbg/zbg_http_api.h"
#include "exchange/zbg/zbg_websocket_api.h"

CZbgExchange::CZbgExchange(std::string strAPIKey, std::string strSecretKey)
{
	m_pWebSocketAPI = new CZbgWebsocketAPI(strAPIKey, strSecretKey);
	m_pHttpAPI = new CZbgHttpAPI(strAPIKey, strSecretKey, "application/json;charset=UTF-8");
	m_listSupportMarket.push_back(eMarketType_ETH_USDT);
	m_listSupportMarket.push_back(eMarketType_BTC_USDT);
}


CZbgExchange::~CZbgExchange()
{
}

void CZbgExchange::OnHttpResponse(eHttpAPIType type, Json::Value& retObj, const std::string& strRet, int customData, std::string strCustomData)
{
	CExchange::OnHttpResponse(type, retObj, strRet, customData, strCustomData);
}

void CZbgExchange::OnWebsocketResponse(Json::Value& retObj, const std::string& strRet)
{
	CExchange::OnWebsocketResponse(retObj, strRet);
}

void CZbgExchange::Run()
{
	CExchange::Run();
	SHttpReqInfo info;
	info.reqType = eHttpReqType_Post;
	info.strMethod = "exchange/config/controller/website/currencycontroller/getCurrencyList";
	info.confirmationType = eHttpConfirmationType_Zbg;
	SHttpResponse resInfo;
	m_pHttpAPI->Request(info, resInfo);

}