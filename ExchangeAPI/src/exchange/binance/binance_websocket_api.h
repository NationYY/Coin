#pragma once
#include "websocket_api/websocket_api.h"
class CBinanceWebsocketAPI : public CWebsocketAPI
{
public:
	CBinanceWebsocketAPI(std::string strAPIKey, std::string strSecretKey);
	~CBinanceWebsocketAPI();
public:
	void SetListenKey(std::string key);
	virtual void Run();
	virtual void Ping();
	//¶©ÔÄºÏÔ¼tick
	virtual void API_FuturesTickerData(bool bAdd, std::string& strCoinType, std::string& standardCurrency);
	virtual void API_FuturesEntrustDepth(bool bAdd, std::string& strCoinType, std::string& standardCurrency);
private:
	bool UpdateStream(bool bAdd, std::string stream);
private:
	std::string m_strStream;
};

