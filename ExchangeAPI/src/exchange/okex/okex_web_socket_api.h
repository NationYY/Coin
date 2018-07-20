#pragma once
#include "websocket_api/websocket_api.h"
class COkexWebSocketAPI : public CWebSocketAPI
{
public:
	COkexWebSocketAPI(std::string strAPIKey, std::string strSecretKey);
	~COkexWebSocketAPI();
	void API_sub_spot_ticker(const char* szTransactionType);
};

