#pragma once
#include "websocket_api/websocket_api.h"
class COkexWebsocketAPI : public CWebsocketAPI
{
public:
	COkexWebsocketAPI(std::string strAPIKey, std::string strSecretKey);
	~COkexWebsocketAPI();
public:
	virtual void Ping();
	virtual void API_EntrustDepth(eMarketType type, int depthSize, bool bAdd);
	virtual void API_LatestExecutedOrder(eMarketType type){}
};

