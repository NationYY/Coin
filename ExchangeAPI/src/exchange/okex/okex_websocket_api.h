#pragma once
#include "websocket_api/websocket_api.h"
class COkexWebsocketAPI : public CWebsocketAPI
{
public:
	COkexWebsocketAPI(std::string strAPIKey, std::string strSecretKey, bool bFutures);
	virtual ~COkexWebsocketAPI();
public:
	virtual bool Ping();
	virtual void API_EntrustDepth(eMarketType type, int depthSize, bool bAdd);
	virtual void API_LatestExecutedOrder(eMarketType type){}
	virtual void API_FuturesKlineData(bool bAdd, std::string& strKlineType, std::string& strCoinType, std::string& strFuturesCycle);
	virtual void API_FuturesTickerData(bool bAdd, std::string& strCoinType, std::string& strFuturesCycle);
};

