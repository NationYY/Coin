#pragma once
#include "http_api/http_api.h"
class CBinanceHttpAPI : public CHttpAPI
{
public:
	CBinanceHttpAPI(std::string strAPIKey, std::string strSecretKey);
	~CBinanceHttpAPI();

	virtual void API_Ping();
	virtual void API_FuturesSetLeverage(bool bSync, std::string& strCoinType, std::string& standardCurrency, int nLeverage, SHttpResponse* pResInfo = NULL);
	virtual void API_ListenKey(bool bSync, SHttpResponse* pResInfo = NULL);
	virtual void API_FuturesAccountInfo(bool bSync, SHttpResponse* pResInfo);
	void API_FuturesTrade(bool bSync, eFuturesTradeType tradeType, std::string& strCoinType, std::string& standardCurrency, std::string& price, std::string& size, std::string& clientOrderID, SHttpResponse* pResInfo = NULL);
	void API_SwapFuturesTransferToSpot(bool bSync, std::string& standardCurrency, std::string amount, SHttpResponse* pResInfo = NULL);
	void API_FuturesOrderInfo(bool bSync, std::string& strCoinType, std::string& standardCurrency, __int64 orderID, SHttpResponse* pResInfo = NULL);
	void API_FuturesCancelOrder(bool bSync, std::string& strCoinType, std::string& standardCurrency, __int64 orderID, SHttpResponse* pResInfo = NULL);
};

