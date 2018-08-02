#include "stdafx.h"
#include "exx_websocket_api.h"
#include "exchange/exchange.h"

CExxWebSocketAPI::CExxWebSocketAPI(std::string strAPIKey, std::string strSecretKey)
{
	SetKey(strAPIKey, strSecretKey);
	SetURI("wss://ws.exx.com/websocket");
}


CExxWebSocketAPI::~CExxWebSocketAPI()
{
}


void CExxWebSocketAPI::API_EntrustDepth(eMarketType type, int depthSize, bool bAdd)
{
	if(m_pExchange)
		m_pExchange->GetDataCenter()->ClearAllEntrustDepth();
	char szBuffer[128] = { 0 };
	_snprintf(szBuffer, 128, "{\"dataType\":\"1_ENTRUST_ADD_%s\",\"dataSize\":%d,\"action\":\"%s\"}", m_pExchange->GetMarketString(type, false), depthSize, (bAdd ? "ADD" : "DEL"));
	Request(szBuffer);
}