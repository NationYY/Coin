#pragma once
#include "exchange/okex/okex_websocket_api.h"
#include "exchange/okex/okex_http_api.h"
extern COkexExchange* pExchange;
extern void Pong();
extern void Listen();
#define OKEX_WEB_SOCKET ((COkexWebsocketAPI*)pExchange->GetWebSocket())
#define OKEX_HTTP ((COkexHttpAPI*)pExchange->GetHttp())
void local_websocket_callbak_open(const char* szExchangeName)
{
	LOCAL_INFO("连接成功");
	Listen();
}

void local_websocket_callbak_close(const char* szExchangeName)
{
	LOCAL_ERROR("断开连接");
}

void local_websocket_callbak_fail(const char* szExchangeName)
{
	LOCAL_ERROR("连接失败");
}

void local_http_callbak_message(eHttpAPIType apiType, Json::Value& retObj, const std::string& strRet, int customData, std::string strCustomData)
{
}

void local_websocket_callbak_message(eWebsocketAPIType apiType, const char* szExchangeName, Json::Value& retObj, const std::string& strRet)
{
	switch(apiType)
	{
	case eWebsocketAPIType_SpotTrade:
		{
			if(retObj.isObject() && retObj["data"].isArray())
			{
				for(int i = 0; i < (int)retObj["data"].size(); ++i)
				{
					std::string strInstrumentID = retObj["data"][i]["instrument_id"].asString();
					std::string strFile = "trade_"+strInstrumentID;
					CActionLog(strFile.c_str(), "%s	%s	%s	%s	%s	%s", retObj["data"][i]["trade_id"].asString().c_str(),
						retObj["data"][i]["price"].asString().c_str(),
						retObj["data"][i]["size"].asString().c_str(),
						retObj["data"][i]["side"].asString().c_str(),
						retObj["data"][i]["timestamp"].asString().c_str(),
						CFuncCommon::FormatDateStr(CFuncCommon::ISO8601ToTime(retObj["data"][i]["timestamp"].asString())).c_str());
				}
			}
		}
		break;
	case eWebsocketAPIType_Pong:
		{
			 Pong();
		}
		break;
	}
}