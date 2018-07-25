#pragma once
#include "exchange/exchange.h"
class CExxExchange : public CExchange
{
public:
	CExxExchange(std::string strAPIKey, std::string strSecretKey);
	~CExxExchange();
	virtual void OnHttpResponse(eHttpAPIType type, Json::Value& retObj, const std::string& strRet);
	virtual void OnWebsocketResponse(Json::Value& retObj, const std::string& strRet);

private:
	void Parse_Balance(Json::Value& retObj, const std::string& strRet);
	void Parse_Ticker(Json::Value& retObj, const std::string& strRet);
	void Parse_EntrustDepth(Json::Value& retObj, const std::string& strRet);
};

