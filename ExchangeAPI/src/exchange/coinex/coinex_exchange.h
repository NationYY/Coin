#pragma once
#include "exchange/exchange.h"
class CCoinexExchange : public CExchange
{
public:
	CCoinexExchange(std::string strAPIKey, std::string strSecretKey);
	~CCoinexExchange();
	virtual void OnHttpResponse(eHttpAPIType type, Json::Value& retObj, const std::string& strRet, int customData, std::string strCustomData);
	virtual void OnWebsocketResponse(Json::Value& retObj, const std::string& strRet);
};

