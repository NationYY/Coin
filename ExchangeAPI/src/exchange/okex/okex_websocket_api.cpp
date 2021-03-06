#include "stdafx.h"
#include "okex_websocket_api.h"
#include "algorithm/hmac.h"
#include <websocketpp/base64/base64.hpp>
#include "log/local_log.h"
#ifdef _OPEN_OKEX_
COkexWebsocketAPI::COkexWebsocketAPI(std::string strAPIKey, std::string strSecretKey, std::string strPassphrase, bool bFutures)
{
	SetKey(strAPIKey, strSecretKey, strPassphrase, bFutures);
	if(bFutures)
		SetURI("wss://real.okex.com:10442/ws/v3");
	else
		SetURI("wss://real.okex.com:10441/websocket");
}


COkexWebsocketAPI::~COkexWebsocketAPI()
{
}

bool COkexWebsocketAPI::Ping()
{
	return Request("ping");
}

void COkexWebsocketAPI::API_EntrustDepth(eMarketType type, int depthSize, bool bAdd)
{
	if(m_pExchange)
		m_pExchange->GetDataCenter()->ClearAllEntrustDepth();
	char szBuffer[512] = {0};
	_snprintf(szBuffer, 512, "{\"event\":\"%s\",\"channel\":\"ok_sub_spot_%s_depth\"}", (bAdd ? "addChannel" : "removeChannel"), m_pExchange->GetMarketString(type, false));
	Request(szBuffer);
}

void COkexWebsocketAPI::API_FuturesKlineData(bool bAdd, bool bSwap, std::string& strKlineType, std::string& strCoinType, std::string& strFuturesCycle)
{
	char szBuffer[512] = {0};
	if(bSwap)
	{
		m_futuresKlineCheck = "swap/";
		_snprintf(szBuffer, 512, "{\"op\":\"%s\",\"args\":[\"swap/%s:%s-USD-SWAP\"]}", (bAdd ? "subscribe" : "unsubscribe"), strKlineType.c_str(), strCoinType.c_str());
	}
	else
	{
		m_futuresKlineCheck = "futures/";
		_snprintf(szBuffer, 512, "{\"op\":\"%s\",\"args\":[\"futures/%s:%s-USD-%s\"]}", (bAdd ? "subscribe" : "unsubscribe"), strKlineType.c_str(), strCoinType.c_str(), strFuturesCycle.c_str());
	}
	m_futuresKlineCheck.append(strKlineType);
	Request(szBuffer);
}


void COkexWebsocketAPI::API_FuturesTickerData(bool bAdd, bool bSwap, std::string& strCoinType, std::string& strFuturesCycle)
{
	char szBuffer[512] = {0};
	if(bSwap)
	{
		m_futuresTickerCheck = "swap/ticker";
		_snprintf(szBuffer, 512, "{\"op\":\"%s\",\"args\":[\"swap/ticker:%s-USD-SWAP\"]}", (bAdd ? "subscribe" : "unsubscribe"), strCoinType.c_str());
	}
	else
	{
		m_futuresTickerCheck = "futures/ticker";
		_snprintf(szBuffer, 512, "{\"op\":\"%s\",\"args\":[\"futures/ticker:%s-USD-%s\"]}", (bAdd ? "subscribe" : "unsubscribe"), strCoinType.c_str(), strFuturesCycle.c_str());
	}
	Request(szBuffer);
}

void COkexWebsocketAPI::API_LoginFutures(std::string& strAPIKey, std::string& strSecretKey, __int64 timeStamp)
{
	unsigned char * mac = NULL;
	unsigned int mac_length = 0;
	char szData[64] = {0};
	_snprintf(szData, 64, "%lld.000GET/users/self/verify", timeStamp);
	int ret = HmacEncode("sha256", strSecretKey.c_str(), strSecretKey.length(), szData, strlen(szData), mac, mac_length);
	std::string strSign = websocketpp::base64_encode(mac, mac_length);
	free(mac);
	char szBuffer[512] = {0};
	_snprintf(szBuffer, 512, "{\"op\":\"login\",\"args\":[\"%s\",\"%s\",\"%lld.000\",\"%s\"]}", strAPIKey.c_str(), m_strPassphrase.c_str(), timeStamp, strSign.c_str());
	Request(szBuffer);
}

void COkexWebsocketAPI::API_FuturesOrderInfo(bool bAdd, bool bSwap, std::string& strCoinType, std::string& strFuturesCycle)
{
	char szBuffer[512] = { 0 };
	if(bSwap)
		_snprintf(szBuffer, 512, "{\"op\":\"%s\",\"args\":[\"swap/order:%s-USD-SWAP\"]}", (bAdd ? "subscribe" : "unsubscribe"), strCoinType.c_str());
	else
		_snprintf(szBuffer, 512, "{\"op\":\"%s\",\"args\":[\"futures/order:%s-USD-%s\"]}", (bAdd ? "subscribe" : "unsubscribe"), strCoinType.c_str(), strFuturesCycle.c_str());
	Request(szBuffer);
}

void COkexWebsocketAPI::API_FuturesAccountInfoByCurrency(bool bAdd, bool bSwap, std::string& strCoinType)
{
	char szBuffer[512] = { 0 };
	if(bSwap)
		_snprintf(szBuffer, 512, "{\"op\":\"%s\",\"args\":[\"swap/account:%s-USD-SWAP\"]}", (bAdd ? "subscribe" : "unsubscribe"), strCoinType.c_str());
	else
		_snprintf(szBuffer, 512, "{\"op\":\"%s\",\"args\":[\"futures/account:%s\"]}", (bAdd ? "subscribe" : "unsubscribe"), strCoinType.c_str());
	Request(szBuffer);
}

void COkexWebsocketAPI::API_SpotKlineData(bool bAdd, std::string& strKlineType, std::string& strCoinType, std::string& strMoneyType)
{
	m_spotKlineCheck = "spot/";
	m_spotKlineCheck.append(strKlineType);
	char szBuffer[512] = {0};
	_snprintf(szBuffer, 512, "{\"op\":\"%s\",\"args\":[\"spot/%s:%s-%s\"]}", (bAdd ? "subscribe" : "unsubscribe"), strKlineType.c_str(), strCoinType.c_str(), strMoneyType.c_str());
	Request(szBuffer);
}

void COkexWebsocketAPI::API_SpotTickerData(bool bAdd, std::string& strCoinType, std::string& strMoneyType)
{
	m_spotTickerCheck = "spot/ticker";
	char szBuffer[512] = {0};
	_snprintf(szBuffer, 512, "{\"op\":\"%s\",\"args\":[\"spot/ticker:%s-%s\"]}", (bAdd ? "subscribe" : "unsubscribe"), strCoinType.c_str(), strMoneyType.c_str());
	Request(szBuffer);
}

void COkexWebsocketAPI::API_SpotAccountInfoByCurrency(bool bAdd, std::string& strCoinType)
{
	char szBuffer[512] = {0};
	_snprintf(szBuffer, 512, "{\"op\":\"%s\",\"args\":[\"spot/account:%s\"]}", (bAdd ? "subscribe" : "unsubscribe"), strCoinType.c_str());
	Request(szBuffer);
}

void COkexWebsocketAPI::API_SpotOrderInfo(bool bAdd, std::string& strCoinType, std::string& strMoneyType)
{
	char szBuffer[512] = {0};
	_snprintf(szBuffer, 512, "{\"op\":\"%s\",\"args\":[\"spot/order:%s-%s\"]}", (bAdd ? "subscribe" : "unsubscribe"), strCoinType.c_str(), strMoneyType.c_str());
	Request(szBuffer);

}
#endif 