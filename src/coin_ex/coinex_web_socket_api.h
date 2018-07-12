#pragma once
#include "websocket_api.h"
class CCoinexWebSocketAPI : public CWebSocketAPI
{
public:
	CCoinexWebSocketAPI(string api_key, string secret_key);
	~CCoinexWebSocketAPI();
	void LoginIn();
	void GetServerTime();
	void Ping();
};

