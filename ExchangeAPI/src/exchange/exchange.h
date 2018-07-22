#pragma once
#include "http_api/http_api.h"
#include "websocket_api/websocket_api.h"
#include "data_center.h"
class CExchange
{
public:
	CExchange();
	~CExchange();
	virtual void Update();
	void SetHttpCallBackMessage(http_callbak_message callbakMessage){
		m_httpCallbakMessage = callbakMessage;
	}
	void SetWebSocketCallBackOpen(websocketpp_callbak_open callbak0pen){
		m_webSocketCallbakOpen = callbak0pen;
	}
	void SetWebSocketCallBackClose(websocketpp_callbak_close callbakClose){
		m_webSocketCallbakClose = callbakClose;
	}
	void SetWebSocketCallBackMessage(websocketpp_callbak_message callbakMessage){
		m_webSocketCallbakMessage = callbakMessage;
	}
	virtual void Run();
	virtual void OnHttpResponse(eHttpAPIType type, Json::Value& retObj, const std::string& strRet);
	virtual void OnWebsocketConnect();
	virtual void OnWebsocketDisconnect();
	virtual void OnWebsocketResponse(Json::Value& retObj, const std::string& strRet);
	CHttpAPI* GetHttp(){
		return m_pHttpAPI;
	} 
	CWebSocketAPI* GetWebSocket(){
		return m_pWebSocketAPI;
	}
	CDataCenter* GetDataCenter(){
		return &m_dataCenter;
	}
protected:
	CDataCenter m_dataCenter;
	CWebSocketAPI* m_pWebSocketAPI;
	CHttpAPI* m_pHttpAPI;
	http_callbak_message m_httpCallbakMessage;
	websocketpp_callbak_open m_webSocketCallbakOpen;
	websocketpp_callbak_close m_webSocketCallbakClose;
	websocketpp_callbak_message m_webSocketCallbakMessage;
	
};

