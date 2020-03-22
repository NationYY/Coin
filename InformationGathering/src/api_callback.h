#pragma once
#include "exchange/okex/okex_websocket_api.h"
#include "exchange/okex/okex_http_api.h"
extern COkexExchange* pExchange;
extern void Pong();
extern void Listen();
void AddKlineData(std::string& instrumentid, std::string& _time, std::string& _open, std::string& _high, std::string& _low, std::string& _close, std::string& _volume);
void ComplementedKLine(std::string& instrumentid, time_t tNowKlineTick, int kLineCnt);
#define OKEX_WEB_SOCKET ((COkexWebsocketAPI*)pExchange->GetWebSocket())
#define OKEX_HTTP ((COkexHttpAPI*)pExchange->GetHttp())

extern std::map<std::string, time_t> mapLastKlineTime;
extern std::map<std::string, __int64> mapLastKlineRecordTime;
std::map<std::string, Json::Value> mapLastKlineJson;
int nKlineCycle = 60;

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
					time_t tNow = time(NULL);
					tm *nowtm = localtime(&tNow);
					char context[256] = { 0 };
					_snprintf(context, sizeof(context)-1, "%02d:%02d:%02d %s %s",
						(int)(nowtm->tm_hour), nowtm->tm_min, nowtm->tm_sec,
						retObj["data"][i]["trade_id"].asString().c_str(),
						retObj["data"][i]["timestamp"].asString().c_str());
					std::cout << context << std::endl;

					std::string strInstrumentID = retObj["data"][i]["instrument_id"].asString();
					std::string strFile = "okex_trade_"+strInstrumentID;
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
	case eWebsocketAPIType_SpotKline:
		{
			time_t curTime = CFuncCommon::ISO8601ToTime(retObj["data"][0]["candle"][0].asString());
			std::string instrumentid = retObj["data"][0]["instrument_id"].asString();
			if(curTime >= mapLastKlineTime[instrumentid])
			{
				if(curTime > mapLastKlineTime[instrumentid] && mapLastKlineTime[instrumentid] != 0)
					AddKlineData(instrumentid, mapLastKlineJson[instrumentid]["data"][0]["candle"][0].asString(), mapLastKlineJson[instrumentid]["data"][0]["candle"][1].asString(), mapLastKlineJson[instrumentid]["data"][0]["candle"][2].asString(), mapLastKlineJson[instrumentid]["data"][0]["candle"][3].asString(), mapLastKlineJson[instrumentid]["data"][0]["candle"][4].asString(), mapLastKlineJson[instrumentid]["data"][0]["candle"][5].asString());
				mapLastKlineTime[instrumentid] = curTime;
				mapLastKlineJson[instrumentid] = retObj;
			}
		}
		break;
	}
}

void ComplementedKLine(std::string& instrumentid, time_t tNowKlineTick, int kLineCnt)
{
	LOCAL_INFO("%s 补充K线", instrumentid.c_str());
	time_t endTick = tNowKlineTick - nKlineCycle;
	time_t beginTick = endTick - (kLineCnt-1)*nKlineCycle;
	std::string strFrom = CFuncCommon::LocaltimeToISO8601(beginTick);
	std::string strTo = CFuncCommon::LocaltimeToISO8601(endTick);
	std::string strKlineCycle = CFuncCommon::ToString(nKlineCycle);
	SHttpResponse resInfo;
	OKEX_HTTP->API_GetSpotSomeKline(false, instrumentid, strKlineCycle, strFrom, strTo, &resInfo);
	Json::Value& retObj = resInfo.retObj;
	if(retObj.isArray())
	{
		for(int i = retObj.size() - 1; i >= 0; --i)
		{
			AddKlineData(instrumentid, retObj[i][0].asString(), retObj[i][1].asString(), retObj[i][2].asString(), retObj[i][3].asString(), retObj[i][4].asString(), retObj[i][5].asString());
		}
	}
}

void AddKlineData(std::string& instrumentid, std::string& _time, std::string& _open, std::string& _high, std::string& _low, std::string& _close, std::string& _volume)
{
	__int64 time = CFuncCommon::ISO8601ToTime(_time);
	if(mapLastKlineRecordTime[instrumentid] != 0 && time - mapLastKlineRecordTime[instrumentid] != nKlineCycle)
		ComplementedKLine(instrumentid, time, int(time - mapLastKlineRecordTime[instrumentid] - nKlineCycle)/nKlineCycle);
	std::string strFile = "okex_1min_kline_"+instrumentid;
	CActionLog(strFile.c_str(), "%s	%s	%s	%s	%s	%s	%s", _open.c_str(),
		_high.c_str(),
		_low.c_str(),
		_close.c_str(),
		_volume.c_str(),
		_time.c_str(),
		CFuncCommon::FormatDateStr(CFuncCommon::ISO8601ToTime(_time)).c_str());
	mapLastKlineRecordTime[instrumentid] = time;
}