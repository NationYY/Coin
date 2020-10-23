#include "stdafx.h"
#include "binance_websocket_api.h"
#include "log/local_log.h"

CBinanceWebsocketAPI::CBinanceWebsocketAPI(std::string strAPIKey, std::string strSecretKey)
{
	SetKey(strAPIKey, strSecretKey);
	SetURI("wss://fstream.binance.com");
}


CBinanceWebsocketAPI::~CBinanceWebsocketAPI()
{
}

void CBinanceWebsocketAPI::Ping()
{
	Request("pong");
}