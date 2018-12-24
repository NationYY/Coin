#pragma once
#include "websocket_api/websocket_api.h"
#ifdef _OPEN_COINEX_
class CCoinexWebSocketAPI : public CWebsocketAPI
{
public:
	CCoinexWebSocketAPI(std::string strAPIKey, std::string strSecretKey);
	virtual ~CCoinexWebSocketAPI();
	void LoginIn();
	void GetServerTime();
public:
	//订阅交易深度
	virtual void API_EntrustDepth(eMarketType type, int depthSize, bool bAdd);
	//订阅最新成交的订单
	virtual void API_LatestExecutedOrder(eMarketType type);
	virtual bool Ping();
};

#endif