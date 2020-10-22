#include "stdafx.h"
#include "binance_exchange.h"
#include "exchange/binance/binance_http_api.h"
#include "exchange/binance/binance_websocket_api.h"
#include "log/local_log.h"

CBinanceExchange::CBinanceExchange(std::string strAPIKey, std::string strSecretKey)
{
	m_pWebSocketAPI = NULL;
	m_pHttpAPI = new CBinanceHttpAPI(strAPIKey, strSecretKey);
	m_pHttpAPI->SetExchange(this);
}


CBinanceExchange::~CBinanceExchange()
{
}

void CBinanceExchange::OnHttpResponse(eHttpAPIType type, Json::Value& retObj, const std::string& strRet, int customData, std::string strCustomData)
{
	CExchange::OnHttpResponse(type, retObj, strRet, customData, strCustomData);
}
