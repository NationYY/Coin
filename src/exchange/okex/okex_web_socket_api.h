#pragma once
#include "websocket_api.h"
class COkexWebSocketAPI : public CWebSocketAPI
{
public:
	COkexWebSocketAPI(string strAPIKey, string strSecretKey);
	~COkexWebSocketAPI();
	void API_sub_spot_ticker(const char* szTransactionType);
};

