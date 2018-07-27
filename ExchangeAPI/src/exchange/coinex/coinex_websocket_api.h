#pragma once
#include "websocket_api/websocket_api.h"
class CCoinexWebSocketAPI : public CWebSocketAPI
{
public:
	CCoinexWebSocketAPI(std::string strAPIKey, std::string strSecretKey);
	~CCoinexWebSocketAPI();
	void LoginIn();
	void GetServerTime();
public:
	//订阅交易深度
	virtual void API_EntrustDepth(eMarketType type, int depthSize, bool bAdd) {};
	//订阅最新成交的订单
	virtual void API_LatestExecutedOrder(eMarketType type);
	virtual void Ping();
private:
	const char* GetMarketString(eMarketType type)
	{
		switch(type)
		{
		case eMarketType_ETH_BTC:
			return "ETHBTC";
		case eMarketType_ETH_USDT:
			return "ETHUSDT";
		case eMarketType_BTC_USDT:
			return "BTCUSDT";
		}
		return NULL;
	}
};

