#include "stdafx.h"
#include "exchange.h"

CExchange::CExchange(): m_httpCallbakMessage(NULL), m_webSocketCallbakOpen(NULL), m_webSocketCallbakClose(NULL),
m_webSocketCallbakMessage(NULL), m_pWebSocketAPI(NULL), m_pHttpAPI(NULL), m_pHttpTradeAPI(NULL)
{
}


CExchange::~CExchange()
{
	if(m_pHttpAPI)
	{
		m_pHttpAPI->Close();
		delete m_pHttpAPI;
		m_pHttpAPI = NULL;
	}
	if(m_pHttpTradeAPI)
	{
		m_pHttpTradeAPI->Close();
		delete m_pHttpTradeAPI;
		m_pHttpTradeAPI = NULL;
	}
	if(m_pWebSocketAPI)
	{
		m_pWebSocketAPI->Close();
		delete m_pWebSocketAPI;
		m_pWebSocketAPI = NULL;
	}
}

void CExchange::Run(bool openWebSokect, int normalHttpThreadCnt, int tradeHttpThreadCnt)
{
	if(m_pHttpAPI && normalHttpThreadCnt > 0)
	{
		m_pHttpAPI->SetCallBakFunction(boost::bind(&CExchange::OnHttpResponse, this, _1, _2, _3, _4, _5));
		m_pHttpAPI->Run(normalHttpThreadCnt);
	}

	if(m_pHttpTradeAPI && tradeHttpThreadCnt > 0)
	{
		m_pHttpTradeAPI->SetCallBakFunction(boost::bind(&CExchange::OnHttpResponse, this, _1, _2, _3, _4, _5));
		m_pHttpTradeAPI->Run(tradeHttpThreadCnt);
	}

	if(m_pWebSocketAPI && openWebSokect)
	{
		m_pWebSocketAPI->SetCallBackOpen(boost::bind(&CExchange::OnWebsocketConnect, this, GetName()));
		m_pWebSocketAPI->SetCallBackClose(boost::bind(&CExchange::OnWebsocketDisconnect, this, GetName()));
		m_pWebSocketAPI->SetCallBackFail(boost::bind(&CExchange::OnWebsocketFailConnect, this, GetName()));
		m_pWebSocketAPI->SetCallBackMessage(boost::bind(&CExchange::OnWebsocketResponse, this, GetName(), _1, _2));
		m_pWebSocketAPI->Run();
	}
}

void CExchange::Update()
{
	if(m_pHttpAPI)
		m_pHttpAPI->Update();
	if(m_pWebSocketAPI)
		m_pWebSocketAPI->Update();
	if(m_pHttpTradeAPI)
		m_pHttpTradeAPI->Update();
}

void CExchange::OnHttpResponse(eHttpAPIType type, Json::Value& retObj, const std::string& strRet, int customData, std::string strCustomData)
{
	if(m_httpCallbakMessage)
		m_httpCallbakMessage(type, retObj, strRet, customData, strCustomData);
}

void CExchange::OnWebsocketConnect(const char* szExchangeName)
{
	if(m_webSocketCallbakOpen)
		m_webSocketCallbakOpen(szExchangeName);

}
void CExchange::OnWebsocketDisconnect(const char* szExchangeName)
{
	if(m_webSocketCallbakClose)
		m_webSocketCallbakClose(szExchangeName);
}

void CExchange::OnWebsocketFailConnect(const char* szExchangeName)
{
	if(m_webSocketCallbakFail)
		m_webSocketCallbakFail(szExchangeName);
}

void CExchange::OnWebsocketResponse(const char* szExchangeName, Json::Value& retObj, const std::string& strRet)
{
	if(m_webSocketCallbakMessage)
		m_webSocketCallbakMessage(eWebsocketAPIType_Max, szExchangeName, retObj, strRet);
}