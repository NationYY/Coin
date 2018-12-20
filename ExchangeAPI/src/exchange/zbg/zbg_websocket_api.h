#pragma once
#include "websocket_api/websocket_api.h"
class CZbgWebsocketAPI : public CWebsocketAPI
{
public:
	CZbgWebsocketAPI(std::string strAPIKey, std::string strSecretKey);
	virtual ~CZbgWebsocketAPI();
public:
	virtual void API_EntrustDepth(eMarketType type, int depthSize, bool bAdd);
	virtual void API_LatestExecutedOrder(eMarketType type) {};
	virtual bool Ping() { return true; };
};

