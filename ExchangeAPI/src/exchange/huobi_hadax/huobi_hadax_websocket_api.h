#pragma once
#include "websocket_api/websocket_api.h"
class CHuobiHadaxWebsocketAPI : public CWebsocketAPI
{
public:
	CHuobiHadaxWebsocketAPI(std::string strAPIKey, std::string strSecretKey);
	virtual ~CHuobiHadaxWebsocketAPI();
public:
	virtual void Ping();
	virtual void API_EntrustDepth(eMarketType type, int depthSize, bool bAdd);
	virtual void API_LatestExecutedOrder(eMarketType type){}
};

