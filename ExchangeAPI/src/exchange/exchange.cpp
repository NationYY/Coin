#include "stdafx.h"
#include "exchange.h"

CExchange::CExchange()
{
}


CExchange::~CExchange()
{
}

void CExchange::Run()
{
	m_pHttpAPI->SetCallBakFunction(boost::bind(&CExchange::OnHttpResponse, this, _1, _2, _3));
	m_pHttpAPI->Run(1);

	m_pWebSocketAPI->SetCallBackOpen(boost::bind(&CExchange::OnWebsocketConnect, this));
	m_pWebSocketAPI->SetCallBackClose(boost::bind(&CExchange::OnWebsocketDisconnect, this));
	m_pWebSocketAPI->SetCallBackMessage(boost::bind(&CExchange::OnWebsocketResponse, this, _1, _2));
	m_pWebSocketAPI->Run();

}

void CExchange::Update()
{
	m_pHttpAPI->Update();

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
void CExchange::OnWebsocketResponse(Json::Value& retObj, const std::string& strRet)
{
	if(m_webSocketCallbakMessage)
		m_webSocketCallbakMessage(retObj, strRet);
}