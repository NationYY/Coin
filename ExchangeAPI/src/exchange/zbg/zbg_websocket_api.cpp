#include "stdafx.h"
#include "zbg_websocket_api.h"


CZbgWebsocketAPI::CZbgWebsocketAPI(std::string strAPIKey, std::string strSecretKey)
{
	SetKey(strAPIKey, strSecretKey);
	SetURI("wss://kline.zbg.com/websocket");
	SetUTF8(true);
}


CZbgWebsocketAPI::~CZbgWebsocketAPI()
{
}

void CZbgWebsocketAPI::API_EntrustDepth(eMarketType type, int depthSize, bool bAdd)
{
	if(m_pExchange)
		m_pExchange->GetDataCenter()->ClearAllEntrustDepth();
	char szBuffer[128] = { 0 };
	_snprintf(szBuffer, 128, "{\"dataType\":\"323_ENTRUST_ADD_%s\",\"dataSize\":%d,\"action\":\"%s\"}", "", depthSize, (bAdd ? "ADD" : "DEL"));
	Request(szBuffer);
}