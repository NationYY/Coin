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

void COkexWebsocketAPI::API_FuturesKlineData(bool bAdd, std::string& strKlineType, std::string& strCoinType, std::string& strFuturesCycle)
{
	m_futuresKlineCheck = "futures/";
	m_futuresKlineCheck.append(strKlineType);
	char szBuffer[512] = {0};
	_snprintf(szBuffer, 512, "{\"op\":\"%s\",\"args\":[\"futures/%s:%s-USD-%s\"]}", (bAdd ? "subscribe" : "unsubscribe"), strKlineType.c_str(), strCoinType.c_str(),  strFuturesCycle.c_str());
	Request(szBuffer);
}


void COkexWebsocketAPI::API_FuturesTickerData(bool bAdd, std::string& strCoinType, std::string& strFuturesCycle)
{
	m_futuresTickerCheck = "futures/ticker";
	char szBuffer[512] = {0};
	_snprintf(szBuffer, 512, "{\"op\":\"%s\",\"args\":[\"futures/ticker:%s-USD-%s\"]}", (bAdd ? "subscribe" : "unsubscribe"), strCoinType.c_str(), strFuturesCycle.c_str());
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

void COkexWebsocketAPI::API_FuturesOrderInfo(bool bAdd, std::string& strCoinType, std::string& strFuturesCycle)
{
	char szBuffer[512] = { 0 };
	_snprintf(szBuffer, 512, "{\"op\":\"%s\",\"args\":[\"futures/order:%s-USD-%s\"]}", (bAdd ? "subscribe" : "unsubscribe"), strCoinType.c_str(), strFuturesCycle.c_str());
	Request(szBuffer);
}

void COkexWebsocketAPI::API_FuturesAccountInfoByCurrency(bool bAdd, std::string& strCoinType)
{
	char szBuffer[512] = { 0 };
	_snprintf(szBuffer, 512, "{\"op\":\"%s\",\"args\":[\"futures/account:%s\"]}", (bAdd ? "subscribe" : "unsubscribe"), strCoinType.c_str());
	Request(szBuffer);
}
#endif 