#pragma once
#include "websocket_api/websocket_api.h"
class CExxWebSocketAPI : public CWebsocketAPI
{
public:
	CExxWebSocketAPI(std::string strAPIKey, std::string strSecretKey);
	virtual ~CExxWebSocketAPI();
	virtual void API_EntrustDepth(eMarketType type, int depthSize, bool bAdd);
	virtual void API_LatestExecutedOrder(eMarketType type) {};
	virtual void Ping(){}
};

