#pragma once
#include "websocket_api.h"
class CCoinexWebSocketAPI : public CWebSocketAPI
{
public:
	CCoinexWebSocketAPI(string strAPIKey, string strSecretKey);
	~CCoinexWebSocketAPI();
	void LoginIn();
	void GetServerTime();
	void Ping();
};

