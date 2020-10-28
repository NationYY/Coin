#include "stdafx.h"
#include "binance_websocket_api.h"
#include "log/local_log.h"

CBinanceWebsocketAPI::CBinanceWebsocketAPI(std::string strAPIKey, std::string strSecretKey):
 m_strStream("")
{
	SetKey(strAPIKey, strSecretKey);
	SetURI("");
}


CBinanceWebsocketAPI::~CBinanceWebsocketAPI()
{
}

void CBinanceWebsocketAPI::Ping()
{
	CWebsocketAPI::Ping();
}

void CBinanceWebsocketAPI::API_FuturesTickerData(bool bAdd, std::string& strCoinType, std::string& standardCurrency)
{
	std::string symbol = strCoinType+standardCurrency;
	CFuncCommon::ToLower(symbol);
	std::string stream = symbol +"@aggTrade";
	if(!UpdateStream(true, stream))
	{
		char szBuffer[128];
		_snprintf(szBuffer, 128, "{\"method\":\"SUBSCRIBE\",\"params\":[\"%s@aggTrade\"],\"id\":%d}", symbol.c_str(), eWebsocketAPIType_FuturesTicker);
		Request(szBuffer);
	}
}

void CBinanceWebsocketAPI::API_FuturesEntrustDepth(bool bAdd, std::string& strCoinType, std::string& standardCurrency)
{
	std::string symbol = strCoinType + standardCurrency;
	CFuncCommon::ToLower(symbol);
	std::string stream = symbol + "@depth5@100ms";
	if(!UpdateStream(true, stream))
	{
		char szBuffer[128];
		_snprintf(szBuffer, 128, "{\"method\":\"SUBSCRIBE\",\"params\":[\"%s@depth5@100ms\"],\"id\":%d}", symbol.c_str(), eWebsocketAPIType_FuturesEntrustDepth);
		Request(szBuffer);
	}
}

void CBinanceWebsocketAPI::SetListenKey(std::string key)
{
	std::string url = "wss://fstream.binance.com/ws/" + key;
	SetURI(url.c_str());
}

bool CBinanceWebsocketAPI::UpdateStream(bool bAdd, std::string stream)
{
	if(bAdd)
	{
		size_t pos = m_strStream.find(stream.c_str());
		if(pos == std::string::npos)
		{
			m_strStream = m_strStream + stream + "/";
			return true;
		}
	}
	return false;
}

void CBinanceWebsocketAPI::Run()
{
	if(!m_bHaveSetURI)
	{
		std::string url = "wss://fstream.binance.com/ws/";
		url = url + m_strStream;
		SetURI(url.c_str());
	}

	CWebsocketAPI::Run();
}