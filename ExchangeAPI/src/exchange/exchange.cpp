#include "stdafx.h"
#include "exchange.h"

CExchange::CExchange(): m_httpCallbakMessage(NULL), m_webSocketCallbakOpen(NULL), m_webSocketCallbakClose(NULL),
m_webSocketCallbakMessage(NULL), m_pWebSocketAPI(NULL), m_pHttpAPI(NULL), m_pHttpTradeAPI(NULL)
{
}


CExchange::~CExchange()
{
}

void CExchange::Run()
{
	if(m_pHttpAPI)
	{
		m_pHttpAPI->SetCallBakFunction(boost::bind(&CExchange::OnHttpResponse, this, _1, _2, _3));
		m_pHttpAPI->Run(5);
	}

	if(m_pHttpTradeAPI)
	{
		m_pHttpTradeAPI->SetCallBakFunction(boost::bind(&CExchange::OnHttpResponse, this, _1, _2, _3));
		m_pHttpTradeAPI->Run(10);
	}

	if(m_pWebSocketAPI)
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

void CExchange::OnHttpResponse(eHttpAPIType type, Json::Value& retObj, const std::string& strRet)
{
	if(m_httpCallbakMessage)
		m_httpCallbakMessage(type, retObj, strRet);
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