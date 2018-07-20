#pragma once
#include "websocket_api/websocket_api.h"
class CCoinexWebSocketAPI : public CWebSocketAPI
{
public:
	CCoinexWebSocketAPI(std::string strAPIKey, std::string strSecretKey);
	~CCoinexWebSocketAPI();
	void LoginIn();
	void GetServerTime();
	void Ping();
};

