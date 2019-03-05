#pragma once
#ifdef _OPEN_OKEX_
#include "websocket_api/websocket_api.h"
class COkexWebsocketAPI : public CWebsocketAPI
{
public:
	COkexWebsocketAPI(std::string strAPIKey, std::string strSecretKey, std::string strPassphrase, bool bFutures);
	virtual ~COkexWebsocketAPI();
public:
	virtual bool Ping();
	virtual void API_EntrustDepth(eMarketType type, int depthSize, bool bAdd);
	virtual void API_LatestExecutedOrder(eMarketType type){}
	virtual void API_FuturesKlineData(bool bAdd, bool bSwap, std::string& strKlineType, std::string& strCoinType, std::string& strFuturesCycle);
	virtual void API_FuturesTickerData(bool bAdd, bool bSwap, std::string& strCoinType, std::string& strFuturesCycle);

	virtual void API_SpotKlineData(bool bAdd, std::string& strKlineType, std::string& strCoinType, std::string& strMoneyType);
	virtual void API_SpotTickerData(bool bAdd, std::string& strCoinType, std::string& strMoneyType);

	void API_LoginFutures(std::string& strAPIKey, std::string& strSecretKey, __int64 timeStamp);

	void API_FuturesOrderInfo(bool bAdd, bool bSwap, std::string& strCoinType, std::string& strFuturesCycle);
	void API_SpotOrderInfo(bool bAdd, std::string& strCoinType, std::string& strMoneyType);
	void API_FuturesEntrustDepth(bool bAdd, bool bSwap, std::string& strCoinType, std::string& strFuturesCycle);

	void API_FuturesAccountInfoByCurrency(bool bAdd, bool bSwap, std::string& strCoinType);
	void API_SpotAccountInfoByCurrency(bool bAdd, std::string& strCoinType);

	void API_FuturesPositionInfo(bool bAdd, bool bSwap, std::string& strCoinType, std::string& strFuturesCycle);
};
#endif
