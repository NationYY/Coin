#pragma once
#include <boost/thread/thread.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include "websocket.h"
#include "exchange/exchange.h"
typedef boost::function<void()> WEBSOCKET_OPEN_FUNCTION_TYPE;
typedef boost::function<void()> WEBSOCKET_CLOSE_FUNCTION_TYPE;
typedef boost::function<void()> WEBSOCKET_FAIL_FUNCTION_TYPE;
typedef boost::function<void(Json::Value&, const std::string&)> WEBSOCKET_MESSAGE_FUNCTION_TYPE;
class CWebsocketAPI
{
public:
	CWebsocketAPI();
	virtual ~CWebsocketAPI();
	void SetKey(std::string strAPIKey, std::string strSecretKey, std::string strPassphrase = "", bool bFutures = false);
	void SetURI(std::string strURI);
	void SetExchange(CExchange* pExchange){
		m_pExchange = pExchange;
	}
	bool Request(const char* szRequestInfo);
	void SetUTF8(bool bValue){
		m_bUTF8 = bValue;
	}
	bool GetIsUTF8(){
		return m_bUTF8;
	}
	void SetGZIP(bool bValue){
		m_bGZIP = bValue;
	}
	bool GetGZIP(){
		return m_bGZIP;
	}
	void CWebsocketAPI::SetCallBackOpen(WEBSOCKET_OPEN_FUNCTION_TYPE func){
		m_openFunc = func;
	}

	void CWebsocketAPI::SetCallBackClose(WEBSOCKET_CLOSE_FUNCTION_TYPE func){
		m_closeFunc = func;
	}

	void CWebsocketAPI::SetCallBackMessage(WEBSOCKET_MESSAGE_FUNCTION_TYPE func){
		m_messageFunc = func;
	}

	void CWebsocketAPI::SetCallBackFail(WEBSOCKET_FAIL_FUNCTION_TYPE func){
		m_failFunc = func;
	}
	
	bool IsConnect(){
		return m_bConnect;
	}
	void Run();
	void Close();
	void Update();
	void PushRet(int type, Json::Value& retObj, const char* szRet);
	static unsigned __stdcall CWebsocketAPI::RunThread(LPVOID arg);
	//订阅现货交易深度
	virtual void API_EntrustDepth(eMarketType type, int depthSize, bool bAdd) = 0;
	//订阅最新成交的订单
	virtual void API_LatestExecutedOrder(eMarketType type) = 0;
	virtual bool Ping() = 0;
	//订阅合约K线
	virtual void API_FuturesKlineData(bool bAdd, std::string& strKlineType, std::string& strCoinType, std::string& strFuturesCycle) {};
	//订阅合约tick
	virtual void API_FuturesTickerData(bool bAdd, std::string& strCoinType, std::string& strFuturesCycle) {};
protected:
	std::string m_strAPIKey;			//用户申请的apiKey
	std::string m_strSecretKey;		//请求参数签名的私钥
	std::string m_strPassphrase;
	std::string m_strURI;
	WebSocket* m_pWebsocket;
	HANDLE m_hThread;
	std::deque<SWebSocketResponse> m_queueResponseInfo;
	boost::mutex m_responseMutex;
	WEBSOCKET_OPEN_FUNCTION_TYPE m_openFunc;
	WEBSOCKET_CLOSE_FUNCTION_TYPE m_closeFunc;
	WEBSOCKET_MESSAGE_FUNCTION_TYPE m_messageFunc;
	WEBSOCKET_FAIL_FUNCTION_TYPE m_failFunc;
	bool m_bUTF8;
	bool m_bGZIP;
	bool m_bFutures;
	CExchange* m_pExchange;
public:
	bool m_bConnect;
	std::string m_futuresKlineCheck;
	std::string m_futuresTickerCheck;
};

