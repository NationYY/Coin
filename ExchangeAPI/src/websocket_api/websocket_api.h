#pragma once
#include <boost/thread/thread.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include "websocket.h"
typedef boost::function<void()> WEBSOCKET_OPEN_FUNCTION_TYPE;
typedef boost::function<void()> WEBSOCKET_CLOSE_FUNCTION_TYPE;
typedef boost::function<void()> WEBSOCKET_FAIL_FUNCTION_TYPE;
typedef boost::function<void(Json::Value&, const std::string&)> WEBSOCKET_MESSAGE_FUNCTION_TYPE;
class CWebSocketAPI
{
public:
	CWebSocketAPI();
	~CWebSocketAPI();
	void SetKey(std::string strAPIKey, std::string strSecretKey);
	void SetURI(std::string strURI);
	void Request(const char* szRequestInfo);
	void SetUTF8(bool bValue){
		m_bUTF8 = bValue;
	}
	bool GetIsUTF8(){
		return m_bUTF8;
	}
	void CWebSocketAPI::SetCallBackOpen(WEBSOCKET_OPEN_FUNCTION_TYPE func){
		m_openFunc = func;
	}

	void CWebSocketAPI::SetCallBackClose(WEBSOCKET_CLOSE_FUNCTION_TYPE func){
		m_closeFunc = func;
	}

	void CWebSocketAPI::SetCallBackMessage(WEBSOCKET_MESSAGE_FUNCTION_TYPE func){
		m_messageFunc = func;
	}

	void CWebSocketAPI::SetCallBackFail(WEBSOCKET_FAIL_FUNCTION_TYPE func){
		m_failFunc = func;
	}
	
	bool IsConnect(){
		return m_bConnect;
	}
	void Run();
	void Close();
	void Update();
	void PushRet(int type, Json::Value& retObj, const char* szRet);
	static unsigned __stdcall CWebSocketAPI::RunThread(LPVOID arg);
	//���Ľ������
	virtual void API_EntrustDepth(eMarketType type, int depthSize, bool bAdd) = 0;
	//�������³ɽ��Ķ���
	virtual void API_LatestExecutedOrder(eMarketType type) = 0;
	virtual void Ping() = 0;
protected:
	std::string m_strAPIKey;			//�û������apiKey
	std::string m_strSecretKey;		//�������ǩ����˽Կ
	std::string m_strURI;
	WebSocket* m_pWebsocket;
	HANDLE m_hThread;
	std::deque<SWebSocketResponse> m_queueResponseInfo;
	boost::mutex m_responseMutex;
	WEBSOCKET_OPEN_FUNCTION_TYPE m_openFunc;
	WEBSOCKET_CLOSE_FUNCTION_TYPE m_closeFunc;
	WEBSOCKET_MESSAGE_FUNCTION_TYPE m_messageFunc;
	WEBSOCKET_FAIL_FUNCTION_TYPE m_failFunc;
	bool m_bConnect;
	bool m_bUTF8;
	
};

