#pragma once
#ifdef _OPEN_HUOBI_PRO_
#include "websocket_api/websocket_api.h"
class CHuobiProWebsocketAPI : public CWebsocketAPI
{
public:
	CHuobiProWebsocketAPI(std::string strAPIKey, std::string strSecretKey);
	virtual ~CHuobiProWebsocketAPI();
public:
	virtual bool Ping();
	virtual void API_EntrustDepth(eMarketType type, int depthSize, bool bAdd);
	virtual void API_LatestExecutedOrder(eMarketType type){}
};
#endif

