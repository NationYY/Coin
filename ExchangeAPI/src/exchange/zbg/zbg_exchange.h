#pragma once
#include "exchange/exchange.h"
class CZbgExchange : public CExchange
{
public:
	CZbgExchange(std::string strAPIKey, std::string strSecretKey);
	~CZbgExchange();
	virtual const char* GetName(){
		return "ZBG";
	}
	virtual void OnHttpResponse(eHttpAPIType type, Json::Value& retObj, const std::string& strRet, int customData, std::string strCustomData);
	virtual void OnWebsocketResponse(Json::Value& retObj, const std::string& strRet);
	virtual void Run(bool openWebSokect = true, int normalHttpThreadCnt = 5, int tradeHttpThreadCnt = 10);
private:

};

