#pragma once
#include "exchange/exchange.h"
class CZBGExchange : public CExchange
{
public:
	CZBGExchange(std::string strAPIKey, std::string strSecretKey);
	~CZBGExchange();
	virtual void OnHttpResponse(eHttpAPIType type, Json::Value& retObj, const std::string& strRet);
	virtual void OnWebsocketResponse(Json::Value& retObj, const std::string& strRet);
private:

};

