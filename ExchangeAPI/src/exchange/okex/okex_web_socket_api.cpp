#include "stdafx.h"
#include "okex_web_socket_api.h"


COkexWebSocketAPI::COkexWebSocketAPI(std::string strAPIKey, std::string strSecretKey)
{
	SetKey(strAPIKey, strSecretKey);
	SetURI("wss://real.okex.com:10441/websocket");
}


COkexWebSocketAPI::~COkexWebSocketAPI()
{
}

void COkexWebSocketAPI::API_sub_spot_ticker(const char* szTransactionType)
{
	char szBuffer[128] = {0};
	_snprintf(szBuffer, 128, "{'event':'addChannel','channel':'ok_sub_spot_%s_ticker'}", szTransactionType);
	Request(szBuffer);
}
