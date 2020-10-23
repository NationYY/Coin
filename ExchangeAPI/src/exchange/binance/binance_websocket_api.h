#pragma once
#include "websocket_api/websocket_api.h"
class CBinanceWebsocketAPI : public CWebsocketAPI
{
public:
	CBinanceWebsocketAPI(std::string strAPIKey, std::string strSecretKey);
	~CBinanceWebsocketAPI();
public:
	virtual void Ping();
};

