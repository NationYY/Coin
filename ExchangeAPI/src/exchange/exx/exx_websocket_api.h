#pragma once
#include "websocket_api/websocket_api.h"
class CExxWebSocketAPI : public CWebSocketAPI
{
public:
	CExxWebSocketAPI(std::string strAPIKey, std::string strSecretKey);
	~CExxWebSocketAPI();
	virtual void API_EntrustDepth(const char* szType, int depthSize, bool bAdd);
};
