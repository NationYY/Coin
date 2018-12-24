#pragma once
#include "websocket_api/websocket_api.h"
#ifdef _OPEN_HUOBI_HADAX_
class CHuobiHadaxWebsocketAPI : public CWebsocketAPI
{
public:
	CHuobiHadaxWebsocketAPI(std::string strAPIKey, std::string strSecretKey);
	virtual ~CHuobiHadaxWebsocketAPI();
public:
	virtual bool Ping();
	virtual void API_EntrustDepth(eMarketType type, int depthSize, bool bAdd);
	virtual void API_LatestExecutedOrder(eMarketType type){}
};

#endif