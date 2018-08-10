#pragma once
#include "websocket_api/websocket_api.h"
class CBWWebsocketAPI : public CWebsocketAPI
{
public:
	CBWWebsocketAPI(std::string strAPIKey, std::string strSecretKey);
	~CBWWebsocketAPI();
public:
	virtual void API_EntrustDepth(eMarketType type, int depthSize, bool bAdd);
	//�������³ɽ��Ķ���
	virtual void API_LatestExecutedOrder(eMarketType type);
	virtual void Ping(){};
};

