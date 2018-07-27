#include "stdafx.h"
#include "exchange.h"

CExchange::CExchange(): m_httpCallbakMessage(NULL), m_webSocketCallbakOpen(NULL), m_webSocketCallbakClose(NULL),
m_webSocketCallbakMessage(NULL), m_pWebSocketAPI(NULL), m_pHttpAPI(NULL), m_pHttpTradeAPI(NULL)
{
}


CExchange::~CExchange()
{
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
		m_pWebSocketAPI->SetCallBackOpen(boost::bind(&CExchange::OnWebsocketConnect, this));
		m_pWebSocketAPI->SetCallBackClose(boost::bind(&CExchange::OnWebsocketDisconnect, this));
		m_pWebSocketAPI->SetCallBackFail(boost::bind(&CExchange::OnWebsocketFailConnect, this));
		m_pWebSocketAPI->SetCallBackMessage(boost::bind(&CExchange::OnWebsocketResponse, this, _1, _2));
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

void CExchange::OnWebsocketConnect()
{
	if(m_webSocketCallbakOpen)
		m_webSocketCallbakOpen();

}
void CExchange::OnWebsocketDisconnect()
{
	if(m_webSocketCallbakClose)
		m_webSocketCallbakClose();
}

void CExchange::OnWebsocketFailConnect()
{
	if(m_webSocketCallbakFail)
		m_webSocketCallbakFail();
}

void CExchange::OnWebsocketResponse(Json::Value& retObj, const std::string& strRet)
{
	if(m_webSocketCallbakMessage)
		m_webSocketCallbakMessage(eWebsocketAPIType_Max, retObj, strRet);
}