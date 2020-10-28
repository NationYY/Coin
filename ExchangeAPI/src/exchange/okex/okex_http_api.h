#pragma once
#ifdef _OPEN_OKEX_
#include "http_api/http_api.h"
class COkexHttpAPI : public CHttpAPI
{
public:
	COkexHttpAPI(std::string strAPIKey, std::string strSecretKey, std::string strPassphrase);
	virtual ~COkexHttpAPI();
	virtual void API_Ping() {};
	virtual void API_Balance() {};
	virtual void API_Ticker(eMarketType type) {};
	virtual void API_EntrustDepth(eMarketType type) {};
	virtual void API_Trade(eMarketType type, std::string strAmount, std::string strPrice, bool bBuy, int customData, std::string strCustomData) {};
	virtual void API_GetTradeOrderListState(eMarketType type, int page, bool bBuy) {};
	virtual void API_GetTradeOrderState(eMarketType type, std::string strID) {};
	virtual void API_CancelTrade(eMarketType type, std::string strID, std::string strCustomData) {};
	virtual void API_FuturesInstruments(bool bSync, bool bSwap, SHttpResponse* pResInfo = NULL);
	virtual void API_FuturesSetLeverage(bool bSync, bool bSwap, std::string& strCoinType, std::string& standardCurrency, std::string& strLeverage, SHttpResponse* pResInfo = NULL);


	void API_FuturesAccountInfoByCurrency(bool bSync, bool bSwap, std::string& currency, std::string& standardCurrency, SHttpResponse* pResInfo = NULL);
	void API_FuturesOrderInfo(bool bSync, bool bSwap, std::string& strCoinType, std::string& standardCurrency, std::string& strFuturesCycle, std::string& orderID, SHttpResponse* pResInfo = NULL);
	void API_FuturesTrade(bool bSync, bool bSwap, eFuturesTradeType tradeType, std::string& strCoinType, std::string& standardCurrency, std::string& strFuturesCycle, std::string& price, std::string& size, std::string& leverage, std::string& clientOrderID, SHttpResponse* pResInfo = NULL);
	void API_FuturesServerTime();
	void API_FuturesCancelOrder(bool bSync, bool bSwap, std::string& strCoinType, std::string& standardCurrency, std::string& strFuturesCycle, std::string& orderID, SHttpResponse* pResInfo = NULL);
	void API_FuturesPositionInfo(bool bSync, bool bSwap, std::string& strCoinType, std::string& standardCurrency, std::string& strFuturesCycle, SHttpResponse* pResInfo = NULL);


	void API_GetFuturesSomeKline(bool bSync, bool bSwap, std::string& strCoinType, std::string& standardCurrency, std::string& strFuturesCycle, std::string& strKlineCycle, std::string& strFrom, std::string& strTo, SHttpResponse* pResInfo = NULL);
	void API_GetSpotSomeKline(bool bSync, std::string& instrumentid, std::string& strKlineCycle, std::string& strFrom, std::string& strTo, SHttpResponse* pResInfo = NULL);
	void API_SpotTrade(bool bSync, bool bMargin, std::string& instrumentID, std::string& tradeType, std::string& price, std::string& size, std::string& clientOrderID, SHttpResponse* pResInfo = NULL);
	void API_SpotOrderInfo(bool bSync, bool bMargin, std::string& instrumentID, std::string& orderID, SHttpResponse* pResInfo = NULL);

	void API_SpotAccountInfoByCurrency(bool bSync, std::string& strMoneyType, SHttpResponse* pResInfo = NULL);
	void API_MarginAccountInfoByCurrency(bool bSync, std::string& strCoinType, std::string strMoneyType, SHttpResponse* pResInfo = NULL);


	void API_SpotCancelOrder(bool bSync, bool bMargin, std::string& instrumentID, std::string& orderID, SHttpResponse* pResInfo = NULL);
	void API_SpotInstruments(bool bSync, SHttpResponse* pResInfo = NULL);

	void API_SwapFuturesTransferToZiJin(bool bSync, std::string& strCoinType, std::string& standardCurrency, std::string amount, SHttpResponse* pResInfo = NULL);
	void API_ZiJinTransferToSwapFutures(bool bSync, std::string& strCoinType, std::string& standardCurrency, std::string amount, SHttpResponse* pResInfo = NULL);
public:
	int m_futuresAccountInfoByCurrencyIndex;
	int m_SpotAccountInfoByCurrencyIndex;
	int m_marginAccountInfoByCurrency;
};
#endif

