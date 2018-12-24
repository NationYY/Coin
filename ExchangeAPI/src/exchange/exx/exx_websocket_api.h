#pragma once
#include "websocket_api/websocket_api.h"
#ifdef _OPEN_EXX_
class CExxWebSocketAPI : public CWebsocketAPI
{
public:
	CExxWebSocketAPI(std::string strAPIKey, std::string strSecretKey);
	virtual ~CExxWebSocketAPI();
	virtual void API_EntrustDepth(eMarketType type, int depthSize, bool bAdd);
	virtual void API_LatestExecutedOrder(eMarketType type) {};
	virtual bool Ping(){ return true; }
};

#endif