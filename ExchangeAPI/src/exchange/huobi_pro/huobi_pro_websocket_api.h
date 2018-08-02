#pragma once
#include "websocket_api/websocket_api.h"
class CHuobiProWebsocketAPI : public CWebsocketAPI
{
public:
	CHuobiProWebsocketAPI(std::string strAPIKey, std::string strSecretKey);
	~CHuobiProWebsocketAPI();
public:
	virtual void Ping();
	virtual void API_EntrustDepth(eMarketType type, int depthSize, bool bAdd);
	virtual void API_LatestExecutedOrder(eMarketType type){}
};

