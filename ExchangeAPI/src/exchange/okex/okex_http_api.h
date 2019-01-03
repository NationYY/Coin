#pragma once
#ifdef _OPEN_OKEX_
#include "http_api/http_api.h"
class COkexHttpAPI : public CHttpAPI
{
public:
	COkexHttpAPI(std::string strAPIKey, std::string strSecretKey, std::string strPassphrase);
	virtual ~COkexHttpAPI();
	virtual void API_Balance() {};
	virtual void API_Ticker(eMarketType type) {};
	virtual void API_EntrustDepth(eMarketType type) {};
	virtual void API_Trade(eMarketType type, std::string strAmount, std::string strPrice, bool bBuy, int customData, std::string strCustomData) {};
	virtual void API_GetTradeOrderListState(eMarketType type, int page, bool bBuy) {};
	virtual void API_GetTradeOrderState(eMarketType type, std::string strID) {};
	virtual void API_CancelTrade(eMarketType type, std::string strID, std::string strCustomData) {};

	void API_FuturesAccountInfoByCurrency(std::string& currency);
	void API_FuturesOrderInfo(std::string& strCoinType, std::string& strFuturesCycle, std::string& orderID);
	void API_FuturesTrade(eFuturesTradeType tradeType, std::string& strCoinType, std::string& strFuturesCycle, std::string& price, std::string& size, std::string& leverage, std::string& clientOrderID);
	void API_FuturesServerTime();
	void API_FuturesCancelOrder(std::string& strCoinType, std::string& strFuturesCycle, std::string& orderID);
	void API_FuturesInstruments(bool bSync, SHttpResponse* pResInfo = NULL);

	void API_SpotTrade(bool bSync, std::string& instrumentID, eTradeType tradeType, std::string price, std::string size, std::string& clientOrderID, SHttpResponse* pResInfo = NULL);
	void API_SpotOrderInfo(bool bSync, std::string& instrumentID, std::string& orderID, SHttpResponse* pResInfo = NULL);
	void API_SpotAccountInfoByCurrency(std::string& strMoneyType);
	void API_SpotCancelOrder(bool bSync, std::string& instrumentID, std::string& orderID, SHttpResponse* pResInfo = NULL);
	void API_SpotInstruments(bool bSync, SHttpResponse* pResInfo = NULL);
public:
	int m_futuresAccountInfoByCurrencyIndex;
};
#endif

