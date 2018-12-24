#pragma once
#include "websocket_api/websocket_api.h"
#ifdef _OPEN_BW_
class CBWWebsocketAPI : public CWebsocketAPI
{
public:
	CBWWebsocketAPI(std::string strAPIKey, std::string strSecretKey);
	virtual ~CBWWebsocketAPI();
public:
	virtual void API_EntrustDepth(eMarketType type, int depthSize, bool bAdd);
	//�������³ɽ��Ķ���
	virtual void API_LatestExecutedOrder(eMarketType type);
	virtual bool Ping(){ return true; };
};

#endif