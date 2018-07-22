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

void CZbgWebsocketAPI::API_EntrustDepth(const char* szType, int depthSize, bool bAdd)
{
	char szBuffer[128] = { 0 };
	_snprintf(szBuffer, 128, "{\"dataType\":\"323_ENTRUST_ADD_%s\",\"dataSize\":%d,\"action\":\"%s\"}", szType, depthSize, (bAdd ? "ADD" : "DEL"));
	Request(szBuffer);
}