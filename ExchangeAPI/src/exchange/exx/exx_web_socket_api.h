#pragma once
#include "websocket_api/websocket_api.h"
class CExxWebSocketAPI : public CWebSocketAPI
{
public:
	CExxWebSocketAPI(string strAPIKey, string strSecretKey);
	~CExxWebSocketAPI();
	virtual void API_EntrustDepth(const char* szType, int depthSize, bool bAdd);
};

