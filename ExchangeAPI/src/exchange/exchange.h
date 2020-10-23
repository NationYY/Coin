#pragma once
#include "http_api/http_api.h"
#include "websocket_api/websocket_api.h"
#include "data_center.h"
class CExchange
{
public:
	CExchange();
	virtual ~CExchange();
	virtual const char* GetName() = 0;
	virtual void Update();
	void SetHttpCallBackMessage(http_callbak_message callbakMessage){
		m_httpCallbakMessage = callbakMessage;
	}
	void SetMarketWebSocketCallBackOpen(websocketpp_callbak_open callbak0pen){
		m_marketWebSocketCallbakOpen = callbak0pen;
	}
	void SetMarketWebSocketCallBackClose(websocketpp_callbak_close callbakClose){
		m_marketWebSocketCallbakClose = callbakClose;
	}
	void SetMarketWebSocketCallBackFail(websocketpp_callbak_fail callbakFail){
		m_marketWebSocketCallbakFail = callbakFail;
	}
	void SetMarketWebSocketCallBackMessage(websocketpp_callbak_message callbakMessage){
		m_marketWebSocketCallbakMessage = callbakMessage;
	}

	void SetAccountWebSocketCallBackOpen(websocketpp_callbak_open callbak0pen){
		m_accountWebSocketCallbakOpen = callbak0pen;
	}
	void SetAccountWebSocketCallBackClose(websocketpp_callbak_close callbakClose){
		m_accountWebSocketCallbakClose = callbakClose;
	}
	void SetAccountWebSocketCallBackFail(websocketpp_callbak_fail callbakFail){
		m_accountWebSocketCallbakFail = callbakFail;
	}
	void SetAccountWebSocketCallBackMessage(websocketpp_callbak_message callbakMessage){
		m_accountWebSocketCallbakMessage = callbakMessage;
	}


	virtual void Run(bool openWebSokect = true, int normalHttpThreadCnt = 5, int tradeHttpThreadCnt = 10);
	virtual void RunAccountWebSocket();
	virtual void OnHttpResponse(eHttpAPIType type, Json::Value& retObj, const std::string& strRet, int customData, std::string strCustomData);
	virtual void OnMarketWebsocketConnect(const char* szExchangeName);
	virtual void OnMarketWebsocketDisconnect(const char* szExchangeName);
	virtual void OnMarketWebsocketFailConnect(const char* szExchangeName);
	virtual void OnMarketWebsocketResponse(const char* szExchangeName, Json::Value& retObj, const std::string& strRet);

	virtual void OnAccountWebsocketConnect(const char* szExchangeName);
	virtual void OnAccountWebsocketDisconnect(const char* szExchangeName);
	virtual void OnAccountWebsocketFailConnect(const char* szExchangeName);
	virtual void OnAccountWebsocketResponse(const char* szExchangeName, Json::Value& retObj, const std::string& strRet);


	CHttpAPI* GetHttp(){
		return m_pHttpAPI;
	}
	CHttpAPI* GetTradeHttp(){
		return m_pHttpTradeAPI;
	}
	CWebsocketAPI* GetMarketWebSocket(){
		return m_pMarketWebSocketAPI;
	}

	CWebsocketAPI* GetAccountWebSocket(){
		return m_pAccountWebSocketAPI;
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
	CWebsocketAPI* m_pMarketWebSocketAPI;
	CWebsocketAPI* m_pAccountWebSocketAPI;
	CHttpAPI* m_pHttpAPI;
	CHttpAPI* m_pHttpTradeAPI;
	http_callbak_message m_httpCallbakMessage;
	websocketpp_callbak_open m_marketWebSocketCallbakOpen;
	websocketpp_callbak_close m_marketWebSocketCallbakClose;
	websocketpp_callbak_fail m_marketWebSocketCallbakFail;
	websocketpp_callbak_message m_marketWebSocketCallbakMessage;

	websocketpp_callbak_open m_accountWebSocketCallbakOpen;
	websocketpp_callbak_close m_accountWebSocketCallbakClose;
	websocketpp_callbak_fail m_accountWebSocketCallbakFail;
	websocketpp_callbak_message m_accountWebSocketCallbakMessage;

	std::list<eMarketType> m_listSupportMarket;
	
};

