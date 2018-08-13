#pragma once
#include "http_api/http_api.h"
#include "websocket_api/websocket_api.h"
#include "data_center.h"
class CExchange
{
public:
	CExchange();
	~CExchange();
	virtual const char* GetName() = 0;
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
	void SetWebSocketCallBackFail(websocketpp_callbak_fail callbakFail){
		m_webSocketCallbakFail = callbakFail;
	}
	void SetWebSocketCallBackMessage(websocketpp_callbak_message callbakMessage){
		m_webSocketCallbakMessage = callbakMessage;
	}
	virtual void Run(bool openWebSokect = true, int normalHttpThreadCnt = 5, int tradeHttpThreadCnt = 10);
	virtual void OnHttpResponse(eHttpAPIType type, Json::Value& retObj, const std::string& strRet, int customData, std::string strCustomData);
	virtual void OnWebsocketConnect(const char* szExchangeName);
	virtual void OnWebsocketDisconnect(const char* szExchangeName);
	virtual void OnWebsocketFailConnect(const char* szExchangeName);
	virtual void OnWebsocketResponse(const char* szExchangeName, Json::Value& retObj, const std::string& strRet);
	CHttpAPI* GetHttp(){
		return m_pHttpAPI;
	}
	CHttpAPI* GetTradeHttp(){
		return m_pHttpTradeAPI;
	}
	CWebsocketAPI* GetWebSocket(){
		return m_pWebSocketAPI;
	}
	CDataCenter* GetDataCenter(){
		return &m_dataCenter;
	}
	virtual const std::list<eMarketType>& GetSupportMarket(){
		return m_listSupportMarket;
	}
	virtual bool IsSupportMarket(eMarketType type){
		std::list<eMarketType>::iterator itB = m_listSupportMarket.begin();
		std::list<eMarketType>::iterator itE = m_listSupportMarket.end();
		while(itB != itE)
		{
			if(*itB == type)
				return true;
			++itB;
		}
		return false;
	}
	virtual const char* GetMarketString(eMarketType type, bool bHttp){
		return "";
	}
	virtual int GetMarketID(eMarketType type){
		return 0;
	}
	virtual double GetTakerRate(){
		return 0;
	}
protected:
	CDataCenter m_dataCenter;
	CWebsocketAPI* m_pWebSocketAPI;
	CHttpAPI* m_pHttpAPI;
	CHttpAPI* m_pHttpTradeAPI;
	http_callbak_message m_httpCallbakMessage;
	websocketpp_callbak_open m_webSocketCallbakOpen;
	websocketpp_callbak_close m_webSocketCallbakClose;
	websocketpp_callbak_fail m_webSocketCallbakFail;
	websocketpp_callbak_message m_webSocketCallbakMessage;
	std::list<eMarketType> m_listSupportMarket;
	
};

