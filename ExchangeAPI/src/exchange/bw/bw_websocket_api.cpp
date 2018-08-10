#include "stdafx.h"
#include "bw_websocket_api.h"
#include "exchange/bw/bw_exchange.h"

CBWWebsocketAPI::CBWWebsocketAPI(std::string strAPIKey, std::string strSecretKey)
{
	SetKey(strAPIKey, strSecretKey);
	SetURI("wss://kline.BW.com/websocket/");
}


CBWWebsocketAPI::~CBWWebsocketAPI()
{
}


void CBWWebsocketAPI::API_EntrustDepth(eMarketType type, int depthSize, bool bAdd)
{
	if(m_pExchange)
		m_pExchange->GetDataCenter()->ClearAllEntrustDepth();
	char szBuffer[512] = {0};
	_snprintf(szBuffer, 512, "{\"dataType\":\"%d_ENTRUST_ADD_%s\", \"dataSize\" : %d, \"action\" : \"%s\"}", ((CBWExchange*)m_pExchange)->GetMarketID(type), m_pExchange->GetMarketString(type, false), depthSize, (bAdd ? "ADD" : "DEL"));
	Request(szBuffer);
}

void CBWWebsocketAPI::API_LatestExecutedOrder(eMarketType type)
{
	char szBuffer[512] = { 0 };
	_snprintf(szBuffer, 512, "{\"dataType\":\"%d_TRADE_%s\", \"dataSize\" : 20, \"action\" : \"ADD\"}", ((CBWExchange*)m_pExchange)->GetMarketID(type), m_pExchange->GetMarketString(type, false));
	Request(szBuffer);
}