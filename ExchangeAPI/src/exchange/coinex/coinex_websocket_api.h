#pragma once
#include "websocket_api/websocket_api.h"
class CCoinexWebSocketAPI : public CWebSocketAPI
{
public:
	CCoinexWebSocketAPI(std::string strAPIKey, std::string strSecretKey);
	~CCoinexWebSocketAPI();
	void LoginIn();
	void GetServerTime();
public:
	//���Ľ������
	virtual void API_EntrustDepth(eMarketType type, int depthSize, bool bAdd);
	//�������³ɽ��Ķ���
	virtual void API_LatestExecutedOrder(eMarketType type);
	virtual void Ping();
};

