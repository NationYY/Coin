#include "stdafx.h"
#include "exchange.h"

CExchange::CExchange(): m_httpCallbakMessage(NULL), m_marketWebSocketCallbakOpen(NULL), m_marketWebSocketCallbakClose(NULL),
m_marketWebSocketCallbakMessage(NULL), m_pMarketWebSocketAPI(NULL), m_pHttpAPI(NULL), m_pHttpTradeAPI(NULL), m_pAccountWebSocketAPI(NULL),
m_accountWebSocketCallbakOpen(NULL), m_accountWebSocketCallbakClose(NULL), m_accountWebSocketCallbakFail(NULL), m_accountWebSocketCallbakMessage(NULL)
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
	if(m_pMarketWebSocketAPI)
	{
		m_pMarketWebSocketAPI->Close();
		delete m_pMarketWebSocketAPI;
		m_pMarketWebSocketAPI = NULL;
	}
	if(m_pAccountWebSocketAPI)
	{
		m_pAccountWebSocketAPI->Close();
		delete m_pAccountWebSocketAPI;
		m_pAccountWebSocketAPI = NULL;
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

	if(m_pMarketWebSocketAPI && openWebSokect)
	{
		m_pMarketWebSocketAPI->SetCallBackOpen(boost::bind(&CExchange::OnMarketWebsocketConnect, this, GetName()));
		m_pMarketWebSocketAPI->SetCallBackClose(boost::bind(&CExchange::OnMarketWebsocketDisconnect, this, GetName()));
		m_pMarketWebSocketAPI->SetCallBackFail(boost::bind(&CExchange::OnMarketWebsocketFailConnect, this, GetName()));
		m_pMarketWebSocketAPI->SetCallBackMessage(boost::bind(&CExchange::OnMarketWebsocketResponse, this, GetName(), _1, _2));
		m_pMarketWebSocketAPI->Run();
	}
}

void CExchange::RunAccountWebSocket()
{
	if(m_pAccountWebSocketAPI)
	{
		m_pAccountWebSocketAPI->SetCallBackOpen(boost::bind(&CExchange::OnAccountWebsocketConnect, this, GetName()));
		m_pAccountWebSocketAPI->SetCallBackClose(boost::bind(&CExchange::OnAccountWebsocketDisconnect, this, GetName()));
		m_pAccountWebSocketAPI->SetCallBackFail(boost::bind(&CExchange::OnAccountWebsocketFailConnect, this, GetName()));
		m_pAccountWebSocketAPI->SetCallBackMessage(boost::bind(&CExchange::OnAccountWebsocketResponse, this, GetName(), _1, _2));
		m_pAccountWebSocketAPI->Run();
	}
}

void CExchange::Update()
{
	if(m_pHttpAPI)
		m_pHttpAPI->Update();
	if(m_pMarketWebSocketAPI)
		m_pMarketWebSocketAPI->Update();
	if(m_pAccountWebSocketAPI)
		m_pAccountWebSocketAPI->Update();
	if(m_pHttpTradeAPI)
		m_pHttpTradeAPI->Update();
}

void CExchange::OnHttpResponse(eHttpAPIType type, Json::Value& retObj, const std::string& strRet, int customData, std::string strCustomData)
{
	if(m_httpCallbakMessage)
		m_httpCallbakMessage(type, retObj, strRet, customData, strCustomData);
}

void CExchange::OnMarketWebsocketConnect(const char* szExchangeName)
{
	if(m_marketWebSocketCallbakOpen)
		m_marketWebSocketCallbakOpen(szExchangeName);

}
void CExchange::OnMarketWebsocketDisconnect(const char* szExchangeName)
{
	if(m_marketWebSocketCallbakClose)
		m_marketWebSocketCallbakClose(szExchangeName);
}

void CExchange::OnMarketWebsocketFailConnect(const char* szExchangeName)
{
	if(m_marketWebSocketCallbakFail)
		m_marketWebSocketCallbakFail(szExchangeName);
}

void CExchange::OnMarketWebsocketResponse(const char* szExchangeName, Json::Value& retObj, const std::string& strRet)
{
	if(m_marketWebSocketCallbakMessage)
		m_marketWebSocketCallbakMessage(eWebsocketAPIType_Max, szExchangeName, retObj, strRet);
}


void CExchange::OnAccountWebsocketConnect(const char* szExchangeName)
{
	if(m_accountWebSocketCallbakOpen)
		m_accountWebSocketCallbakOpen(szExchangeName);

}
void CExchange::OnAccountWebsocketDisconnect(const char* szExchangeName)
{
	if(m_accountWebSocketCallbakClose)
		m_accountWebSocketCallbakClose(szExchangeName);
}

void CExchange::OnAccountWebsocketFailConnect(const char* szExchangeName)
{
	if(m_accountWebSocketCallbakFail)
		m_accountWebSocketCallbakFail(szExchangeName);
}

void CExchange::OnAccountWebsocketResponse(const char* szExchangeName, Json::Value& retObj, const std::string& strRet)
{
	if(m_accountWebSocketCallbakMessage)
		m_accountWebSocketCallbakMessage(eWebsocketAPIType_Max, szExchangeName, retObj, strRet);
}