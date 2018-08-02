#include "stdafx.h"
#include "huobi_hadax_websocket_api.h"


CHuobiHadaxWebsocketAPI::CHuobiHadaxWebsocketAPI(std::string strAPIKey, std::string strSecretKey)
{
	SetKey(strAPIKey, strSecretKey);
	SetURI("wss://api.hadax.com/ws");
}


CHuobiHadaxWebsocketAPI::~CHuobiHadaxWebsocketAPI()
{
}

void CHuobiHadaxWebsocketAPI::Ping()
{
	char szBuffer[512] = {0};
	_snprintf(szBuffer, 512, "{\"ping\": %lld}", time(NULL)*1000);
	Request(szBuffer);
}

void CHuobiHadaxWebsocketAPI::API_EntrustDepth(eMarketType type, int depthSize, bool bAdd)
{
	if(m_pExchange)
		m_pExchange->GetDataCenter()->ClearAllEntrustDepth();
	char szBuffer[512] = {0};
	if(bAdd)
		_snprintf(szBuffer, 512, "{\"sub\": \"market.%s.depth.step0\",\"id\" : \"id1\"}", m_pExchange->GetMarketString(type, false));
	Request(szBuffer);
}