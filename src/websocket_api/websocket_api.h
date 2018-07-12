#pragma once
#include "websocket.h"
class CWebSocketAPI
{
public:
	CWebSocketAPI();
	~CWebSocketAPI();
	void SetKey(string api_key, string secret_key);
	void SetUri(string uri);
	void Request(const char* requestInfo);
	void SetCallBackOpen(websocketpp_callbak_open callbak_open);
	void SetCallBackClose(websocketpp_callbak_close callbak_close);
	void SetCallBackMessage(websocketpp_callbak_message callbak_message);
	void Run();
	void Close();
	static unsigned __stdcall CWebSocketAPI::RunThread(LPVOID arg);
private:
	websocketpp_callbak_open		m_callbak_open;
	websocketpp_callbak_close		m_callbak_close;
	websocketpp_callbak_message		m_callbak_message;
protected:
	string m_api_key;			//用户申请的apiKey
	string m_secret_key;		//请求参数签名的私钥
	string m_uri;
	WebSocket* pWebsocket;
	HANDLE hThread;

	
};

