#include "stdafx.h"
#include "zbg_exchange.h"
#include "exchange/zbg/zbg_http_api.h"
#include "exchange/zbg/zbg_websocket_api.h"

CZBGExchange::CZBGExchange(std::string strAPIKey, std::string strSecretKey)
{
	m_pWebSocketAPI = new CZbgWebsocketAPI(strAPIKey, strSecretKey);
	m_pHttpAPI = new CZbgHttpAPI(strAPIKey, strSecretKey, "application/json;charset=UTF-8");
}


CZBGExchange::~CZBGExchange()
{
}

void CZBGExchange::OnHttpResponse(eHttpAPIType type, Json::Value& retObj, const std::string& strRet)
{
	CExchange::OnHttpResponse(type, retObj, strRet);
}

void CZBGExchange::OnWebsocketResponse(Json::Value& retObj, const std::string& strRet)
{
	CExchange::OnWebsocketResponse(retObj, strRet);
}
