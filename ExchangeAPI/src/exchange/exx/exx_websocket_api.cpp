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
	char szBuffer[128] = { 0 };
	switch(type)
	{
	case eMarketType_ETH_BTC:
		_snprintf(szBuffer, 128, "{\"dataType\":\"1_ENTRUST_ADD_ETH_BTC\",\"dataSize\":%d,\"action\":\"%s\"}", depthSize, (bAdd ? "ADD" : "DEL"));
		break;
	case eMarketType_ETH_USDT:
		_snprintf(szBuffer, 128, "{\"dataType\":\"1_ENTRUST_ADD_ETH_USDT\",\"dataSize\":%d,\"action\":\"%s\"}", depthSize, (bAdd ? "ADD" : "DEL"));
		break;
	case eMarketType_BTC_USDT:
		_snprintf(szBuffer, 128, "{\"dataType\":\"1_ENTRUST_ADD_BTC_USDT\",\"dataSize\":%d,\"action\":\"%s\"}", depthSize, (bAdd ? "ADD" : "DEL"));
		break;
	default:
		break;
	}
	Request(szBuffer);
}